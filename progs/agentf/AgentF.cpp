//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012  loh.tar@googlemail.com
//
//   Filu is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 2 of the License, or
//   (at your option) any later version.
//
//   Filu is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Filu. If not, see <http://www.gnu.org/licenses/>.
//

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSqlRecord>
#include <QRegExp>
#include <QSqlQuery>
#include <QTextStream>
#include <QTimer>

#include "AgentF.h"

#include "CmdClass.h"
#include "CmdHelper.h"
#include "Depots.h"
#include "Exporter.h"
#include "FiluU.h"
#include "FTool.h"
#include "RcFile.h"
#include "Scanner.h"
#include "Script.h"
#include "SymbolTuple.h"
#include "muParser.h"

AgentF::AgentF(QCoreApplication& app)
      : FCoreApp("AgentF", app)
      , mScript(0)
      , mExporter(0)
      , mScanner(0)
      , mQuit(true)
      , mIamEvil(false)
{
  readSettings();
  setMsgTargetFormat(eVerbose, "%C: %x");
  setMsgTargetFormat(eConsLog, "%C: *** %t *** %x");

  mCmd->regCmds("this full rcf exp scan daemon "
                "deleteBars splitBars info depots fetch");

  CmdClass::allRegCmds(mCmd);

  mCmd->regStdOpts("verbose");
  mCmd->inGreeter("AgentF is part of Filu. Visit http://filu.sourceforge.net");

  exec(mCommandLine);

  QTimer::singleShot(500, this, SLOT(run()));
}

AgentF::~AgentF()
{
  foreach(CmdClass* cmd, mCmds) delete cmd;

  if(mScript)   delete mScript;
  if(mExporter) delete mExporter;
  if(mScanner)  delete mScanner;

  if(!hasMessage())   verbose(FUNC, tr("Done."));
  else if(hasFatal()) verbose(FUNC, tr("Houston, we have a problem."));
  else if(hasError()) verbose(FUNC, tr("Exit with error."));
  else verbose(FUNC, tr("Not the best."));
}

void AgentF::run()
{
  if(mQuit)
  {
    quit();
    return;
  }
}

void AgentF::quit()
{
  addErrors(mCmd->errors());
  QCoreApplication::exit(hasError());
}

void AgentF::readSettings()
{
  if(verboseLevel(eMax)) printSettings();
}

void AgentF::addEODBarData(const QString& symbol, const QString& market, const QString& provider
                         , const QDate& fromDate, const QDate& toDate, int fiId, int marketId)
{
  DateRange dateRange;
  mFilu->getEODBarDateRange(dateRange, fiId, marketId, Filu::eBronze);
  // FIXME: To find out if anything is todo we have to check more smarter.
  //        We have to use the 'offday' table, but its not implemented yet.

  QDate from = fromDate;
  QDate to   = toDate;

  // Avoid 'holes' in the data table
  if(from > dateRange.value("last")) from = dateRange.value("last").addDays(1);
  else if(from == QDate(1000, 1, 1)) from = dateRange.value("last").addDays(1);

  if(from.dayOfWeek() == Qt::Saturday) from = from.addDays(2);
  else if(from.dayOfWeek() == Qt::Sunday) from = from.addDays(1);

  if(from > QDate::currentDate())
  {
    verbose(FUNC, tr("Nothing todo, last bar is up to date: %1 %2")
                    .arg(mCmd->argStr(1), mCmd->argStr(2)), eAmple);
    return;
  }

  // Once more, avoid 'holes' in the data table
  if(to < dateRange.value("first")) to = dateRange.value("first").addDays(-1);
  else if(to == QDate(3000, 1, 1)) to = QDate::currentDate();

  // Could happens if you update bars on weekend but there is nothing todo
  if(from > to) return;

  // Build the parameter list needed by the script
  QStringList scriptParms;
  scriptParms.append(from.toString(Qt::ISODate));
  scriptParms.append(to.toString(Qt::ISODate));
  scriptParms.append(symbol);
  scriptParms.append(market);

  if(!mScript) mScript = new Script(this);

  QStringList* data = mScript->askProvider(provider, "fetchBar", scriptParms);

  if(!data)
  {
    fatal(FUNC, "Got no 'QStringList* data'.");
    return;
  }

  if(data->size() < 2)
  {
    warning(FUNC, tr("No data from script: %1 - %2").arg(provider, scriptParms.join(" ")));
    return;
  }

  // Here is the beef...
  mFilu->addEODBarData(fiId, marketId, data);
  delete data; // No longer needed
  if(check4FiluError(FUNC)) return;
  // ...and as dessert check for events
  if(!mScanner)
  {
    mScanner = new Scanner(this);
    mScanner->autoSetup();
  }

  mScanner->scanThis(fiId, marketId);
}

void AgentF::addEODBarData()
{
  if(mCmd->isMissingParms(5))
  {
    if(mCmd->printThisWay("<Symbol> <Market> <Provider> <FromDate> <ToDate> [<FiId> <MarketId>]")) return;

    mCmd->printComment(tr("It's used by 'full'. You should use 'fetch' instead, "
                          "because the time is coming where it will removed."));

    mCmd->aided();
    return;
  }

  QDate fromDate   = mCmd->argDate(4, QDate(1000, 1, 1));
  QDate toDate     = mCmd->argDate(5, QDate::currentDate());
  int fiId         = mCmd->argInt(6);
  int marketId     = mCmd->argInt(7);

  if(mCmd->hasError()) return;

  if(!fiId or !marketId)
  {
    SymbolTuple* st = mFilu->getSymbol(mCmd->argStr(1), mCmd->argStr(2), mCmd->argStr(3));
    if(!st)
    {
      error(FUNC, tr("Symbol not found: %1-%2-%3").arg(mCmd->argStr(1), mCmd->argStr(2), mCmd->argStr(3)));
      return;
    }

    st->next();

    fiId     = st->fiId();
    marketId = st->marketId();

    delete st;
  }

  addEODBarData(mCmd->argStr(1)
              , mCmd->argStr(2)
              , mCmd->argStr(3)
              , fromDate
              , toDate
              , fiId
              , marketId);
}

void AgentF::updateAllBars()
{
  if(mCmd->isMissingParms())
  {
    if(mCmd->printThisWay("[<FromDate> [<ToDate>]]")) return;

    mCmd->printComment(tr("Without a given date the Agent take a look at the database "
                          "which bars could be missing."));
    mCmd->aided();
    return;
  }

  QString fromDate = mCmd->argDate(1, QDate(1000, 1, 1)).toString(Qt::ISODate);
  QString toDate   = mCmd->argDate(2, QDate::currentDate()).toString(Qt::ISODate);

  if(mCmd->hasError()) return;

  verbose(FUNC, tr("Processing..."), eInfo);

  // The empty strings "" says: Any caption and all kind of fType
  QSqlQuery* query = mFilu->searchFi("", "");

  if(!query)
  {
    check4FiluError(FUNC);
    return;
  }

  if(!query->size())
  {
    warning(FUNC, tr("No provider symbols found."));
    QCoreApplication::exit(1);
    return;
  }

  record(FUNC, tr("\n*\n* Update bar data of all %1 FIs.\n*").arg(query->size()));

  // Build the parameter list needed by addEODBarData()
  // <Caller> -this <Symbol> <Market> <Provider> [<FromDate> [<ToDate>]]
  QStringList parameters;
  // No parameters.append("<Caller>"); done later by beEvil()
  parameters.append("this");
  parameters.append(""); // Placeholders, will fill below
  parameters.append("");
  parameters.append("");
  parameters.append(fromDate);
  parameters.append(toDate);
  parameters.append("");
  parameters.append("");

  while(query->next())
  {
    QSqlRecord r = query->record();
    parameters[1] = r.value("Symbol").toString();
    parameters[2] = r.value("Market").toString();
    parameters[3] = r.value("Provider").toString();
    parameters[6] = r.value("FiId").toString();
    parameters[7] = r.value("MarketId").toString();

    // Save the now completed parameter list needed by addEODBarData()
    mCommands.append(parameters);
  }

  parameters.clear();
  parameters << "MASTER_CMD" << "MarkScanned";
  mCommands.append(parameters);

  startClones();
  mQuit = false; // Don't quit after all, enter main event loop
}

bool AgentF::lineToCommand(const QString& line, QStringList& cmd)
{
  if(line.startsWith("*")) return false; // Ignore remarks
  if(line.isEmpty()) return false;

  cmd = line.split(" ", QString::SkipEmptyParts);

  // Concat splitted parts if there was quotation marks
  int size = cmd.size();
  for(int i = 0; i < size; ++i)
  {
    if(cmd.at(i).startsWith("\""))
    {
      QString help = cmd.at(i);
      for(int j = i; j < size; )
      {
        help.append(" " + cmd.at(j));
        cmd.removeAt(j);
        --size;
        if(help.endsWith("\"")) break;
      }
      help.remove("\"");
      cmd[i] = help;
    }
  }

  return true;
}

void AgentF::cmdRcf()
{
  // Command looks like
  // agentf rcf mycommands.txt

  if(mCmd->isMissingParms(1))
  {
    if(mCmd->printThisWay("<FileName>")) return;

    mCmd->printComment(tr("Lines begin with an asterisk are ignored."));
    mCmd->aided();
    return;
  }

  // Don't execute some commands which could make trouble
  bool saveIamEvil = mIamEvil;
  mIamEvil = true;

  QString fileName = mCmd->argStr(1);

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    error(FUNC, tr("Can't open file: %1").arg(fileName));
    return;
  }

  // Read the commands from file
  QTextStream in(&file);
  int lnNo = 0;
  while (!in.atEnd())
  {
    QString line = in.readLine();
    ++lnNo;
    QStringList cmd;

    if(!lineToCommand(line, cmd)) continue;

    cmd.prepend("RCF"); // Add the "caller", normaly is here "agentf" placed
    exec(cmd);
    if(hasError())
    {
      error(FUNC, tr("Error after executing line %1 of file %2").arg(lnNo).arg(fileName));
      break;
    }
  }

  file.close();
  mIamEvil = saveIamEvil;
}

void AgentF::beEvil()
{
  if(mIamEvil) return; // Don't do stupid things

  mIamEvil = true;

  // Command looks like
  // agentf daemon [<Name>]

  if(mCmd->isMissingParms())
  {
    if(mCmd->printThisWay("[<Name>]")) return;

    mCmd->printComment(tr("The optional name is only used at message logging."));
    mCmd->aided();
    return;
  }

  if(verboseLevel(eMax))
  {
    setMsgTargetFormat(eVerbose, QString("%D %T %C %1: %F %x").arg(mCmd->argStr(1)));
    setMsgTargetFormat(eConsLog, QString("%D %T %C %1: *** %t *** %F %x").arg(mCmd->argStr(1)));
    setMsgTargetFormat(eRecord,  QString("%D %T %C %1: %F %x").arg(mCmd->argStr(1)));
  }
  else
  {
    setMsgTargetFormat(eVerbose, QString("%D %T %C %1: %x").arg(mCmd->argStr(1)));
    setMsgTargetFormat(eConsLog, QString("%D %T %C %1: *** %t *** %x").arg(mCmd->argStr(1)));
    setMsgTargetFormat(eRecord,  QString("%D %T %C %1: %x").arg(mCmd->argStr(1)));
  }

  setNoFileLogging();
  mFilu->setNoErrorLogging();

  QTextStream console(stdout);
  QTextStream in(stdin);
  while (!in.atEnd())
  {
    console << "[READY] (Ctrl+D or \"quit\" for quit)" << endl;
    QString line = in.readLine();
    QStringList cmd;
    if(!lineToCommand(line, cmd)) continue;
    if(cmd.at(0) == "quit") break;

    cmd.prepend("DAEMON"); // Add the "caller", normaly is here "agentf" placed
    exec(cmd);
  }

  mQuit = true;
}

void AgentF::exxport()
{
  if(!mExporter) mExporter = new Exporter(this);

  mExporter->exec(mCmd);
  addErrors(mExporter->errors());
}

void AgentF::scan()
{
  // Command list looks like
  // agentf scan --group all --indi Watchdog

  if(!mScanner) mScanner = new Scanner(this);

  mScanner->exec(mCmd);
  addErrors(mScanner->errors());
}

void AgentF::depots()
{
  // Command list looks like
  // agentf depots --check

  Depots* depots = new Depots(this);

  depots->exec(mCmd);

  addErrors(depots->errors());

  delete depots;
}

void AgentF::cmdExec(const QString& cmd)
{
  CmdClass* cmdClass;

  if(mCmds.contains(cmd))
  {
    cmdClass = mCmds.value(cmd);
  }
  else
  {
    cmdClass = CmdClass::createNew(cmd, this);
    mCmds.insert(cmd, cmdClass);
  }

  cmdClass->exec(mCmd);
  addErrors(cmdClass->errors());
}

void AgentF::deleteBars()
{
  // Command looks like
  // agentf deleteBars <Symbol> <Market> <FromDate> [<ToDate>]

  if(mCmd->isMissingParms(3))
  {
    if(mCmd->printThisWay("<Symbol> <Market> <FromDate> [<ToDate>]")) return;

    mCmd->printComment(tr("Use dot-dot '..' for most old/new date. "
                          "Without <ToDate> only one bar will delete."));

    mCmd->printForInst("AAPL NewYork 2012-01-01 2012-06-01");
    mCmd->printForInst("AAPL NewYork 2012-01-01 ..");
    mCmd->aided();
    return;
  }

  QDate fromDate = mCmd->argDate(3, QDate(1000, 1, 1));
  QDate toDate   = mCmd->argDate(4, fromDate, QDate(3000, 1, 1));

  if(mCmd->hasError()) return;

  mFilu->setSqlParm(":symbol",   mCmd->argStr(1));
  mFilu->setSqlParm(":market",   mCmd->argStr(2));
  mFilu->setSqlParm(":fromDate", fromDate.toString(Qt::ISODate));
  mFilu->setSqlParm(":toDate",   toDate.toString(Qt::ISODate));

  QSqlQuery* query = mFilu->execSql("DeleteBars");
  if(check4FiluError(FUNC)) return;

  int nra = query->numRowsAffected();
  if(!nra) warning(FUNC, tr("NO bars deleted!"));
  else verbose(FUNC, tr("%1 bars deleted.").arg(nra));
}

void AgentF::splitBars()
{
  // Command looks like
  // agentf splitBars <Symbol> <Market> <FromDate> <ToDate> <SplitPre:Post>

  if(mCmd->isMissingParms(5))
  {
    if(mCmd->printThisWay("<Symbol> <Market> <FromDate> <ToDate> <SplitPre:Post>")) return;

    mCmd->printNote(tr("To accomplish a split event use the 'add split' command."));
    mCmd->printForInst("KO NewYork 2011-06-01 2012-08-13 1:2");
    mCmd->aided();
    return;
  }

  double  pre;
  double  post;
  double  ratio;
  bool    ok;

  QStringList sl = mCmd->argStr(5).split(":");
  if(sl.size() < 2)
  {
    error(FUNC, "Ratio must be <SplitPre:Post>.");
    return;
  }

  pre = sl[0].toDouble(&ok);
  if(pre == 0.0)
  {
    error(FUNC, "<SplitPre:Post>, Pre must not 0.");
    return;
  };

  post = sl[1].toDouble(&ok);
  if(post == 0.0)
  {
    error(FUNC, "<SplitPre:Post>, Post must not 0.");
    return;
  };

  ratio = pre / post;

  mFilu->setSqlParm(":fromDate", mCmd->argDate(3).toString(Qt::ISODate));
  mFilu->setSqlParm(":toDate",   mCmd->argDate(4).toString(Qt::ISODate));

  if(mCmd->hasError()) return;

  mFilu->setSqlParm(":symbol",   mCmd->argStr(1));
  mFilu->setSqlParm(":market",   mCmd->argStr(2));
  mFilu->setSqlParm(":ratio",    ratio);
  mFilu->setSqlParm(":quality",  0);

  QSqlQuery* query = mFilu->execSql("SplitBars");
  if(check4FiluError(FUNC)) return;

  int nra = query->numRowsAffected();
  if(!nra) warning(FUNC, tr("NO bars adjusted!"));
  else verbose(FUNC, tr("%1 bars adjusted.").arg(nra));
}

void AgentF::cmdFetch()
{
  if(mCmd->isMissingParms(1))
  {
    if(mCmd->printThisWay("<FuzzyString> [<FromDate> [<ToDate>]]")) return;

    mCmd->printComment(tr("The FuzzyString can be part of the FI name or symbol. All matching FIs will updated. "
                          "Without a given date, or if dot-dot '..', AgentF take a look at the database "
                          "which bars could be missing."));

    mCmd->printForInst("apple");
    mCmd->printForInst("aapl 2007-04-01");
    mCmd->aided();
    return;
  }

  QDate fromDate = mCmd->argDate(2, QDate(1000, 1, 1));
  QDate toDate   = mCmd->argDate(3, QDate::currentDate());

  if(mCmd->hasError()) return;

  QSqlQuery* query = mFilu->searchFi(mCmd->argStr(1), ""); // The empty string "" says: All kind of fType

  if(!query)
  {
    check4FiluError(FUNC);
    return;
  }

  if(!query->size())
  {
    verbose(FUNC, tr("No FIs match '%1'.").arg(mCmd->argStr(1)));
    return;
  }

  if(query->size() > 1)
  {
    verbose(FUNC, tr("Found %1 provider symbol(s) to use for eodBar update.").arg(query->size()));
  }

  while(query->next())
  {
    verbose(FUNC, tr("Update bars of %1 for %2").arg(query->value(6).toString()     // Market
                                                   , query->value(3).toString()));  // FiName

    addEODBarData(query->value(5).toString()  // Symbol
                , query->value(6).toString()  // Market
                , query->value(7).toString()  // Provider
                , fromDate
                , toDate
                , query->value(0).toInt()     // FiId
                , query->value(1).toInt());   // MarketId
  }
}

void AgentF::exec(const QStringList& parm)
{
  if(mFilu->hasError()) return;

  // mCmd->regCmds("this...") is done in ctor !
  // mCmd->regOpts("verbose");

  if(mCmd->cmdLineLooksBad(parm)) return;

  if(mCmd->hasOpt("verbose")) setVerboseLevel(FUNC, mCmd->cmdLine());

  if(mCmd->wantHelp())
  {
    mCmd->inCmdBrief("this", tr("Download eod bars of one defined FI"));
    mCmd->inCmdBrief("full", tr("Download eod bars of all FIs"));
    mCmd->inCmdBrief("rcf", tr("Read Command File. The file can contain each command supported by AgentF"));
    mCmd->inCmdBrief("daemon", tr("Is not a daemon as typical known. It is very similar to rcf"));
    mCmd->inCmdBrief("deleteBars", tr("Delete one or a range of eod bars of one FI"));
    mCmd->inCmdBrief("splitBars", tr("To correct faulty data of the provider"));
    mCmd->inCmdBrief("info", tr("Print some settings and more"));
    mCmd->inCmdBrief("fetch", tr("To fetch data (currently only eodBars) from providers"));

    CmdClass::allBriefIn(mCmd);

    Depots::briefIn(mCmd);
    Scanner::briefIn(mCmd);
    Exporter::briefIn(mCmd);

    mCmd->inOptBrief("verbose", "<Level>"
                              , tr("How talkative has it to be. Level can be 0-3 or "
                                   "Quiet Info Ample Max"));
  }
  else
  {
    QStringList cmd = parm;
    cmd.removeAt(0);
    record(FUNC, tr("Exec: %1").arg(cmd.join(" ")));
  }

  if(mCmd->needHelp(1))
  {
    if(mCmd->printThisWay("<Command> [<ParmList>] [<SubCommand> [<ParmList>]]..")) return;

    mCmd->printComment(tr("Calling a command without any parameter may give a hint like --help."));
    mCmd->printComment(tr("But of cause not if no parameter is needed e.g at 'full'."));
    mCmd->aided();
    return;
  }

  // Look for each known command and call the related function
  if(mCmd->hasCmd("this"))               addEODBarData();
  else if(mCmd->hasCmd("full"))          updateAllBars();
  else if(mCmd->hasCmd("rcf"))           cmdRcf();
  else if(mCmd->hasCmd("imp"))           cmdExec("Imp");
  else if(mCmd->hasCmd("exp"))           exxport();
  else if(mCmd->hasCmd("scan"))          scan();
  else if(mCmd->hasCmd("depots"))        depots();
  else if(mCmd->hasCmd("add"))           cmdExec("Add");
  else if(mCmd->hasCmd("daemon"))        beEvil();
  else if(mCmd->hasCmd("db"))            cmdExec("DB");
  else if(mCmd->hasCmd("deleteBars"))    deleteBars();
  else if(mCmd->hasCmd("splitBars"))     splitBars();
  else if(mCmd->hasCmd("sum"))           cmdExec("Summon");
  else if(mCmd->hasCmd("exo"))           cmdExec("Exorcise");
  else if(mCmd->hasCmd("set"))           cmdExec("Set");
  else if(mCmd->hasCmd("fetch"))         cmdFetch();
  else if(mCmd->hasCmd("info"))
  {
    if(verboseLevel(eMax)) return; // Already printed, don't print twice
    else printSettings();
  }
  else
  {
    fatal(FUNC, QString("Unsupported command: %1").arg(mCmd->cmd()));
  }
}

void AgentF::printSettings()
{
  mFilu->printSettings();

  print(tr("AgentF Infos:"));

  QString txt = "  %1 = %2";
  int width = -20; // Negative value = left-aligned
  print(txt.arg("Using muParser", width).arg(mu::Parser().GetVersion().data()));
  print("");

  print(tr("AgentF Config Keys:"));
  print(txt.arg("MaxClones", width).arg(mRcFile->getIT("MaxClones")));
  print(txt.arg("ProviderPath", width).arg(mRcFile->getST("ProviderPath")));
  print(txt.arg("Verbose", width).arg(verboseLevel()));
  print("");
}

void AgentF::startClones()
{
  int maxClones = mRcFile->getIT("MaxClones");

  if((mCommands.size() - 1) < maxClones)
  {
    // Don't create more clones as useful
    // One less since we have at least one MASTER_CMD
    // FIXME: That could be differend sometimes, calc/check it smarter
    maxClones = mCommands.size() - 1;
  }

  QString logFile = mRcFile->getST("LogFile");

  for(int i = 0; i < maxClones; ++i)
  {
    QProcess* clone = new QProcess;
    clone->setReadChannel(QProcess::StandardOutput);
    clone->setStandardErrorFile(logFile, QIODevice::Append);
    //clone->setProcessChannelMode(QProcess::MergedChannels);
    connect(clone, SIGNAL(readyRead()), this, SLOT(cloneIsReady()));
    connect(clone, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(cloneHasFinished()));

    mCloneNames.append(QString::number(i + 1));
    QString cmd = QString("%1 %2 %3").arg(QCoreApplication::applicationFilePath(), "daemon").arg(mCloneNames.at(i));
    if(mCmd->hasOpt("verbose")) cmd.append(" --verbose " + mCmd->optStr("verbose"));
    if(mConfigParms.size()) cmd.append(" --config " + mConfigParms.join(" "));
    clone->start(cmd);

    if(!clone->waitForStarted())
    {
      fatal(FUNC, QString("Clone not started."));
      clone->kill();
      delete clone;
      QCoreApplication::exit(1);
      return;
    }

    mClones.append(clone);
    verbose(FUNC, tr("Clone %1 started.").arg(mCloneNames.at(i)), eInfo);
    //qDebug() << QCoreApplication::hasPendingEvents (); always true, I give up :-(
    //QCoreApplication::flush();// Do not take effect
    //QCoreApplication::sendPostedEvents();// Do not take effect
    //QCoreApplication::processEvents(); // Do not take effect
    //cloneIsReady(); // Do not work
  }
}

void AgentF::cloneIsReady() // Slot
{
  // Search the clone waiting for a job
  QProcess* clone = 0;
  QString   cloneName;
  for(int i = 0; i < mClones.size(); ++i)
  {
    if(mClones.at(i)->bytesAvailable() == 0) continue;

    clone = mClones.at(i);
    cloneName = mCloneNames.at(i);
    break;
  }

  if(!clone)
  {
    fatal(FUNC, "Curious, no clone found.");
    return; // No more todo
  }

  QString text(clone->readAllStandardOutput());
  //qDebug() << "AgentF::cloneIsReady: text :" << text;

  if(!text.contains("[READY]")) return;

  QString feedTxt = tr("Feed clone %1: ").arg(cloneName);

  // Feed the clone
  if(mCommands.size() > 0)
  {
    QString cmd = mCommands.at(0).join(" ");
    verbose(FUNC, feedTxt + cmd.toUtf8(), eInfo);

    cmd.append("\n");
    clone->write(cmd.toUtf8());

    mCommands.removeAt(0);
  }
  else
  {
    // Clean up and exit
    verbose(FUNC, feedTxt + "quit");
    clone->write("quit\n");
  }

  check4MasterCMD();
}

void AgentF::check4MasterCMD()
{
  if(mCommands.size() < 1) return;
  if(mCommands.at(0).at(0) != "MASTER_CMD") return;

  if(mCommands.at(0).at(1) == "MarkScanned")
  {
    if(!mScanner) mScanner = new Scanner(this);
    mScanner->setVerboseLevel(eNoVerbose);
    mScanner->autoSetup();
    mScanner->mark();
  }
  else
  {
    fatal(FUNC, QString("Unknown MasterCMD: %1").arg(mCommands.at(0).at(1)));
  }

  mCommands.removeAt(0);
}

void AgentF::cloneHasFinished() // Slot
{
  // Search the/a finished clone
  for(int i = 0; i < mClones.size(); ++i)
  {
    if(mClones.at(i)->state() != QProcess::NotRunning) continue;

    //qDebug() << "AgentF::cloneHasFinished: found" << i;
    delete mClones.at(i);
    mClones.removeAt(i);
    mCloneNames.removeAt(i);
    break;
  }

  if(mClones.size() == 0)
  {
    quit();
  }
}
