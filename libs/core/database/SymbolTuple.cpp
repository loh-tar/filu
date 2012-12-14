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

#include "SymbolTuple.h"

SymbolTuple::SymbolTuple(int size)
           : Tuple(size)
           , mOwnerId(new int[size])
           , mFiId(new int[size])
           , mMarketId(new int[size])
           , mCaption(new QString[size])
           , mMarket(new QString[size])
           , mOwner(new QString[size])
           , mIssueDate(new QDate[size])
           , mMaturityDate(new QDate[size])
{}

SymbolTuple::~SymbolTuple()
{
  delete []mOwnerId;
  delete []mFiId;
  delete []mMarketId;
  delete []mCaption;
  delete []mMarket;
  delete []mOwner;
  delete []mIssueDate;
  delete []mMaturityDate;
}

void SymbolTuple::set(const QString& s, const QString& m, const QString& o
                    , const QDate* idate /*QDate(1000, 1, 1)*/
                    , const QDate* mdate /*QDate(3000, 1, 1)*/)
{
  mCaption[mIndex] = s;
  mMarket[mIndex] = m;
  mOwner[mIndex] = o;
  mIssueDate[mIndex] = (0 == idate ? QDate(1000, 1, 1) : *idate);
  mMaturityDate[mIndex] = (0 == mdate ? QDate(3000, 1, 1) : *mdate);
}

const QDate& SymbolTuple::issueDate() const
{
  return mIssueDate[mIndex];
}

const QDate& SymbolTuple::maturityDate() const
{
  return mMaturityDate[mIndex];
}
