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
{}

SymbolTuple::~SymbolTuple()
{
  delete []mOwnerId;
  delete []mFiId;
  delete []mMarketId;
  delete []mCaption;
  delete []mMarket;
  delete []mOwner;
}

void SymbolTuple::set(const QString& s, const QString& m, const QString& o)
{
  mOwnerId[mIndex] = 0;
  mFiId[mIndex] = 0;
  mMarketId[mIndex] = 0;
  mCaption[mIndex] = s;
  mMarket[mIndex] = m;
  mOwner[mIndex] = o;
}
