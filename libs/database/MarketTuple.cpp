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

#include "MarketTuple.h"

MarketTuple::MarketTuple(int size) : Tuple(size)
{
  Id       = new int[size];
  Name     = new QString[size];
  Currency = new QString[size];
}

MarketTuple::~MarketTuple()
{
  delete []Id;
  delete []Name;
  delete []Currency;
}

int MarketTuple::id()
{
  return Id[Index];
}

QString MarketTuple::name()
{
  return Name[Index];
}

QString MarketTuple::currency()
{
  return Currency[Index];
}
