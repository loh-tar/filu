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

#include <QDate>
#include <QString>

#include "FiTuple.h"
#include "SymbolTuple.h"

FiTuple::FiTuple(int size)
       : Tuple(size)
       , mTypeId(new int[size])
       , mType(new QString[size])
       , mName(new QString[size])
       , mExpiryDate(new QDate[size])
       , mSymbol(new SymbolTuple*[size])

{}

FiTuple::~FiTuple()
{
  delete []mTypeId;
  delete []mType;
  delete []mName;
  delete []mExpiryDate;
  // Clean up Symbols
  for (int i = 0; i < mMaxIndex ; i++) delete []mSymbol[i] ;
  delete []mSymbol;
}

const QDate& FiTuple::expiryDate() const
{
  return mExpiryDate[mIndex];
}

void FiTuple::set(const QString& name, const QString& type, SymbolTuple* symbol)
{
  mName[mIndex] = name;
  mType[mIndex] = type;
  mSymbol[mIndex] = symbol;
}
