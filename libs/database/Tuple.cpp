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

#include "Tuple.h"

Tuple::Tuple(int size)
     : Index(-1)
     , MaxIndex(size - 1)
{}

Tuple::~Tuple()
{}

bool Tuple::next()
{
  if(Index < MaxIndex)
  {
    ++Index;
    return true;
  }

  return false;
}

int Tuple::rewind(int start /* = -1*/)
{
  int status = eValid;

  if(start < 0) status = eUnderRange;
  if(start > MaxIndex) status = eOverRange;

  if(status != eValid) Index = -1;
  else Index = start;

  return status;
}

int Tuple::count()
{
  return MaxIndex + 1;
}

bool Tuple::isInvalid()
{
  if(Index <= MaxIndex and Index > -1) return false;

  return true;
}
