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

#ifndef BARTUPLE_HPP
#define BARTUPLE_HPP

class QDate;
class QTime;

#include "Tuple.h"

class BarTuple : public Tuple
{
  public:
                  BarTuple(int Size);
    virtual      ~BarTuple();

    int           findDate(const QDate& date) const;

    const QDate&  date() const;
    const QTime&  time() const;
    double        open() const;
    double        high() const;
    double        low() const;
    double        close() const;
    double        volume() const;
    int           fiId() const;
    int           marketId() const;

    friend class Filu;
    friend class DataTupleSet;

  protected:
    // Holds the beef
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

inline double BarTuple::open() const
{
  return mOpen[mIndex];
}

inline double BarTuple::high() const
{
  return mHigh[mIndex];
}

inline double BarTuple::low() const
{
  return mLow[mIndex];
}

inline double BarTuple::close() const
{
  return mClose[mIndex];
}

inline double BarTuple::volume() const
{
  return mVolume[mIndex];
}

inline int BarTuple::fiId() const
{
  return mFiId;
}

inline int BarTuple::marketId() const
{
  return mMarketId;
}

#endif
