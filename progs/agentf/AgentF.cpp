//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011  loh.tar@googlemail.com
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

#include "AgentF.h"

#include "Script.h"
#include "Importer.h"
#include "Exporter.h"
#include "Scanner.h"
#include "Depots.h"
#include "CmdAdd.h"

AgentF::AgentF(QCoreApplication& app)
      : FCoreApp("AgentF", app)
      , mScript(0)
      , mImporter(0)
      , mExporter(0)
      , mScanner(0)
      , mQuit(true)
      , mIamEvil(false)
{
  readSettings();
  setMsgTargetFormat(eVerbose, "%C: %x");

  execCmd(mCommandLine);

  QTimer::singleShot(500, this, SLOT(run()));
}

AgentF::~AgentF()
{
  if(mScript)   delete mScript;
  if(mImporter) delete mImporter;
  if(mExporter) delete mExporter;
  if(mScanner)  delete mScanner;

  verbose(FUNC, tr("Done."), eEver);
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
  QCoreApplication::exit(0);
}

void AgentF::readSettings()
{
  if(mRcFile->getIT("DebugLevel")) printSettings();
}

bool AgentF::dateIsNotValid(QString& date)
{
  if(date == "auto") return false;  // We accept "auto" as valid
  if(QDate::fromString(date, Qt::ISODate).isValid()) return false;

  error(FUNC, tr("Bad date: %1").arg(date));
  return true;
}

QStringList* AgentF::fetchBarsFromProvider(const QString& provider,
                                   const QStringList& parameters)
{
  // Call a script which download bar data from a provider.
  // Returns the data as it is delivered by the script as QStringList.
  // Each line in the list mußt have the format:
  // "Date, Open, High, Low, Close, Volume, OpenInterest, Quality"

  if(!mScript) mScript = new Script(this);

  QStringList* data = mScript->askProvider(provider, "fetchBar", parameters);

  return data;
}

void AgentF::addEODBarData(const QStringList& parm)
{
  //
  // This function update the bars of one defined FI.
  // parm list looks like:
  // <caller> this <symbol> <market> <provider> [<fromDate> [<toDate>]]

  if(parm.size() == 9)
  {
    addEODBarDataFull(parm);
    return;
  }

  QStringList parameters = parm; // Parameter list for addEODBarDataFull(...)

  QString fromDate = "auto";
  QString toDate   = "auto";

  if(parm.size() == 5)
  {
    parameters << fromDate << toDate;
  }
  else if(parm.size() == 6)
  {
    fromDate = parm[5];
    parameters << toDate;
  }
  else
  {
    fromDate = parm[5];
    toDate = parm[6];
  }

  if(dateIsNotValid(fromDate)) return;
  if(dateIsNotValid(toDate)) return;

  // We need fiId and marketId
  // parm[2]=<symbol>, parm[3]=<market>, parm[4]=<provider>
  SymbolTuple* st = mFilu->searchSymbol(parm[2], parm[3], parm[4]);
  if(!st)
  {
    error(FUNC, tr("Symbol not found: %1, %2, %3").arg(parm[2], parm[3], parm[4]));
    return;
  }

  st->next();

  parameters.append(QString::number(st->fiId()));
  parameters.append(QString::number(st->marketId()));

  delete st;

  addEODBarDataFull(parameters);
}

void AgentF::addEODBarDataFull(const QStringList& parm)
{
  //
  // This function update the bars of one defined FI.
  // parm list looks like:
  // <caller> this <symbol> <market> <provider> <fromDate> <toDate> <fiId> <marketId>
  //    0      1      2        3         4          5         6       7        8

  if(parm.size() < 9)
  {
    error(FUNC, "addEODBarDataFull: To less arguments."); // No tr(), should never happens
    return;
  }

  QString fromDate = parm.at(5);
  QString toDate   = parm.at(6);
  int fiId         = parm.at(7).toInt();
  int marketId     = parm.at(8).toInt();

  // Build the parameter list needed by the script
  QStringList parameters;

  DateRange dateRange;
  if(fromDate == "auto")
  {
    mFilu->getEODBarDateRange(dateRange, fiId, marketId, Filu::eBronze);
    // FIXME: To find out if anything is todo we have to check more smarter.
    //        We have to use the 'offday' table, but its not implemented yet.
    QDate date = dateRange.value("last").addDays(1);
    if(date.dayOfWeek() == Qt::Saturday) date.addDays(2);
    if(date.dayOfWeek() == Qt::Sunday) date.addDays(1);

    if(date > QDate::currentDate())
    {
      verbose(FUNC, tr("Nothing todo, last bar is up to date: %1").arg(date.toString(Qt::ISODate)), eInfo);
      return;
    }

    parameters.append(date.toString(Qt::ISODate));
  }
  else
  {
    // 26.2.'10 mFilu->getEODBarDateRange(dateRange, Filu::Approved);
    mFilu->getEODBarDateRange(dateRange, fiId, marketId, Filu::eBronze);
    // Avoid 'holes' in the data table
    QDate date = QDate::fromString(fromDate, Qt::ISODate);
    if(date > dateRange.value("last")) date = dateRange.value("last").addDays(1);
    parameters.append(date.toString(Qt::ISODate));
  }

  if(toDate == "auto")
  {
    parameters.append(QDate::currentDate().toString(Qt::ISODate));
  }
  else
  {
    // Once more, avoid 'holes' in the data table
    QDate date = QDate::fromString(toDate, Qt::ISODate);
    if(date < dateRange.value("first")) date = dateRange.value("first").addDays(-1);
    parameters.append(date.toString(Qt::ISODate));
  }

  parameters.append(parm[2]);// parm[2]=<symbol>
  parameters.append(parm[3]);// parm[3]=<market>

  //qDebug() << parameters;
  // parameters should now looks like "<fromDate> <toDate> <symbol> <market>"
  // Puh...we can call the script
  QStringList* data = fetchBarsFromProvider(parm[4], parameters); // parm[4]=<provider>

  if(!data)
  {
    warning(FUNC, tr("No data from script."));
    return;
  }

  // Here is the beef...
  mFilu->addEODBarData(fiId, marketId, data);
  delete data; // No longer needed

  // ...and as dessert check for events
  if(!mScanner)
  {
    mScanner = new Scanner(this);
    mScanner->autoSetup();
  }

  mScanner->scanThis(fiId, marketId);
}

void AgentF::updateAllBars(const QStringList& parm)
{
  // parm list looks like
  // agentf full [<fromDate>] [<toDate>]

  QString fromDate("auto");
  if(parm.size() > 2) fromDate = parm[2];
  if(dateIsNotValid(fromDate)) return;

  QString toDate(QDate::currentDate().toString(Qt::ISODate));
  if(parm.size() > 3) toDate = parm[3];
  if(dateIsNotValid(toDate)) return;

  SymbolTuple* symbols = mFilu->getAllProviderSymbols();
  if(!symbols)
  {
    warning(FUNC, tr("No provider symbols found."));
    QCoreApplication::exit(1);
    return;
  }

  // Build the parameter list needed by addEODBarData()
  // <caller> -this <symbol> <market> <provider> [<fromDate> [<toDate>]]
  QStringList parameters;
  // No parameters.append("<caller>"); done later by beEvil()
  parameters.append("this");
  parameters.append(""); // Placeholders, will fill below
  parameters.append("");
  parameters.append("");
  parameters.append(fromDate);
  parameters.append(toDate);
  parameters.append("");
  parameters.append("");

  verbose(FUNC, tr("Processing..."), eInfo);

  while (symbols->next())
  {
    parameters[1] = symbols->caption();
    parameters[2] = symbols->market();
    parameters[3] = symbols->owner();
    parameters[6] = QString::number(symbols->fiId());
    parameters[7] = QString::number(symbols->marketId());

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

void AgentF::readCommandFile(const QStringList& parm)
{

  // parm list looks like
  // agentf rcf mycommands.txt

  if(parm.count() != 3)
  {
    error(FUNC, tr("rcf: Wrong parameter count."));
    return;
  }

  QFile file(parm[2]);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    error(FUNC, tr("rcf: Can't open file: %1").arg(parm[2]));
    return;
  }

  // Read the commands from file
  QTextStream in(&file);
  while (!in.atEnd())
  {
    QString line = in.readLine();
    QStringList cmd;

    if(!lineToCommand(line, cmd)) continue;

    cmd.prepend("RCF"); // Add the "caller", normaly is here "agentf" placed
    execCmd(cmd);
  }

  file.close();
}

void AgentF::beEvil(const QStringList &/*parm*/) //FIXME: improvemts possible to use parm?
{
  if(mIamEvil) return; // Don't do stupid things

  mIamEvil = true;

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
    execCmd(cmd);
  }

  mQuit = true;
}

void AgentF::import(const QStringList& parm)
{
  QTextStream* in;
  QFile* file = 0;

  if(parm.count() > 2)
  {
    file = new QFile(parm[2]);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
      error(FUNC, tr("imp: Can't open file: %1").arg(parm[2]));
      return;
    }

    in = new QTextStream(file);
  }
  else
  {
    in = new QTextStream(stdin);
  }

  if(!mImporter) mImporter = new Importer(this);

  while (!in->atEnd())
  {
    QString line = in->readLine();
    if(!mImporter->import(line)) break;
  }

  if(file)
  {
    file->close();
    delete file;
  }

  delete in;
}

void AgentF::exxport(const QStringList& parm)
{
  // parm list looks like
  // agentf exp -useGroup favorites -fiNames -symbols -eodRaw -split

  // Remove "agentf" and "exp"
  QStringList parm2(parm);
  parm2.removeAt(0);
  parm2.removeAt(0);

  if(!mExporter) mExporter = new Exporter(this);

  mExporter->exxport(parm2);
}

void AgentF::scan(const QStringList& parm)
{
  // parm list looks like
  // agentf scan --group all --indi Watchdog

  if(!mScanner) mScanner = new Scanner(this);

  mScanner->exec(parm);

  if(mScanner->hasError()) addErrors(mScanner->errors());
}

void AgentF::depots(const QStringList& parm)
{
  // parm list looks like
  // agentf depots --check

  Depots* depots = new Depots(this);

  depots->exec(parm);

  if(depots->hasError()) addErrors(depots->errors());
}

void AgentF::filu(const QStringList& parm)
{
  // parm list looks like
  // agentf filu --update
  // agentf filu --create [--db <db-name>] [--schema <schema>]

  if(parm.contains("--update"))
  {
    mFilu->createFunctions();
    if(!mFilu->hasError())
    {
      verbose(FUNC, tr("Database user functions successful updated."));
    }
  }
  else if(parm.contains("--create"))
  {
    verbose(FUNC, tr("Not yet implemented, feel free to FIXME."));
  }
  else
  {
    verbose(FUNC, tr("What was that? :o)"));
  }
}

void AgentF::cmdAdd(const QStringList& parm)
{
  CmdAdd* cmdAdd = new CmdAdd(this);
  cmdAdd->exec(parm);
  delete cmdAdd;
  return;
}

void AgentF::execCmd(const QStringList& parm)
{
  if(mFilu->hasError()) return;

  if(parm.size() == 1)
  {
    printUsage();
    return;
  }

  const QString cmd(parm.at(1));

  // Look for each known command and call the related function
  if(cmd == "this")          addEODBarData(parm);
  else if(cmd == "full")     updateAllBars(parm);
  else if(cmd == "rcf")      readCommandFile(parm);
  else if(cmd == "imp")      import(parm);
  else if(cmd == "exp")      exxport(parm);
  else if(cmd == "scan")     scan(parm);
  else if(cmd == "depots")   depots(parm);
  else if(cmd == "add")      cmdAdd(parm);
  else if(cmd == "daemon")   beEvil(parm);
  else if(cmd == "filu")     filu(parm);
  else if(cmd == "info")
  {
    if(mRcFile->getIT("DebugLevel")) return; // Already printed, don't print twice
    else printSettings();
  }
  else
  {
    error(FUNC, tr("Unknown command: %1").arg(cmd));
    errInfo(FUNC, tr("Call me without any command for help."));
  }
}

void AgentF::printUsage()
{
  print(tr("Hello! I'm part of Filu. Please call me this way:"));
  print("");
  print("  agentf <command> [<parameterList>] (see doc/first-steps.txt)");
  print("");
  print("  agentf add <dataType> [<parameterList>]");
  print("    agentf add (tells you more)");
  print("");
  print("  agentf daemon");
  print("  agentf depots <parameterList>");
  print("    agentf depots --check");
  print("");
  print("  agentf exp <parameterList> (see doc/export-data.txt)");
  print("  agentf filu <parameterList>");
  print("  agentf full [<fromDate> [<toDate>]]");
  print("    agentf full 2001-01-01");
  print("");
  print("  agentf imp [<fileName>] (if no fileName is given then will read from stdin)");
  print("  agentf info");
  print("  agentf rcf <fileName>");
  print("  agentf scan <parameterList>");
  print("    agentf scan --group all --indi MyNewIdea --verbose Info");
  print("    agentf scan --group all --auto --force --timeFrame Quarter");
  print("");
  print("  agentf this <symbol> <market> <provider> [<fromDate> [<toDate>]]");
  print("    agentf this AAPL NYSE Yahoo");
  print("    agentf this AAPL NYSE Yahoo 2007-04-01");
  print("");
}

void AgentF::printSettings()
{
  QString txt = "%1 = %2";
  int width = -15; // Negative value = left-aligned
  print(tr("AgentF settings are:"));
  print(txt.arg("Using QtVersion", width).arg(qVersion()));
  print(txt.arg("Settings file", width).arg(mRcFile->fileName()));
  print(txt.arg("Fallback file", width).arg("/etc/xdg/Filu.conf")); //FIXME: how to make system independent?
  print(txt.arg("ProviderPath", width).arg(mRcFile->getST("ProviderPath")));
  print(txt.arg("MaxClones", width).arg(mRcFile->getIT("MaxClones")));
  print(txt.arg("DebugLevel", width).arg(mRcFile->getIT("DebugLevel")));
  print(txt.arg("LogFile", width).arg(mRcFile->getST("LogFile")));

  mFilu->printSettings();
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

    QString cmd = QString("%1 %2").arg(QCoreApplication::applicationFilePath(), "daemon");
    if(mFiluParms.size()) cmd.append(" --Filu " + mFiluParms.join(" "));
    clone->start(cmd);

    if(!clone->waitForStarted())
    {
      fatal(FUNC, tr("Clone not started."));
      clone->kill();
      delete clone;
      QCoreApplication::exit(1);
      return;
    }

    mClones.append(clone);
    mCloneNames.append(QString::number(i + 1));
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
    error(FUNC, "Curious, no clone found."); // No tr(), should never happens
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
    mScanner->autoSetup();
    mScanner->mark();
    delete mScanner;
    mScanner = 0;
  }
  else
  {
    error(FUNC, "Unknown MasterCMD: " + mCommands.at(0).at(1)); // No tr(), should never happens
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
