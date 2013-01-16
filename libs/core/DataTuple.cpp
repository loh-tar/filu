//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
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

#include <QColor>
#include <QDate>
#include <QString>

#include "DataTuple.h"

#include "DataTupleSet.h"

DataTuple::DataTuple(int size, const QString& name)
{
  Value = new double[size];
  Color = 0;    // Don't waste memory and time if no color is needed
  mNeedsBars = 1;

  Name  = name;
  Label = name;
  MaxIndex = size - 1;

  Index     = new int(-1);
  RangeFrom = new int(-1);
  RangeTo   = new int(MaxIndex);

  mOldRangeFrom = -1;
  mOldRangeTo   = -1;

  setRange();

  firstValid = 0;
  countValid = MaxIndex + 1;

  mMyOwnData = true;
  mIsAutonom = true;

}

DataTuple::DataTuple(const QString& name, double* value, DataTupleSet* dts)
{
  Value = value;
  Color = 0;
  mNeedsBars = 1;

  Name  = name;
  Label = name;

  MaxIndex  =  dts->MaxIndex;
  Index     = &dts->Index;
  RangeFrom = &dts->RangeFrom;
  RangeTo   = &dts->RangeTo;

  mOldRangeFrom = -1;
  mOldRangeTo   = -1;

  //setRange();

  firstValid = 0;
  countValid = MaxIndex + 1;

  mMyOwnData = false;
  mIsAutonom = false;
}

DataTuple::~DataTuple()
{
  if(mMyOwnData) delete []Value;

  if(Color) delete []Color;

  if(mIsAutonom)
  {
    delete Index;
    delete RangeFrom;
    delete RangeTo;
  }
}

bool DataTuple::next()
{
  if(*Index < *RangeTo)
  {
    ++*Index;
    return true;
  }

  return false;
}

void DataTuple::rewind()
{
  *Index = *RangeFrom - 1;
}

int DataTuple::size()
{
  return MaxIndex + 1;
}

bool DataTuple::getValue(double& value)
{
  if( (*Index < firstValid) or (countValid == 0) ) return false;

  value = Value[*Index - firstValid];
  return true;
}

void DataTuple::setValue(const double& value)
{
  if((*Index - firstValid) < 0 or (*Index - firstValid) > MaxIndex) return;
  if(!Value) Value = new double[MaxIndex + 1];

  Value[*Index - firstValid] = value;
}

bool DataTuple::getColor(QColor& color)
{
  if(*Index < firstValid) return false;
  if(!Color) return false;

  color = Color[*Index - firstValid];
  return true;
}

void DataTuple::setColor(const QString& color)
{
  if(*Index < 0 or* Index > MaxIndex) return;
  if(!Color) Color = new QColor[MaxIndex + 1];

  Color[*Index].setNamedColor(color);
}

void DataTuple::setEvenColor(const QString& color)
{
  if(!Color) Color = new QColor[MaxIndex + 1];

  for(int i = 0; i < MaxIndex; i++)
    Color[i].setNamedColor(color);
}

void DataTuple::setRange(int from /* = -1*/, int count /* = -1*/)
{
  int to = count + from;

  // Check if new range setting or not
  if( (from == *RangeFrom) and ( to == *RangeTo) ) return;

  if(from < 0 or from > MaxIndex) *RangeFrom = 0;
  else* RangeFrom = from;

  if(count < 0 or to > MaxIndex) *RangeTo = MaxIndex;
  else* RangeTo = to;

  rewind();

}

void DataTuple::getExtrema(double& min, double& max)
{
  if(mOldRangeFrom == *RangeFrom and mOldRangeTo == *RangeTo)
  {
    min = mMinLow;
    max = mMaxHigh;
    return;
  }

  mOldRangeFrom = *RangeFrom;
  mOldRangeTo   = *RangeTo;

  min =  1000000000;//DBL_MAX;
  max = -1000000000;//DBL_MIN;
  double value;
  rewind();
  while(next())
  {
    if(getValue(value))
    {
      if(value < min) min = value;
      if(value > max) max = value;
    }
  }

  mMinLow  = min;
  mMaxHigh = max;
}

void DataTuple::quitAutonomy(DataTupleSet* dts)
{
  if(mIsAutonom)
  {
    delete Index;
    delete RangeFrom;
    delete RangeTo;
  }

  Index     = &dts->Index;
  RangeFrom = &dts->RangeFrom;
  RangeTo   = &dts->RangeTo;

  mIsAutonom = false;
}
