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

#include <float.h>

#include "CalcTrade.h"

CalcTrade::CalcTrade(Indicator* parent)
         : CalcType(parent, FUNC)
{
  mType   = "Trade";
  mTrader = new Trader(this);
}

CalcTrade::~CalcTrade()
{
  delete mTrader;
}

bool CalcTrade::prepare(CalcParms& parms)
{
  init(parms);

  // mOuts looks like: ("")
  // mIns looks like: ("rule")

  if(!checkInputCount(1)) return false;

  if(!mTrader->useRuleFile(mIns.at(0)))
  {
    addErrors(mTrader->errors());
    return false;
  }

  mTrader->getVariablesList(mUsedVariables);

  mOuts << "OPEN" << "HIGH" << "LOW" << "CLOSE";

  return true;
}

bool CalcTrade::calc()
{
  getIndicatorVariables();

  // Create our output variable
  // ... not needed

  mData->setRange();

  if(!mTrader->simulate(mData))
  {
    addErrors(mTrader->errors());
    return false;
  }

  // Print report to console
  if(verboseLevel(eAmple))
  {
    QList<QStringList> report;
    mTrader->getReport(report);
    for(int i = 0; i < report.size(); ++i) verbose(FUNC, report.at(i).join("; "));
  }

  mData->rewind();
  while(mData->next())
  {
    //checkLog();
    double v1, v2;

    // In the following we only validate the the first one.
    // When this is valid, all other values are for shure valid too
    if(!mData->getValue("Long", v1))
    {
      // No valid data, paint all in grey and continue
      mData->setColor(mOuts.at(0), "grey");
      mData->setColor(mOuts.at(1), "grey");
      mData->setColor(mOuts.at(2), "grey");
      mData->setColor(mOuts.at(3), "grey");
      continue;
    }

    // Ok, paint the candles in a suitable color
    //
    // We check the status one after the other an repaint
    // some parts if necessary

    mData->getValue("OffMarket", v2);

    if(v1)
    {
      mData->setColor(mOuts.at(0), "green");
      mData->setColor(mOuts.at(1), "green");
      mData->setColor(mOuts.at(2), "green");
      mData->setColor(mOuts.at(3), "green");
    }
    else if(v2)
    {
      mData->setColor(mOuts.at(0), "grey");
      mData->setColor(mOuts.at(1), "grey");
      mData->setColor(mOuts.at(2), "grey");
      mData->setColor(mOuts.at(3), "grey");
      //continue;
    }

    mData->getValue("OOLongBuy", v1);
    if(v1)
    {
      mData->setColor(mOuts.at(0), "green");
      mData->setColor(mOuts.at(1), "yellow");
      mData->setColor(mOuts.at(2), "yellow");
      mData->setColor(mOuts.at(3), "green");
      continue;
    }

    mData->getValue("OOLongSell", v1);
    if(v1)
    {
      mData->setColor(mOuts.at(0), "red");
      mData->setColor(mOuts.at(1), "yellow");
      mData->setColor(mOuts.at(2), "yellow");
      mData->setColor(mOuts.at(3), "red");
      continue;
    }

/*    if(mTradeStatus == Short)
    {
      mData->setColor(mOuts.at(0), "red");
      mData->setColor(mOuts.at(1), "red");
      mData->setColor(mOuts.at(2), "red");
    }*/

  }

// ???  mData->setRange();

  return true;
}
