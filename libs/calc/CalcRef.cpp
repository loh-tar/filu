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

#include "CalcRef.h"

CalcRef::CalcRef(Indicator* parent)
       : CalcType(parent)
{
  mType = "Ref";
}

CalcRef::~CalcRef()
{}

bool CalcRef::prepare(CalcParms& parms)
{
  init(parms);

  // mIns looks like: "foo, 50"
  // mOuts looks like: "foo2"

  checkOutputCount(1);
  if(!checkInputCount(2)) return false;

  checkInputVariable(0);

  bool ok;
  mBackRef = mIns.at(1).toInt(&ok);
  if(!ok)
  {
    addErrorText("CalcRef::prepare: BackRef value is not an integer");
  }

  if(hasError()) return false;

  return true;
}

bool CalcRef::calc()
{
  if(hasError()) return false;

  getIndicatorVariables();

  // Create our output variable
  addToDataSet(mOuts.at(0));

  int outBegIdx = 0;
  int outNbElement = 0;
  mData->getValidRange(mIns.at(0), outBegIdx, outNbElement);

  double* in  = mData->valueArray(mIns.at(0));
  double* out = mData->valueArray(mOuts.at(0));

  for(int i = 0; i < outNbElement; ++i) out[i] = in[i];

  mData->setValidRange(mOuts.at(0), outBegIdx + mBackRef, outNbElement - mBackRef);
  return true;
}

int CalcRef::barsNeeded(DataTupleSet* data)
{
  mData = data;

  int needed = mData->neededBars(mIns.at(0)) + mBackRef;

  addToDataSet(mOuts.at(0));
  mData->setNeededBars(mOuts.at(0), needed);

  return needed;
}
