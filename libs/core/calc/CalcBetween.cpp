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

#include "CalcBetween.h"

CalcBetween::CalcBetween(Indicator* parent)
           : CalcType(parent, FUNC)
{
  mType = "Between";
}

CalcBetween::~CalcBetween()
{}

bool CalcBetween::prepare(CalcParms& parms)
{
  init(parms);

  // mIns looks like: "lowBorder, highBorder, foo1, ... fooN"
  // mOuts looks like: "foo"

  checkOutputCount(1);
  if(!checkInputCount(3)) return false;

  int size  = mIns.size();
  mVarCount = size - 2;

  for(int i = 0; i < size; ++i)
  {
    checkInputVariable(i);
  }

  if(hasError()) return false;

  return true;
}

bool CalcBetween::calc()
{
  if(hasError()) return false;

  getIndicatorVariables();

  // Check if all operant variables exist. This should every time happens.
  // But in case of an operand is a constant, we have probably to add
  // a new variable, but not if the same konstant exist already

  for(int i = 0; i < mVarCount + 2; ++i)
  {
    if(addToDataSet(mIns.at(i)))
    {
      if(mIns.contains(QString("Operand-%1-IsNumber").arg(i)))
      {
        double value = mIns.at(i).toDouble();
        mData->rewind();
        while(mData->next())
        {
          mData->setValue(mIns.at(i), value);
        }
      }
      else
      {
        fatal(FUNC, QString("Oops!?!? Unknown variable No.%1 '%2' found, should never heappens.").arg(i).arg(mIns.at(i)));
        return false;
      }
    }
  }

  // Create our output variable
  addToDataSet(mOuts.at(0));

  int outBegIdx = 0;
  int outNbElement = 0;
  int firstValid = 0;
  for(int i = 0; i < mVarCount + 2; ++i)
  {
    mData->getValidRange(mIns.at(i), outBegIdx, outNbElement);
    if(firstValid < outBegIdx) firstValid = outBegIdx;
  }

  mData->setValidRange(mOuts.at(0), firstValid + 1, mData->dataTupleSize() - firstValid);
  mData->setRange(firstValid, mData->dataTupleSize());

  // Prepare for battle...
  double lowBorder, highBorder, value, result;

  //...and rock 'n' roll...
  while(mData->next())
  {
    if(!mData->getValue(mIns.at(0), lowBorder)) {/* error */}
    if(!mData->getValue(mIns.at(1), highBorder)) {/* error */}

    result = 1.0;

    for(int i = 0; i < mVarCount; ++i)
    {
      if(!mData->getValue(mIns.at(i + 2), value)) {/* error */}

      if((value > lowBorder) and (value < highBorder)) continue;

      result = 0.0;
      break;
    }
    mData->setValue(mOuts.at(0), result);
  }

  mData->setRange();
  return true;
}
