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

#include <QDate>
#include <QString>

#include "COTuple.h"

COTuple::COTuple(int size)
       : Tuple(size)
       , mFiId(new int[size])
       , mMarketId(new int[size])
       , mViewName(new QString[size])
       , mAnchorDate(new QDate[size])
       , mType(new QString[size])
       , mAttributes(new QString[size])
{}

COTuple::~COTuple()
{
  delete []mFiId;
  delete []mMarketId;
  delete []mViewName;
  delete []mAnchorDate;
  delete []mType;
  delete []mAttributes;
}

void COTuple::set(int id, int fiId, int marketId, const QString& name
                , const QDate& date, const QString& type, const QString& attr)
{
  mId[mIndex] = id;
  mFiId[mIndex] = fiId;
  mMarketId[mIndex] = marketId;
  mViewName[mIndex] = name;
  mAnchorDate[mIndex] = date;
  mType[mIndex] = type;
  mAttributes[mIndex] = attr;
}
