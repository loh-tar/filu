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

CalcType::CalcType(Indicator* parent, const QString& className)
        : FClass(parent, className)
        , mType("BaseType")
        , mIndicator(parent)
{}

CalcType::~CalcType()
{}

CalcType* CalcType::createNew(Indicator* indi, const QString& type)
{
  if(!type.compare("SETCOLOR"))  return new CalcSetColor(indi);
  if(!type.compare("TALIB"))     return new CalcTALib(indi);
  if(!type.compare("CROSS"))     return new CalcCross(indi);
  if(!type.compare("REF"))       return new CalcRef(indi);
  if(!type.compare("FILU"))      return new CalcFilu(indi);
  if(!type.compare("BETWEEN"))   return new CalcBetween(indi);
  if(!type.compare("SIMTRADE"))  return new CalcTrade(indi);
  if(!type.compare("MUP"))       return new CalcMuParser(indi);
  if(!type.compare("WATCHDOGS")) return new CalcWatchDogs(indi);

  /*FIXME: add new fancy CalcTypes

  if(!type.compare(""))  return new (indi);

  */

  return 0;
}

bool CalcType::prepare(CalcParms &/*parms*/)
{
  error(FUNC, tr("Oops!? Base type never can prepare."));
  return false;
}

bool CalcType::calc()
{
  error(FUNC, tr("Oops?! Base type nerver can calc."));
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
    verbose(FUNC, tr("Variable name '%1' already exist.").arg(key), eMax);
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
  if(mOuts.size() < count)
  {
    if(1 == count)
    {
      error(FUNC, tr("'%1': No output variable found.").arg(mType));
    }
    else
    {
      error(FUNC, tr("'%1': Too less output variables.").arg(mType));
      errInfo(FUNC, tr("Found: %1, Expect: %2").arg(mOuts.size()).arg(count));
    }

    return false; // Error
  }
  else
  {
    // Register all output variable
    for(int i = 0; i < count; ++i) mUsedVariables->insert(mOuts.at(i));
  }

  return true; // Ok
}

bool CalcType::checkInputCount(int count)
{
  if(mIns.size() < count)
  {
    if(1 == count)
    {
      error(FUNC, tr("'%1': No input parameter found.").arg(mType));
    }
    else
    {
      error(FUNC, tr("'%1': Too less input parameters.").arg(mType));
      errInfo(FUNC, tr("Found: %1, Expect: %2").arg(mOuts.size()).arg(count));
    }

    return false; // Error
  }

  return true; // Ok
}

bool CalcType::checkInputVariable(int i)
{
  if(isUnknown(mIns.at(i)))
  {
    QString help = mIns.at(i);
    bool isNumber;
    mIns.at(i).toDouble(&isNumber);
    if(isNumber) mIns.append(QString("Operand-%1-IsNumber").arg(i));
    else
    {
      error(FUNC, tr("'%1': Variable at position %1 not found: %2").arg(i + 1).arg(mIns.at(i)).arg(mType));
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
