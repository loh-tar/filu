//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
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

#include <QElapsedTimer>
#include <QQueue>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTimer>

#include "AgentF.h"

#include "CmdClass.h"
#include "CmdHelper.h"
#include "Depots.h"
#include "Exporter.h"
#include "FTool.h"
#include "FiluU.h"
#include "RcFile.h"
#include "Scanner.h"
#include "muParser.h"

AgentF::AgentF(QCoreApplication& app)
      : FCoreApp("AgentF", app)
      , mExporter(0)
      , mScanner(0)
      , mRolex(0)
      , mQuit(true)
{
  readSettings();
  setMsgTargetFormat(eVerbose, "%C: %x");
  setMsgTargetFormat(eConsLog, "%C: *** %t *** %x");

  mCmd->regCmds("full exp scan info depots");

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
  if(mRolex)    delete mRolex;

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
  }
  else
  {
    startClone();
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

  QSqlQuery* query = mFilu->execSql("GetActiveFIs");

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

  record(FUNC, tr("\n*\n* Update bar data of %1 FI/Market relations.\n*").arg(query->size()));

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

  mQuit = false; // Don't quit after all, enter main event loop
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
  else if(mCmd->hasCmd("exp"))           exxport();
  else if(mCmd->hasCmd("scan"))          scan();
  else if(mCmd->hasCmd("depots"))        depots();
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
  print(txt.arg("MinJobsPerClone", width).arg(mRcFile->getIT("MinJobsPerClone")));
  print(txt.arg("MaxTimeLag", width).arg(mRcFile->getIT("MaxTimeLag")));
  print(txt.arg("MinTimeLag", width).arg(mRcFile->getIT("MinTimeLag")));
  print(txt.arg("ProviderPath", width).arg(mRcFile->getST("ProviderPath")));
  print(txt.arg("Verbose", width).arg(verboseLevel()));
  print("");
}

void AgentF::startClone()
{
  static const int maxClones = mRcFile->getIT("MaxClones");
  static const int minJobs   = mRcFile->getIT("MinJobsPerClone");

  if(mClones.size() == maxClones or
     mClones.size() * minJobs > mCommands.size())
  {
    return;
  }

  QString logFile = mRcFile->getST("LogFile");

  int i = mClones.size();
  QProcess* clone = new QProcess;
  clone->setReadChannel(QProcess::StandardOutput);
  clone->setStandardErrorFile(logFile, QIODevice::Append);
  //clone->setProcessChannelMode(QProcess::MergedChannels);
  connect(clone, SIGNAL(readyRead()), this, SLOT(cloneIsReady()));
  connect(clone, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(cloneHasFinished()));
  connect(clone, SIGNAL(error(QProcess::ProcessError)), this, SLOT(cloneHasTrouble(QProcess::ProcessError)));

  mCloneNames.append(QString::number(i + 1));
  QString cmd = QString("%1 %2 %3")
                  .arg(QCoreApplication::applicationFilePath(), "do --stdin")
                  .arg(mCloneNames.at(i));

  if(mCmd->hasOpt("verbose")) cmd.append(" --verbose " + mCmd->optStr("verbose"));
  if(mConfigParms.size()) cmd.append(" --config " + mConfigParms.join(" "));

  clone->start(cmd);

  mClones.append(clone);
  verbose(FUNC, tr("Clone %1 started.").arg(mCloneNames.at(i)), eInfo);
}

void AgentF::cloneIsReady() // Slot
{
  if(!mRolex)
  {
    mRolex = new QElapsedTimer;
    mRolex->start();
  }

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

  if(!text.contains("[READY]")) return;

  QString feedTxt = tr("Feed clone %1: ").arg(cloneName);

  // Feed the clone
  if(mCommands.size() > 0)
  {
    bool tooSlow = false;

    if(mCommands.at(0).at(0) == "this")
    {
      static const int minTimeLag = mRcFile->getIT("MinTimeLag");
      static const int maxTimeLag = mRcFile->getIT("MaxTimeLag");
      static QQueue<int> lagLst;

      lagLst.enqueue(mRolex->elapsed());
      if(lagLst.size() > 5) lagLst.dequeue();

      if(mRolex->elapsed() < minTimeLag)
      {
        verbose(FUNC, tr("Don't ask the provider too rapidly. Will sleep %1ms")
                         .arg(minTimeLag - mRolex->elapsed()), eAmple);
        FTool::sleep(minTimeLag - mRolex->elapsed());
      }
      else
      {
        int avgLag = 0;
        foreach(int lag, lagLst) avgLag += lag;
        avgLag /= lagLst.size();

        verbose(FUNC, tr("AvgTimeLag is %1ms").arg(avgLag), eAmple);
        if(avgLag > maxTimeLag)
        {
          tooSlow = true;
        }
      }

      mRolex->restart();
    }

    QString cmd = mCommands.at(0).join(" ");
    verbose(FUNC, feedTxt + cmd.toUtf8(), eInfo);

    cmd.append("\n");
    clone->write(cmd.toUtf8());

    mCommands.removeAt(0);

    if(tooSlow) startClone();
  }
  else
  {
    // Clean up and exit
    verbose(FUNC, feedTxt + "quit", eInfo);
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

void AgentF::cloneHasTrouble(QProcess::ProcessError err) // Slot
{
  QStringList errType = QStringList() << "FailedToStart"
                                      << "Crashed"
                                      << "Timedout"
                                      << "WriteError"
                                      << "ReadError"
                                      << "UnknownError";

  fatal(FUNC, QString("Clone error: %1").arg(errType.at(err)));
  QCoreApplication::exit(1);
}
