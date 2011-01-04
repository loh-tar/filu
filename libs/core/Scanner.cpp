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

#include "Scanner.h"
#include "Indicator.h"
#include "DataTupleSet.h"

Scanner::Scanner(FClass* parent)
       : FClass(parent)
       , mBarsToLoad(0)
       , mForce(false)
       , mForcedFrame(0)
{
  reset();
}

Scanner::~Scanner()
{
  foreach(Indicator* indi, mIndicators) delete indi;
}

bool Scanner::exec(const QStringList& command)
{
  // Look for each command, and execute them if was given.
  // The order of look up is important.
  if(command.contains("--reset"))     reset();
  if(command.contains("--verbose"))   setVerboseLevel(command);
  if(command.contains("--force"))     mForce = true;
  if(command.contains("--timeFrame")) setTimeFrame(command);
  if(command.contains("--indi"))      loadIndicator(command);
  if(command.contains("--auto"))      autoSetup();
  if(command.contains("--group"))     scanGroup(command);
  if(command.contains("--this"))      scanThis(command);
  if(command.contains("--mark"))      mark();

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
  mForce = false;
  mForcedFrame = 0;
  Newswire::setVerboseLevel(eNoVerbose);

  clearErrors();
}

void Scanner::loadIndicator(const QStringList& parm)
{
  if(hasError()) return;

  QStringList indiNames;
  FTool::getParameter(parm, "--indi", indiNames);

  bool autoSetup = false;
  if(parm.contains("--auto")) autoSetup = true;

  mRcFile->beginGroup("LastTimeScanned");

  Indicator* indicator = 0;
  foreach(QString indi, indiNames)
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
      addErrorText(indicator->errorText());
      continue;
    }

    if(!indicator->hasScan4())
    {
      addErrorText(QString("Scanner::loadIndicator: No 'SCAN4' variable in indicator %1").arg(indi));
      continue;
    }

    int frame = indicator->scanFreq(); // In fake days
    if(-1 == frame)
    {
      addErrorText(indicator->errorText().join("\n"));
      continue;
    }

    QDate lastScanned = mRcFile->getDT(indi);

    if(autoSetup and !mForce and lastScanned.isValid())
    {
      int days = lastScanned.daysTo(mToday);
      frame    = indicator->scanFreq(true); // In true days

      if(days < frame)
      {
        verbose(FFI_, tr("Today is no need to scan:") + indi, eMax);
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

  if(verboseLevel() == eMax)
  {
    QString txt = tr("Loaded Indi: %1 Frame: %2");
    for(int i = 0; i < mIndicators.size(); ++i)
    {
      verbose(FFI_, txt.arg(mIndicators.at(i)->fileName()).arg(mTimeFrames.at(i)), eMax);
    }

    if(mIndicators.size() > 0) verbose(FFI_, tr("BarsToLoad: %1").arg(mBarsToLoad), eMax);
    else verbose(FFI_, tr("No indicators loaded!"), eMax);
  }
}

void Scanner::setTimeFrame(const QStringList& parm)
{
  QStringList frame;
  if(FTool::getParameter(parm, "--timeFrame", frame) < 1)
  {
    addErrorText("Scanner::setTimeFrame: No frame given.");
    return;
  }

  mForcedFrame = FTool::timeFrame(frame.at(0));

  if(mForcedFrame == -1)
  {
    addErrorText(QString("Scanner::setTimeFrame: Frame unknown: %1").arg(frame.at(0)));
  }
}

void Scanner::setVerboseLevel(const QStringList& parm)
{
  QStringList level;
  if(FTool::getParameter(parm, "--verbose", level) < 1) return; // We ignore that fault

  Newswire::setVerboseLevel(FFI_, level.at(0));
}

void Scanner::autoSetup()
{
  if(hasError()) return;

  QString indicatorPath = mRcFile->getST("IndicatorPath");

  QDir dir(indicatorPath);
  QStringList files = dir.entryList(QDir::Files, QDir::Name);

  QStringList indiList;
  indiList << "--auto" << "--indi";

  for(int i = 0; i < files.size(); ++i)
  {
    if(files.at(i).endsWith("~")) continue;

    QFile file(indicatorPath + files.at(i));
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      addErrorText(QString("Scanner::autoSetup: Can't open indicator file: %1").arg(files.at(i)));
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
        break;
      }
    }
    file.close();
  }

  if(verboseLevel() == eMax)
  {
    for(int i = 2; i < indiList.size(); ++i)
    {
      verbose(FFI_, tr("Select indicator:") + indiList.at(i), eMax);
    }

    if(indiList.size() == 2) verbose(FFI_, tr("None selected!"), eMax);
  }

  loadIndicator(indiList);
}

void Scanner::scanGroup(const QStringList& parm)
{
  if(hasError() or !mBarsToLoad) return;

  // parm looks like "--group foo bar..."
  QStringList arg;
  if(FTool::getParameter(parm, "--group", arg) < 1)
  {
    addErrorText("Scanner::scanGroup: No group given.");
    return;
  }

  foreach(QString group, arg)
  {
    if("all" == group)
    {
      scanAll();
      continue;
    }

    QSqlQuery* query = mFilu->getGMembers(mFilu->getGroupId(group));
    if(!query)
    {
      verbose(FFI_, tr("No FIs in group:") + group, eMax);
      continue;
    }

    if(verboseLevel() == eMax)
    {
      QString txt = tr("%1 FIs to scan in group %2");
      verbose(FFI_, txt.arg(query->size()).arg(group), eMax);
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
    addErrorText("Scanner::scanAll: No symbols found.");
    return;
  }

  if(verboseLevel() == eMax)
  {
    QString txt = tr("%1 FIs to scan.");
    verbose(FFI_, txt.arg(symbols->count()), eMax);
  }

  while(symbols->next())
  {
    //qDebug() << "scan: " << symbols->caption() << symbols->fiId() << hasError();
    scanThis(symbols->fiId(), symbols->marketId());
  }
}

void Scanner::scanThis(const QStringList& parm)
{
  if(hasError() or !mBarsToLoad) return;

  // parm looks like "--this symbol market"
  QStringList arg;
  if(FTool::getParameter(parm, "--this", arg) < 2)
  {
    addErrorText("Scanner::scanThis: Too less arguments.");
    return;
  }

  scan(mFilu->getBars(arg.at(0), arg.at(1), mBarsToLoad));
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

      if(verboseLevel())
      {
        QString info = tr("'%1' found FiId: %2, %3");
        FiTuple* fi = mFilu->getFi(bars->fiId());
        fi->next(); // No need to check if !0

        verbose(FFI_, info.arg(mIndicators.at(i)->fileName()).arg(bars->fiId(), 4).arg(fi->name()), Newswire::eNoVerbose);
        delete fi;
      }

      mFilu->addToGroup(mGroupIDs.at(i), bars->fiId());
    }
    //else if(mIndicators.at(i)->hasError())
    //{
    //  addErrorText(mIndicators.at(i)->errorText().join("\n"));
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
    verbose(FFI_, indi->fileName() + mToday.toString(Qt::ISODate), eMax);

    mRcFile->set(indi->fileName(), mToday);
  }

  mRcFile->endGroup();
}