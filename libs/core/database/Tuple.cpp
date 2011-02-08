//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011  loh.tar@googlemail.com
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
     : mIndex(-1)
     , mMaxIndex(size - 1)
{
  mId = new int[size];
  mQuality = new int[size];
}

Tuple::~Tuple()
{
  delete []mId;
  delete []mQuality;
}

int Tuple::rewind(int start /* = -1*/)
{
  int status = eValid;

  if(start < 0) status = eUnderRange;
  if(start > mMaxIndex) status = eOverRange;

  if(status != eValid) mIndex = -1;
  else mIndex = start;

  return status;
}
