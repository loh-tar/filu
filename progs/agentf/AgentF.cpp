//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010  loh.tar@googlemail.com
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

AgentF::AgentF(QCoreApplication& app)
      : FCoreApp("AgentF", app)
      , mScript(0)
      , mImporter(0)
      , mExporter(0)
      , mScanner(0)
      , mQuit(true)
      , mIamEvil(false)
      , mConsole(stdout)
      , mErrConsole(stderr)
{
  readSettings();

  execCmd(mCommandLine);

  QTimer::singleShot(500, this, SLOT(run()));
}

AgentF::~AgentF()
{
  if(mScript)   delete mScript;
  if(mImporter) delete mImporter;
  if(mExporter) delete mExporter;
  if(mScanner)  delete mScanner;
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
  mConsole << "agentf: Done\n";
  QCoreApplication::exit(0);
}

void AgentF::check4FiluError(const QString message)
{
  if(mFilu->hadTrouble())
  {
    printError(message);
    printError(mFilu->errorText());
  }
}

void AgentF::printError(const QString message)
{
  QStringList errLines = message.split("\n");
  for(int i = 0; i < errLines.size(); ++i)
    mErrConsole << "agentf: " << errLines.at(i) << endl;
}

void AgentF::readSettings()
{
  mMaxClones   = mRcFile->getIT("MaxClones");
  mLogFile     = mRcFile->getST("LogFile");

  QString home = QDir::homePath();

  mLogFile.replace("~", home);

  if(mRcFile->getIT("DebugLevel")) printSettings();
}

bool AgentF::dateIsNotValid(QString& date)
{
  if(date == "auto") return false;  // We accept "auto" as valid
  if(QDate::fromString(date, Qt::ISODate).isValid()) return false;

  printError("Bad date: " + date);
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
    printError(QString("Symbol not found: %1, %2, %3").arg(parm[2], parm[3], parm[4]));
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
    printError("addEODBarDataFull: To less arguments.");
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
      printError("Nothing todo, last bar is up to date: " + date.toString(Qt::ISODate));
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
    printError("No data from script");
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
    printError("No provider symbols found.");
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

  printError("Processing...");

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

void AgentF::addFi(const QStringList& parm)
{
  // parm list looks like
  // agentf addFi <longName> <fiType> <symbol> <market> <symbolType> [<symbol> <market> <symbolType> ...]

  if(((parm.count() - 4) % 3 > 0) or parm.count() < 7)
  {
    printError("addFi: Wrong parameter count");
    return;
  }

  int symbolCount = (parm.count() - 4) / 3;

  FiTuple fi(1);
  SymbolTuple* symbol = new SymbolTuple(symbolCount);

  fi.next(); // Set on first position

  fi.setSymbol(symbol);
  fi.setName(parm[2]);
  fi.setType(parm[3]);

  int i = 4;
  while(symbol->next())
  {
    symbol->setCaption(parm[i]);    // 4, 7, 10...
    symbol->setMarket(parm[i + 1]); // 5, 8, 11...
    symbol->setOwner(parm[i + 2]);  // 6, 9, 12...
    i += 3;
  }

  // Here is the beef
  mFilu->addFiCareful(fi);

  check4FiluError("FAIL! FI not added.");
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
    printError("rcf: Wrong parameter count");
    return;
  }

  QFile file(parm[2]);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    printError("rcf: Can't open file: " + parm[2]);
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

  QTextStream in(stdin);
  while (!in.atEnd())
  {
    mConsole << "[READY] (Ctrl+D or \"quit\" for quit)\n";
    mConsole.flush();
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
      printError("imp: Can't open file:" + parm[2]);
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

  if(mScanner->hasError()) printError(mScanner->errorText().join("\n"));
}

void AgentF::addSplit(const QStringList& parm)
{
  // parm list looks like
  // agentf addSplit AAPL 2005-02-28 1:2
  // agentf addSplit AAPL 2005-02-28 1:2 1

  // Remove "agentf" and "addSplit"
  QStringList parm2(parm);
  parm2.removeAt(0);
  parm2.removeAt(0);

  QString header = "[Header]RefSymbol;SplitDate;SplitPre:Post";

  if(parm2.size() == 4)
  {
    header.append(";Quality");
  }

  QString data = parm2.join(";");

  if(!mImporter) mImporter = new Importer(this);

  if(!mImporter->import(header)) return;
  if(!mImporter->import(data)) return;

}

void AgentF::execCmd(const QStringList& parm)
{
  if(parm.size() == 1)
  {
    printUsage();
    return;
  }

  const QString cmd(parm.at(1));

  // Look for each known command and call the related function
  if(cmd == "this")          addEODBarData(parm);
  else if(cmd == "full")     updateAllBars(parm);
  else if(cmd == "addFi")    addFi(parm);
  else if(cmd == "rcf")      readCommandFile(parm);
  else if(cmd == "imp")      import(parm);
  else if(cmd == "exp")      exxport(parm);
  else if(cmd == "scan")     scan(parm);
  else if(cmd == "addSplit") addSplit(parm);
  else if(cmd == "daemon")   beEvil(parm);
  else if(cmd == "printSettings")
  {
    if(mRcFile->getIT("DebugLevel")) return; // Already printed, don't print twice
    else printSettings();
  }
  else
  {
    printError("Unknown command: " + cmd);
    printError("Call me without any command for help.");
  }
}

void AgentF::printUsage()
{
  qDebug() << "agentf: Hello! I'm part of Filu. Please call me this way:";
  qDebug();
  qDebug() << "  agentf <command> [<parameter list>]";
  qDebug();
  qDebug() << "  agentf this <symbol> <market> <provider> [<fromDate> [<toDate>]]";
  qDebug() << "    agentf this AAPL NYSE Yahoo";
  qDebug() << "    agentf this AAPL NYSE Yahoo 2007-04-01";
  qDebug();
  qDebug() << "  agentf full [<fromDate>] [<toDate>]";
  qDebug() << "    agentf full 2001-01-01";
  qDebug();
  qDebug() << "  agentf scan <parameter list> (see doc/first-steps.txt)";
  qDebug() << "    agentf scan --group all --indi MyNewIdea --verbose Info";
  qDebug() << "    agentf scan --group all --auto --force --timeFrame Quarter";
  qDebug();
  qDebug() << "  agentf addFi <longName> <fiType> <symbol> <market> <symbolType>[<symbol> <market> <symbolType> ...]";
  qDebug() << "    agentf addFi \"Apple Computer\" Stock AAPL NYSE Yahoo ";
  qDebug();
  qDebug() << "  agentf rcf <fileName>";
  qDebug() << "  agentf imp [<fileName>] (if no fileName is given then will read from stdin)";
  qDebug() << "  agentf exp <parameter list> (see doc/export-data.txt)";
  qDebug() << "  agentf addSplit <symbol> <date> <splitPre:Post>";
  qDebug() << "    agentf addSplit AAPL 2005-02-28 1:2";
  qDebug();
  qDebug() << "  agentf daemon";
  qDebug() << "  agentf printSettings";
  qDebug();
}

void AgentF::printSettings()
{
  mConsole << "AgentF::printSettings: ..."  << endl;
  mConsole << "Settings file\t= "       << mRcFile->fileName() << endl;
  mConsole << "Fallback file\t= /etc/xdg/Filu.conf" << endl; //FIXME: how to make system independent?
  mConsole << "ProviderPath\t= "        << mRcFile->getST("ProviderPath") << endl;
  mConsole << "MaxClones\t= "           << mMaxClones /*mRcFile->getIT("MaxClones")*/ << endl;
  mConsole << "DebugLevel\t= "          << mRcFile->getIT("DebugLevel") << endl;
  mConsole << "LogFile  \t= "           << mLogFile /*mRcFile->getST("LogFile")*/ << endl;

  mFilu->printSettings();
}

void AgentF::startClones()
{
  if(mCommands.size() < mMaxClones)
  {
    // Don't create more clones as useful
    mMaxClones = mCommands.size();
  }

  for(int i = 0; i < mMaxClones; ++i)
  {
    QProcess* clone = new QProcess;
    clone->setReadChannel(QProcess::StandardOutput);
    clone->setStandardErrorFile(mLogFile, QIODevice::Append);
    //clone->setProcessChannelMode(QProcess::MergedChannels);
    connect(clone, SIGNAL(readyRead()), this, SLOT(cloneIsReady()));
    connect(clone, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(cloneHasFinished()));
    clone->start(QCoreApplication::applicationFilePath() + " daemon");
    if(!clone->waitForStarted())
    {
      printError("startClones: Error! Clone not started");
      clone->kill();
      delete clone;
      QCoreApplication::exit(1);
      return;
    }

    mClones.append(clone);
    printError(QString("Clone %1 started").arg(i + 1));
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
  int cloneNumber = 0;
  for(; cloneNumber < mClones.size(); ++cloneNumber)
  {
    if(mClones.at(cloneNumber)->bytesAvailable() == 0) continue;

    clone = mClones.at(cloneNumber);
    break;
  }

  if(!clone)
  {
    printError("cloneIsReady: Curious, no clone found");
    return; // No more todo
  }

  QString text(clone->readAllStandardOutput());
  //qDebug() << "AgentF::cloneIsReady: text :" << text;

  if(text.contains("agentf: Done"))
  {
    return;
  }

  if(!text.contains("[READY]")) return;

  QString feedTxt = QString("Feed clone %1: ").arg(cloneNumber + 1);

  // Feed the clone
  if(mCommands.size() > 0)
  {
    QString cmd = mCommands.at(0).join(" ");
    printError(feedTxt + cmd.toUtf8());

    cmd.append("\n");
    clone->write(cmd.toUtf8());

    mCommands.removeAt(0);
  }
  else
  {
    // Clean up and exit
    printError(feedTxt + "quit");
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
    printError("check4MasterCMD: Unknown cmd: " + mCommands.at(0).at(1));
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
    break;
  }

  if(mClones.size() == 0)
  {
    quit();
  }
}
