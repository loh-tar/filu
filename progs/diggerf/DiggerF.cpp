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

#include "DiggerF.h"

DiggerF::DiggerF(QObject* parent)
       : QObject (parent), FClass("DiggerF")
{
  mIndicator = 0;

  QSqlQuery* groups = mFilu->getGroups(-1); // get all groups
  mScannerGroupId = 0;
  while(groups->next())
  {
    if(groups->value(1).toString() == "ScanResults")
    {
      mScannerGroupId = groups->value(0).toInt();
      break;
    }
  }
  if(!mScannerGroupId)
  {
    mFilu->putGroup(0, "ScanResults", 0);
    groups = mFilu->getGroups(mScannerGroupId);
    while(groups->next())
    {
      if(groups->value(1).toString() == "ScanResults")
      {
        mScannerGroupId = groups->value(0).toInt();
        break;
      }
    }
  }
}

DiggerF::~DiggerF()
{

}

void DiggerF::printUsage()
{
  qDebug() << "Hello! I'm part of Filu. Please call me this way:";
  qDebug() << "diggerf <command> <parameter list> <command> <parameter list>";
  qDebug() << "\t-ind <indicator name>\t\tThe indicator to use for scanning";
  qDebug() << "\t-this <symbol> <market>\t\tScan only this FI";
  qDebug() << "\t-group <group name>\t\tScan this group. Group 'all' scans all FIs";
  qDebug() << "\ne.g.\tdiggerf -group all -ind 50x200\n";
}

int DiggerF::execCmd(QStringList& parm)
{
  // look for each known command and call the related function
  if(parm.contains("-ind"))
    if(!loadIndicator(parm)) return 1;

  if(parm.contains("-this"))  return singleScan(parm);
  else if(parm.contains("-group")) return scanGroup(parm);

  return 0;
}

Indicator* DiggerF::loadIndicator(QStringList& parm)
{
  int filePos = parm.indexOf("-ind") + 1;
  if(!filePos or (filePos > parm.size() - 1)) return 0;

  if(!mIndicator)
  {
    mIndicator = new Indicator(this);
    //mIndicator->ignorePlot(true);
  }

  QStringList* indiFile = mIndicator->useFile(parm.at(filePos));

  if(!indiFile)
  {
    addErrorText(mIndicator->errorText());
    delete mIndicator;
    mIndicator = 0;
    return 0;
  }

  QSet<QString> names;
  mIndicator->getVariableNames(&names);
  if(!names.contains("SCAN4"))
  {
    qDebug() << "DiggerF::loadIndicator: no 'SCAN4' variable in indicator" << parm.at(filePos);
    delete mIndicator;
    mIndicator = 0;
    return 0;
  }

  mBarsToLoad = mIndicator->barsNeeded();
qDebug() << "mBarsToLoad" << mBarsToLoad;
  QSqlQuery* groups = mFilu->getGroups(mScannerGroupId);
  mGroupId = 0;
  if(!groups)
  {
    mFilu->putGroup(0, parm.at(filePos), mScannerGroupId);
    groups = mFilu->getGroups(mScannerGroupId);
    while(groups->next())
    {
      if(groups->value(1).toString() == parm.at(filePos))
      {
        mGroupId = groups->value(0).toInt();
        break;
      }
    }
  }
  else
  {
    while(groups->next())
    {
      if(groups->value(1).toString() == parm.at(filePos))
      {
        mGroupId = groups->value(0).toInt();
        break;
      }
    }
    if(!mGroupId)
    {
      mFilu->putGroup(0, parm.at(filePos), mScannerGroupId);
      groups = mFilu->getGroups(mScannerGroupId);
      while(groups->next())
      {
        if(groups->value(1).toString() == parm.at(filePos))
        {
          mGroupId = groups->value(0).toInt();
          break;
        }
      }
    }
  }
  return mIndicator;
}

int DiggerF::singleScan(QStringList& parm)
{
  int idx = parm.indexOf("-this");
  if((idx + 2) > (parm.size() - 1))
  {
    qDebug() << "DiggerF::singleScan: to less -this parameter";
    return 1;
  }

  QString symbol = parm.at(idx + 1);
  QString market = parm.at(idx + 2);
  if(symbol.startsWith("-") or market.startsWith("-"))
  {
    qDebug() << "DiggerF::singleScan: to less -this parameter";
    return 1;
  }

  int fiId = symbol.toInt();
  int marketId = market.toInt();
  if(!fiId or !marketId)
  {
    SymbolTuple* st = mFilu->searchSymbol(symbol, market);
    if(!st)
    {
      qDebug() << "DiggerF::singleScan: no fittings for" << symbol << market;
      return 1;
    }
    st->next();
    scanThis(st->fiId(), st->marketId());
    delete st;
  }
  else scanThis(fiId, marketId);

  return 0;
}

int DiggerF::scanGroup(QStringList& parm)
{
  int idx = parm.indexOf("-group") + 1;
  if(idx > (parm.size() - 1) or parm.at(idx).startsWith("-"))
  {
    qDebug() << "DiggerF::scanGroup: to less -group parameter";
    return 1;
  }

  if(parm.at(idx) == "all")
  {
    // get all available provider symbols
    mFilu->setMarketName("");
    mFilu->setSymbolCaption("");
    mFilu->setProviderName("");
    mFilu->setFiId(0);
    mFilu->setFiType("");
    mFilu->setOnlyProviderSymbols(true);
    SymbolTuple* symbols = mFilu->getSymbols();
    if(!symbols)
    {
      qDebug() << "DiggerF::scanGroup: no symbols found";
      return 1;
    }

    while(symbols->next())
    {
      scanThis(symbols->fiId(), symbols->marketId());
    }

  }
  else
  {
    QSqlQuery* groups = mFilu->getGroups(-1);
    if(!groups) return 1;
    int groupToScan = 0;
    while(groups->next())
    {
      if(groups->value(1).toString() == parm.at(idx))
      {
        groupToScan = groups->value(0).toInt();
        break;
      }
    }

    if(!groupToScan)
    {
      qDebug() << "DiggerF::scanGroup: group not found" << parm.at(idx);
      return 1;
    }

    QSqlQuery* members = mFilu->getGMembers(groupToScan);
    while(members->next())
    {
      scanThis(members->value(1).toInt(), members->value(4).toInt());
    }
  }
  return 0;
}

int DiggerF::scanThis(int fiId, int marketId)
{
  if(!mIndicator)
  {
    qDebug() << "DiggerF::scanThis: no indicator loaded";
    return 1;
  }

  mFilu->setFiId(fiId);
  mFilu->setMarketId(marketId);

  mFilu->setBarsToLoad(mBarsToLoad+500);//FIXME

  BarTuple* bars = mFilu->getBars();
  if(!bars) return 1;

  DataTupleSet* data = mIndicator->calculate(bars);

  if(!data)
  {
     addErrorText(mIndicator->errorText());
     delete bars;
     return 1;
  }

  data->setRange(bars->count() - 1, 2);
  //data->rewind();
  data->next();

  double yes;
  if(!data->getValue("SCAN4", yes))
  {
    qDebug() << "DiggerF::scanThis: ???" ;
    delete bars;
    return 1;
  }

  if(yes > 0.0)
  {
    qDebug() << "bingoyes=" << yes << fiId;
    mFilu->addToGroup(mGroupId, fiId);
  }

  delete bars;

  return 0;
}
