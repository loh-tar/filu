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
#include <QSqlQuery>
#include <QTextStream>

#include "Scanner.h"

#include "BarTuple.h"
#include "CmdHelper.h"
#include "DataTupleSet.h"
#include "FTool.h"
#include "FiTuple.h"
#include "FiluU.h"
#include "Indicator.h"
#include "RcFile.h"
#include "SymbolTuple.h"

const QString cCmd1 = "scan";
const QString cCmd1Brief = QObject::tr("Scans the database for events");

Scanner::Scanner(FClass* parent)
       : FClass(parent, FUNC)
       , mCmd(0)
       , mBarsToLoad(0)
       , mAutoSetup(false)
       , mForce(false)
       , mForcedFrame(0)
{
  reset();
}

Scanner::~Scanner()
{
  foreach(Indicator* indi, mIndicators) delete indi;
}

void Scanner::briefIn(CmdHelper* cmd)
{
  if(!cmd) return;

  cmd->inCmdBrief(cCmd1, cCmd1Brief);
}

bool Scanner::exec(CmdHelper* ch)
{
  if(!ch)
  {
    fatal(FUNC, "Called with NULL pointer.");
    return false;
  }

  mCmd = ch;
  mCmd->regStdOpts("indi auto group this force timeFrame mark"/*"reset "*/);
  mCmd->makeOneOfOptsMandatory("FiSet", "indi auto");
  mCmd->makeOneOfOptsMandatory("Scanner", "group this");

  if(mCmd->isMissingParms())
  {
//     mCmd->inOptBrief("reset", "", "Only used by ");
    mCmd->inOptBrief("force", "", tr("Forces '--auto' to scan anyway if needed or not"));

    mCmd->inOptBrief("timeFrame", "<Name>|<Number>"
                    , tr("The time frame to be scanned. Overwrite the ScanFreq Key "
                         "of the indicator when '--auto' is used. But still respect "
                         "if there is a need to be scanned. Use '--force' to ignore that too"));

    mCmd->inOptBrief("indi", "<Name>..", tr("A list of indicators to use for scanning"));

    mCmd->inOptBrief("auto", ""
                    , tr("Uses all indicators with a ScanFreq Key. And take care of its value, to "
                         "avoid unwanted scans. When you also use '--indi' will these indicator also "
                         "used"));

    mCmd->inOptBrief("group", "<Path>.."
                    , tr("A list of groups to scan. When 'all' is given all FIs are scanned and "
                         "any existing <Path> will ignored. Without '--group' is nothing scanned"));

    mCmd->inOptBrief("this", "<Symbol> <Market>", tr("Scan only this FI"));
    mCmd->inOptBrief("mark", "", tr("Save the scan date in the settings file ~/.config/Filu.conf"));

    mCmd->inOptGroup("FiSet", "FI sets", "group this");
    mCmd->inOptGroup("Scanner", "scanners", "auto indi");

    if(mCmd->printThisWay("<FiSet> <Scanner>")) return true;

    mCmd->prin4Comment(tr("The scan run over the entire database, one or more groups or "
                          "only one FI with one or more scanners. These scanners are indicators "
                          "with a SCAN4 variable, see doc/indicator-file-format.txt. "
                          "When the scan has found a FI "
                          "will it added to the PerformerF group 'ScanResults/<Today>/<IndicatorName>'."));

    mCmd->prin4Note(tr("Each time the bars of an FI are be updated a scan will automatic performed "
                       "with the '--auto' switch. So you normally not have to scan directly except "
                       "for testing purposes or multiuser mode."));

    mCmd->printForInst("--group all --indi MyNewIdea --timeFrame Quarter");
    mCmd->printForInst("--group all --auto --force --verbose 2");
    mCmd->aided();
    return true;
  }

  // Look for each command, and execute them if was given.
  // The order of look up is important.
//   if(mCmd->has("reset"))     reset();
  if(mCmd->has("verbose"))   setVerboseLevel(FUNC, mCmd->parmStr(1));
  if(mCmd->has("force"))     mForce = true;
  if(mCmd->has("timeFrame")) setTimeFrame(mCmd->parmStr(1));
  if(mCmd->has("indi"))      loadIndicator(mCmd->parmList());
  if(mCmd->has("auto"))      autoSetup();
  if(mCmd->has("group"))     scanGroup();
  if(mCmd->has("this"))      scanThis();
  if(mCmd->has("mark"))      mark();

  return hasError();
}

void Scanner::reset()
{
  foreach(Indicator* indi, mIndicators) delete indi;
  mIndicators.clear();
  mLoadedIndicators.clear();
  mGroupIDs.clear();
  mTimeFrames.clear();

  mToday = QDate::currentDate();
  mGroupPath = QString("ScanResults/%1/").arg(mToday.toString(Qt::ISODate));
  mBarsToLoad = 0;
  mAutoSetup = false;
  mForce = false;
  mForcedFrame = 0;

  clearErrors();
}

void Scanner::loadIndicator(const QStringList& indiList)
{
  if(hasError()) return;

  mRcFile->beginGroup("LastTimeScanned");

  Indicator* indicator = 0;
  foreach(QString indi, indiList)
  {
    if(mLoadedIndicators.contains(indi)) continue;

    if(!indicator)
    {
      indicator = new Indicator(this);
      indicator->ignorePlot();
    }

    indicator->useFile(indi);

    if(indicator->hasError())
    {
      addErrors(indicator->errors());
      continue;
    }

    if(!indicator->hasScan4())
    {
      error(FUNC, tr("No 'SCAN4' variable in indicator '%1'.").arg(indi));
      continue;
    }

    int frame = indicator->scanFreq(); // In fake days
    if(-1 == frame)
    {
      addErrors(indicator->errors());
      continue;
    }

    QDate lastScanned = mRcFile->getDT(indi);

    if(mAutoSetup and !mForce and lastScanned.isValid())
    {
      int days = lastScanned.daysTo(mToday);
      frame    = indicator->scanFreq(true); // In true days

      if(days < frame)
      {
        verbose(FUNC, tr("Today is no need to scan with '%1', ").arg(indi), eAmple);
        continue;
      }

      if(days > frame)
      {
        frame = days;
      }
      else
      {
        frame = indicator->scanFreq(); // In fake days
      }
    }

    if(mForcedFrame) frame = mForcedFrame;

    mIndicators.append(indicator);
    mLoadedIndicators.append(indi);
    mTimeFrames.append(frame);
    mGroupIDs.append(0); // Fill with placeholder
    mBarsToLoad = qMax(mBarsToLoad, indicator->barsNeeded() + frame);
    indicator = 0;
  }

  if(indicator) delete indicator;

  mRcFile->endGroup();

  if(verboseLevel(eInfo))
  {
    QString txt = tr("Use Indicator: %1 Frame: %2");
    for(int i = 0; i < mIndicators.size(); ++i)
    {
      verbose(FUNC, txt.arg(mIndicators.at(i)->fileName(), -20).arg(mTimeFrames.at(i)));
    }

    if(mIndicators.size() > 0) verbose(FUNC, tr("%1 Bars to load.").arg(mBarsToLoad));
    else verbose(FUNC, tr("No need to scan today."));
  }
}

void Scanner::setTimeFrame(const QString& frame)
{
  mForcedFrame = FTool::timeFrame(frame);

  if(mForcedFrame == -1)
  {
    error(FUNC, tr("Frame '%1' unknown.").arg(frame));
  }
}

void Scanner::autoSetup()
{
  if(hasError()) return;

  QString indicatorPath = mRcFile->getST("IndicatorPath");

  QDir dir(indicatorPath);
  QStringList files = dir.entryList(QDir::Files, QDir::Name);

  QStringList indiList;
  for(int i = 0; i < files.size(); ++i)
  {
    if(files.at(i).endsWith("~")) continue;

    QFile file(indicatorPath + files.at(i));
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      fatal(FUNC, QString("Can't open indicator file '%1'.").arg(files.at(i)));
      continue;
    }

    // Read the indicator
    QTextStream in(&file);
    while (!in.atEnd())
    {
      QString line = in.readLine();
      line.remove(" ");
      if(line.startsWith("*ScanFreq"))
      {
        indiList.append(files.at(i));
        verbose(FUNC, tr("Auto select indicator '%1'.").arg(files.at(i)), eAmple);
        break;
      }
    }
    file.close();
  }

  if(!indiList.size()) warning(FUNC, tr("None indicator auto selected!")/*, eAmple*/);

  mAutoSetup = true;
  loadIndicator(indiList);
}

void Scanner::scanGroup()
{
  if(hasError() or !mBarsToLoad) return;

  QStringList arg = mCmd->parmList();

  if(arg.contains("all"))
  {
    scanAll();
    return;
  }

  foreach(QString group, arg)
  {
    int groupId = mFilu->getGroupId(group);
    if(groupId < 1)
    {
      warning(FUNC, tr("Group not found: %1").arg(group));
      continue;
    }

    QSqlQuery* query = mFilu->getGMembers(groupId);
    if(!query)
    {
      warning(FUNC, tr("No FIs in group: %1").arg(group));
      continue;
    }

    if(verboseLevel(eAmple))
    {
      QString txt = tr("%1 FIs to scan in group %2");
      verbose(FUNC, txt.arg(query->size()).arg(group));
    }

    while(query->next())
    {
      // query: gmember_id, fi_id, symbol, market, market_id
      scanThis(query->value(1).toInt(), query->value(4).toInt());
      if(hasError()) break;
    }
  }
}

void Scanner::scanAll()
{
  SymbolTuple* symbols = mFilu->getAllProviderSymbols();
  if(!symbols)
  {
    error(FUNC, tr("No symbols found."));
    return;
  }

  if(verboseLevel(eInfo))
  {
    QString txt = tr("%1 FIs to scan.");
    verbose(FUNC, txt.arg(symbols->count()));
  }

  while(symbols->next())
  {
    //qDebug() << "scan: " << symbols->caption() << symbols->fiId() << hasError();
    scanThis(symbols->fiId(), symbols->marketId());
  }
}

void Scanner::scanThis()
{
  if(hasError() or !mBarsToLoad) return;

  QString symbol = mCmd->parmStr(1);
  QString market = mCmd->parmStr(2);
  if(mCmd->hasError()) return;

  scan(mFilu->getBars(symbol, market, mBarsToLoad));
}

void Scanner::scanThis(int fiId, int marketId)
{
  if(hasError() or !mBarsToLoad) return;

  scan(mFilu->getBars(fiId, marketId, mBarsToLoad));
}

void Scanner::scan(BarTuple* bars)
{
  if(hasError() or !bars) return;

  for(int i = 0; i < mIndicators.size(); ++i)
  {
    DataTupleSet* data = mIndicators.at(i)->calculate(bars);
    if(!data) continue;

    double bingo = 0.0;
    data->rewind(data->dataTupleSize() - 1 - mTimeFrames.at(i));
    while(data->next())
    {
      data->getValue("SCAN4", bingo);
      if(bingo > 0.0) break;
    }

    if(bingo)
    {
      if(!mGroupIDs.at(i))
      {
        mGroupIDs[i] = mFilu->addGroup(mGroupPath + mIndicators.at(i)->fileName());
      }

      if(verboseLevel(eAmple))
      {
        QString info = tr("%1 found FiId: %2, %3");
        FiTuple* fi = mFilu->getFi(bars->fiId());

        verbose(FUNC, info.arg(mIndicators.at(i)->fileName()).arg(bars->fiId(), 4).arg(fi->name()));
        delete fi;
      }
      else
      {
        //FIXME Do it nicer
        verbose(FUNC, tr("Found FiId: %1").arg(bars->fiId(), 4));
      }

      mFilu->addToGroup(mGroupIDs.at(i), bars->fiId());
    }
    //else if(mIndicators.at(i)->hasError())
    //{
    //  addErrors(mIndicators.at(i)->errors());
    //  removeErrorText("No watchdog on patrol");
    //  //return;
    //}
  }
}

void Scanner::mark()
{
  if(hasError()) return;

  mRcFile->beginGroup("LastTimeScanned");

  foreach(Indicator* indi, mIndicators)
  {
    verbose(FUNC, indi->fileName() + mToday.toString(Qt::ISODate), eMax);

    mRcFile->set(indi->fileName(), mToday);
  }

  mRcFile->endGroup();
}
