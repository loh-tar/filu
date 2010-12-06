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

#ifndef MARKETTUPLE_HPP
#define MARKETTUPLE_HPP

#include "Tuple.h"

class MarketTuple : public Tuple
{
  public:
                MarketTuple(int size);
    virtual    ~MarketTuple();

    QString     name();
    QString     currency();

    friend class Filu;

  protected:
    // Holds the beef
    QString*    mName;
    QString*    mCurrency;
};

inline QString MarketTuple::name()
{
  return mName[mIndex];
}

inline QString MarketTuple::currency()
{
  return mCurrency[mIndex];
}

#endif
