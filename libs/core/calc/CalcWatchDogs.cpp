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

#include "../../gui/graphics/COType.h"
#include "CalcCross.h"
#include "CalcWatchDogs.h"

CalcWatchDogs::CalcWatchDogs(Indicator* parent)
             : CalcType(parent)
{
  mType = "WatchDog";
}

CalcWatchDogs::~CalcWatchDogs()
{}

bool CalcWatchDogs::prepare(CalcParms& parms)
{
  init(parms);

  if(!isUnknown("WATCHDOG1"))
  {
    addErrorText("CalcWatchDogs::prepare: Only one watchdog call is supported per indicator");
    return false;
  }

  mUsedVariables->insert("WATCHDOG1");

  // mIns looks like: "foo" or "foo, bar,..."
  // mOuts looks like: "burglar"

  checkOutputCount(1);
  checkInputCount(1);

  if(hasError()) return false;

  return true;
}

bool CalcWatchDogs::calc()
{
  static const char* noDogTxt("No watchdog on patrol");

  removeErrorText(noDogTxt);

  if(hasError()) return false;

  getIndicatorVariables();

  // Fetch the ids of the FI where we has to call the watch dogs
  int fiId, marketId;
  bool exist = mData->getIDs("THIS", fiId, marketId);
  if(!exist)
  {
    addErrorText("CalcWatchDogs::calc: FI alias not used: " + mIns.at(1));
    return false;
  }

  // Create our output variable
  addToDataSet(mOuts.at(0));

  int watchDogNB = 1; // Count dogs ((output)lines/variables)

  // Clean up old dog shit
  while(!isUnknown("WATCHDOG" + QString::number(watchDogNB)))
  {
    mUsedVariables->remove("WATCHDOG" + QString::number(watchDogNB));
    mUsedVariables->remove("WATCHDOGRESULT" + QString::number(watchDogNB));
    ++watchDogNB;
  }

  watchDogNB = 0;

  QDate fromDate, toDate;
  mData->setRange();
  mData->getDateRange(fromDate, toDate);

  // Load chart objects
  mFilu->setFiId(fiId);
  mFilu->setMarketId(marketId);
  mFilu->setFromDate();
  mFilu->setToDate();
  COTuple* cot = mFilu->getCOs();

  if(!cot)
  {
    addErrorText(noDogTxt);
    return false;
  }

  bool dogFound = false;

  while(cot->next())
  {
    if(cot->type() != "Line") continue;
    if(!cot->attributes().contains("WatchDog=true")) continue;
    if(!mIns.contains(cot->viewName())) continue;

    QHash<QString, QString> attributes;
    FTool::strToAttributes(cot->attributes(), attributes);

    QString watchRef = attributes.value("WatchRef");

    if(isUnknown(watchRef)) continue;

    dogFound = true;

    // Build the line/watchdog variable
    QString dogName = "WATCHDOG" + QString::number(++watchDogNB);
    addToDataSet(dogName);
    mUsedVariables->insert(dogName);

    int leftIdx  = mData->findDate(cot->anchorDate());
    int rightIdx = mData->findDate(QDate::fromString(attributes.value("RightDate"), Qt::ISODate));

    double leftValue  = attributes.value("AnchorValue").toDouble();
    double deltaValue = attributes.value("RightValue").toDouble() - leftValue;
    double slope      = deltaValue / (rightIdx - leftIdx);

    if(attributes.value("Horizontal") == "true")
    {
      if(leftIdx < 0) leftIdx = 0;
      slope = 0;

      // Calculate the values of the line and add them to mData
      mData->setValidRange(dogName, leftIdx, mData->dataTupleSize() - leftIdx);
      mData->setRange(leftIdx, mData->dataTupleSize());
      int i = leftIdx;
      while(mData->next())
      {
        double value = leftValue + slope * (i++ - leftIdx);
        mData->setValue(dogName, value);
      }
    }
    else
    {
      int i = leftIdx;

      mData->setValidRange(dogName, leftIdx, mData->dataTupleSize() - leftIdx);
      mData->setRange(leftIdx, mData->dataTupleSize());

      if((leftIdx < 0) or (rightIdx < 0))
      {
        // Date badly not found (not in current data set)
        // We have to find out how many bars between leftIdx/rightIdx
        // and between first known bar and rightIdx.
        // Then we can calc the watchdog line. We need more bars!

        BarTuple* bars = mFilu->getBars(fiId, marketId, attributes.value("LeftDate"));

        leftIdx  = bars->findDate(QDate::fromString(attributes.value("LeftDate"), Qt::ISODate));
        rightIdx = bars->findDate(QDate::fromString(attributes.value("RightDate"), Qt::ISODate));

        leftValue  = attributes.value("LeftValue").toDouble();
        deltaValue = attributes.value("RightValue").toDouble() - leftValue;
        slope      = deltaValue / (rightIdx - leftIdx);

        mData->setValidRange(dogName, 0, mData->dataTupleSize());
        mData->setRange(0, mData->dataTupleSize());
        i = bars->findDate(fromDate);
        delete bars;
      }

      while(mData->next())
      {
        double value = leftValue + slope * (i++ - leftIdx);
        mData->setValue(dogName, value);
      }
    }

    QString dogResult = "WATCHDOGRESULT" + QString::number(watchDogNB);

    CalcParms dogParm;
    dogParm.setOuts(QStringList(dogResult));

    QStringList dogIn;
    dogIn << watchRef << dogName << "BOTH";
    dogParm.setIns(dogIn);

    CalcCross dog(mIndicator);
    if(!dog.prepare(dogParm))
    {
      addErrorText(dog.errorText());
      return false;
    }
    dog.calc();

    // "or copy" the interim result into the final
    mData->setRange();
    while(mData->next())
    {
      double value;
      if(mData->getValue(dogResult, value))
      {
        if(value) mData->setValue(mOuts.at(0), value);
      }
    }
  }

  if(!dogFound)
  {
    addErrorText(noDogTxt);
    return false;
  }

  return true;
}

int CalcWatchDogs::barsNeeded(DataTupleSet* data)
{
  mData = data;

  addToDataSet(mOuts.at(0));
  mData->setNeededBars(mOuts.at(0), 2);

  return 2;
}
