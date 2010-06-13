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

#include "CalcCross.h"

CalcCross::CalcCross(Indicator* parent)
         : CalcType(parent)

{
  mType = "Cross";
}

CalcCross::~CalcCross()
{}

bool CalcCross::prepare(CalcParms& parms)
{
  init(parms);

  // mIns looks like: "foo1, foo2, UP"
  // mOuts looks like: "foo3"

  checkOutputCount(1);
  if(!checkInputCount(2)) return false;

  if(mIns.size() > 2)
  {
    if(     mIns.at(2) != "UP"
        and mIns.at(2) != "DOWN"
        and mIns.at(2) != "BOTH")
    {
      addErrorText("CalcCross::prepare: Unknown operator: " + mIns.at(2));
    }
  }
  else
  {
    mIns << "BOTH";
  }

  checkInputVariable(0);
  checkInputVariable(1);

  if(hasError()) return false;

  return true;
}

bool CalcCross::calc()
{
  if(hasError()) return false;

  getIndicatorVariables();

  QString operant1 = mIns.at(0);
  QString operant2 = mIns.at(1);
  QString opertr   = mIns.at(2);

  // check if both operant variables exist. This should every time happens.
  // But in case of an operant is a constant, we have probably to add
  // a new variable, but not if the same konstant exist already

  // 1st operant...
  if(addToDataSet(operant1))
  {
    if(mIns.contains("Operant-0-IsNumber"))
    {
      double value = operant1.toDouble();
      mData->rewind();
      while(mData->next())
      {
        mData->setValue(operant1, value);
      }
    }
    else
    {
      qDebug() << "CalcCross::calc: oops!?!? unknown variable-1 found, should never heappens" << operant1;
      return false;
    }
  }

  // ..and the 2nd operant
  if(addToDataSet(operant2))
  {
    if(mIns.contains("Operant-1-IsNumber"))
    {
      double value = operant2.toDouble();
      mData->rewind();
      while(mData->next())
      {
        mData->setValue(operant2, value);
      }
    }
    else
    {
      qDebug() << "CalcCross::calc: oops!?!? unknown variable-2 not found, should never heappens" << operant2;
      return false;
    }
  }

  // create our output variable
  addToDataSet(mOuts.at(0));

  int outBegIdx = 0;
  int outNbElement = 0;
  int firstValid = 0;

  mData->getValidRange(operant1, outBegIdx, outNbElement);
  firstValid = outBegIdx;
  mData->getValidRange(operant2, outBegIdx, outNbElement);
  if(firstValid < outBegIdx) firstValid = outBegIdx;

  mData->setValidRange(mOuts.at(0), firstValid + 1, mData->dataTupleSize() - firstValid);

  mData->setRange(firstValid, mData->dataTupleSize());

  // prepare for battle...
  double op1, op2, result=0.0;

  //...and rock 'n' roll...
  if(opertr == "UP")
  {
    int wasUnder = 2; // 2=undefine, 1=yes, 0=no
    while(mData->next())
    {
      if(!mData->getValue(operant1, op1)) {/* error */}
      if(!mData->getValue(operant2, op2)) {/* error */}

      if(wasUnder == 1)
      {
        if(op1 > op2)
        {
          result = 1.0;
          wasUnder = 0;
        }
      }
      else
      {
        result = 0.0;
        if(op1 < op2) wasUnder = 1;
      }
      mData->setValue(mOuts.at(0), result);
    }
  }
  else if(opertr == "DOWN")
  {
    int wasUpper = 2; // 2=undefine, 1=yes, 0=no
    while(mData->next())
    {
      if(!mData->getValue(operant1, op1)) {/* error */}
      if(!mData->getValue(operant2, op2)) {/* error */}

      if(wasUpper == 1)
      {
        if(op1 < op2)
        {
          result = 1.0;
          wasUpper = 0;
        }
      }
      else
      {
        result = 0.0;
        if(op1 > op2) wasUpper = 1;
      }
      mData->setValue(mOuts.at(0), result);
    }
  }
  else if(opertr == "BOTH")
  {
    int wasUpper = 2; // 2=undefine, 1=yes, 0=no
    int wasUnder = 2;
    while(mData->next())
    {
      if(!mData->getValue(operant1, op1)) {/* error */}
      if(!mData->getValue(operant2, op2)) {/* error */}

      result = 0.0;

      if(wasUnder == 1)
      {
        if(op1 > op2)
        {
          result = 1.0;
          wasUnder = 0;
        }
      }
      else
      {
        if(op1 < op2) wasUnder = 1;
      }

      if(wasUpper == 1)
      {
        if(op1 < op2)
        {
          result = 1.0;
          wasUpper = 0;
        }
      }
      else
      {
        if(op1 > op2) wasUpper = 1;
      }

      mData->setValue(mOuts.at(0), result);
    }
  }
  else return false;

  mData->setRange();
  return true;
}

int CalcCross::barsNeeded(DataTupleSet* data)
{
  mData = data;

  int needed = mData->neededBars(mIns.at(0));

  if(needed < mData->neededBars(mIns.at(1))) needed = mData->neededBars(mIns.at(1));

  ++needed;

  addToDataSet(mOuts.at(0));
  mData->setNeededBars(mOuts.at(0), needed);

  return needed;
}
