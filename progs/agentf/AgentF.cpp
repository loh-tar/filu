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
#include "muParser.h"

AgentF::AgentF(QCoreApplication& app)
      : FCoreApp("AgentF", app)
      , mExporter(0)
      , mScanner(0)
      , mQuit(true)
      , mIamEvil(false)
{
  readSettings();
  setMsgTargetFormat(eVerbose, "%C: %x");
  setMsgTargetFormat(eConsLog, "%C: *** %t *** %x");

  mCmd->regCmds("full rcf exp scan daemon info depots");

  mKnownCmds = CmdClass::allRegCmds(mCmd);

  mCmd->regStdOpts("verbose");
  mCmd->inGreeter("AgentF is part of Filu. Visit http://filu.sourceforge.net");

  exec(mCommandLine);

  QTimer::singleShot(500, this, SLOT(run()));
}

AgentF::~AgentF()
{
  foreach(CmdClass* cmd, mCmds) delete cmd;

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

  // Build the parameter list needed by cmdThis()
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

    // Save the now completed parameter list needed by cmdThis()
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

void AgentF::exec(const QStringList& parm)
{
  if(mFilu->hasError()) return;

  // mCmd->regCmds("this...") is done in ctor !
  // mCmd->regOpts("verbose");

  if(mCmd->cmdLineLooksBad(parm)) return;

  if(mCmd->has("verbose")) setVerboseLevel(FUNC, mCmd->cmdLine());

  if(mCmd->wantHelp())
  {
    mCmd->inCmdBrief("full", tr("Download eod bars of all FIs"));
    mCmd->inCmdBrief("rcf", tr("Read Command File. The file can contain each command supported by AgentF"));
    mCmd->inCmdBrief("daemon", tr("Is not a daemon as typical known. It is very similar to rcf"));
    mCmd->inCmdBrief("info", tr("Print some settings and more"));

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
  if(mKnownCmds.contains(mCmd->cmd()))   cmdExec(mCmd->cmd());
  else if(mCmd->hasCmd("full"))          updateAllBars();
  else if(mCmd->hasCmd("rcf"))           cmdRcf();
  else if(mCmd->hasCmd("exp"))           exxport();
  else if(mCmd->hasCmd("scan"))          scan();
  else if(mCmd->hasCmd("depots"))        depots();
  else if(mCmd->hasCmd("daemon"))        beEvil();
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
