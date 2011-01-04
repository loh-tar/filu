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

#include "BarTuple.h"

BarTuple::BarTuple(int size) : Tuple(size)
{
  mDate = new QDate[size];
  mTime = new QTime[size];
  mOpen = new double[size];
  mHigh = new double[size];
  mLow  = new double[size];
  mClose  = new double[size];
  mVolume = new double[size];
  mOpenInterest = new double[size];
}

BarTuple::~BarTuple()
{
  delete []mDate;
  delete []mTime;
  delete []mOpen;
  delete []mHigh;
  delete []mLow;
  delete []mClose;
  delete []mVolume;
  delete []mOpenInterest;
}

int BarTuple::findDate(const QDate& date) const
{
  // Find the date inside of mDate[].
  // We don't iterate simple 0 -> cont() which is to slow
  // but we use a ..binary search (that's the correct name?)

  int iLow  = 0;
  int iHigh = count() -1;

  // Before we do anything check if date is in available range
  if(date < mDate[iLow])  return Tuple::eUnderRange;
  if(date > mDate[iHigh]) return Tuple::eOverRange;

  int i     = iHigh / 2;
  while(iLow < iHigh)
  {
    //qDebug() << "BarTuple::rewind() search:" << date <<  ++j << iLow << iHigh << mDate[i] << i;
    if(date > mDate[i])
    {
      iLow = i + 1;
      i    = i + ((iHigh - i) / 2);
    }
    else if(date < mDate[i])
    {
      iHigh = i - 1;
      i     = i - ((iLow + i) / 2);
    }
    else if(date == mDate[i])
    {
      //qDebug() << "BarTuple::rewind() found:" << date <<  ++j  <<  mDate[i] << i;
      break;
    }
  }
  //qDebug() << "BarTuple::rewind() last low/high/i" << iLow << iHigh << i;

  if(iLow == iHigh) i = iLow; // No other possibility
  return i;
}