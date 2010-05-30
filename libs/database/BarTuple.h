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

#ifndef BARTUPLE_HPP
#define BARTUPLE_HPP

#include "Tuple.h"

class BarTuple : public Tuple
{
  public:
                BarTuple(int Size);
    virtual    ~BarTuple();

    int         findDate(const QDate& date);

    QDate       date();
    QTime       time();
    double      open();
    double      high();
    double      low();
    double      close();
    double      volume();

    // holds the beef
    QDate*      mDate;
    QTime*      mTime;
    double*     mOpen;
    double*     mHigh;
    double*     mLow;
    double*     mClose;
    double*     mVolume;
    double*     mOpenInterest;

    int         mFiId;
    int         mMarketId;
};

#endif
