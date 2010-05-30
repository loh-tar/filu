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

#include "COTuple.h"

COTuple::COTuple(int size) : Tuple(size)
{
  mId         = new int[size];
  mFiId       = new int[size];
  mMarketId   = new int[size];
  mViewName   = new QString[size];
  mAnchorDate = new QDate[size];
  mType       = new QString[size];
  mAttributes = new QString[size];
}

COTuple::~COTuple()
{
  delete []mId;
  delete []mFiId;
  delete []mMarketId;
  delete []mViewName;
  delete []mAnchorDate;
  delete []mType;
  delete []mAttributes;
}

int COTuple::id()
{
  return mId[Index];
}

int COTuple::fiId()
{
  return mFiId[Index];
}

int COTuple::marketId()
{
  return mMarketId[Index];
}

QString COTuple::viewName()
{
  return mViewName[Index];
}

QDate COTuple::anchorDate()
{
  return mAnchorDate[Index];
}

QString COTuple::type()
{
  return mType[Index];
}

QString COTuple::attributes()
{
  return mAttributes[Index];
}

void COTuple::setId(int id)
{
  mId[Index] = id;
}

void COTuple::setFiId(int id)
{
  mFiId[Index] = id;
}

void COTuple::setMarketId(int id)
{
  mMarketId[Index] = id;
}

void COTuple::setViewName(const QString& name)
{
  mViewName[Index] = name;
}

void COTuple::setAnchorDate(const QDate& date)
{
  mAnchorDate[Index] = date;
}

void COTuple::setType(const QString& type)
{
  mType[Index] = type;
}

void COTuple::setAttributes(const QString& attr)
{
  mAttributes[Index] = attr;
}
