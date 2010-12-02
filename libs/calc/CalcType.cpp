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

#include "CalcType.h"
#include "CalcSetColor.h"
#include "CalcTALib.h"
#include "CalcCross.h"
#include "CalcRef.h"
#include "CalcFilu.h"
#include "CalcBetween.h"
#include "CalcTrade.h"
#include "CalcMuParser.h"
#include "CalcWatchDogs.h"

CalcType::CalcType(Indicator* parent)
        : FClass(parent)
        , mType("BaseType")
        , mIndicator(parent)
{}

CalcType::~CalcType()
{}

CalcType* CalcType::createNew(const QString& type)
{
  if(!type.compare("SETCOLOR"))  return new CalcSetColor(mIndicator);
  if(!type.compare("TALIB"))     return new CalcTALib(mIndicator);
  if(!type.compare("CROSS"))     return new CalcCross(mIndicator);
  if(!type.compare("REF"))       return new CalcRef(mIndicator);
  if(!type.compare("FILU"))      return new CalcFilu(mIndicator);
  if(!type.compare("BETWEEN"))   return new CalcBetween(mIndicator);
  if(!type.compare("SIMTRADE"))  return new CalcTrade(mIndicator);
  if(!type.compare("MUP"))       return new CalcMuParser(mIndicator);
  if(!type.compare("WATCHDOGS")) return new CalcWatchDogs(mIndicator);

  /*FIXME: add new fancy CalcTypes

  if(!type.compare(""))  return new (mIndicator);

  */

  if(type.isEmpty()) return new CalcType(mIndicator); // Needed???

  addErrorText("CalcType::createNewType: Type not found: " + type);
  return 0;
}

bool CalcType::prepare(CalcParms &/*parms*/)
{
  addErrorText("CalcType::prepare: Oops!? base type never can prepare");
  return false;
}

bool CalcType::calc()
{
  addErrorText("CalcType::calc: Oops?! base type nerver can calc");
  return false;
}

int CalcType::barsNeeded(DataTupleSet* data)
{
  mData = data;

  for(int i = 0; i < mOuts.size(); ++i) addToDataSet(mOuts.at(i));

  return 1;
}

QString CalcType::getType()
{
  return mType;
}

bool CalcType::addToDataSet(const QString& key)
{
  // Returns true if the variable already exist,
  // than knows the caller that there is no need to do
  // some calculations to create a constant
  if(!mData->append(key)) return false;
  else
  {
    if(mDebugLevel == 2) qDebug() << "CalcType::addToDataSet: Variable name already exist: " << key;
    return true;
  }
}

void CalcType::init(CalcParms& parms)
{
  mOuts = parms.outs();
  mIns  = parms.ins();

  getIndicatorVariables();
}

bool CalcType::isUnknown(const QString& key)
{
  if(mUsedVariables->contains(key)) return false;
  if(mIndicator->mBuildInVariables->contains(key)) return false;

  return true;
}

bool CalcType::checkOutputCount(int count)
{
  bool ok = true;
  QString err = "Calc" + mType + "::prepare: ";

  if(mOuts.size() < count)
  {
    if(1 == count)
    {
      err += "No out variable found.";
    }
    else
    {
      err += "Too less output variables.\n\t"
               "Found: " + QString::number(mOuts.size())
             + ", Expect: " + QString::number(count);
    }

    addErrorText(err);
    ok = false;
  }
  else
  {
    // Register all output variable
    for(int i = 0; i < count; ++i) mUsedVariables->insert(mOuts.at(i));
  }

  return ok;
}

bool CalcType::checkInputCount(int count)
{
  bool ok = true;
  QString err = "Calc" + mType + "::prepare: ";

  if(mIns.size() < count)
  {
    if(1 == count)
    {
      err += "No input parameter found.";
    }
    else
    {
      err += "Too less input parameters.\n\t"
               "Found: " + QString::number(mIns.size())
             + ", Expect: " + QString::number(count);
    }

    addErrorText(err);
    ok = false;
  }

  return ok;
}

bool CalcType::checkInputVariable(int i)
{
  QString err = "Calc" + mType + "::prepare: ";

  if(isUnknown(mIns.at(i)))
  {
    QString help = mIns.at(i);
    bool isNumber;
    mIns.at(i).toDouble(&isNumber);
    if(isNumber) mIns.append(QString("Operand-%1-IsNumber").arg(i));
    else
    {
      addErrorText(err + QString("Variable at position %1 not found: %2").arg(i + 1).arg(mIns.at(i)));
      return false;
    }
  }

  mUsedVariables->insert(mIns.at(i));

  return true;
}

void CalcType::getIndicatorVariables()
{
  mData             = mIndicator->mData;
  mUsedVariables    = mIndicator->mUsedVariables;
  //mViewName         = mIndicator->mViewName;
}
