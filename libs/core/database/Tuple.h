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

#ifndef TUPLE_HPP
#define TUPLE_HPP

#include <QtCore>  // Not needed here but by the derived classes

/***********************************************************************
*
*   This is the base class of each xxxTuple class.
*   They is not directly used.
*
************************************************************************/

class Tuple
{
  public:

    typedef enum
    {
      eOverRange  = -4,
      eUnderRange = -3,
      eNotValid   = -2,
      eValid      = -1,
      eInRange    =  0
    }IdxStatus;

                Tuple(int size);
    virtual    ~Tuple();

    int         id() const { return mId[mIndex]; };
    int         quality() const { return mQuality[mIndex]; };

    void        setId(int id) { mId[mIndex] = id; };
    void        setQuality(int q) { mQuality[mIndex] = q; };

    bool        next();
    int         rewind(int start = -1);
    int         count() const { return  mMaxIndex + 1; };
    bool        isInvalid() const;

    friend class Filu;

  protected:
    int         mIndex;
    int         mMaxIndex;

    int*        mId;
    int*        mQuality;
};

inline bool Tuple::next()
{
  if(mIndex < mMaxIndex)
  {
    ++mIndex;
    return true;
  }

  return false;
}

inline bool Tuple::isInvalid() const
{
  if(mIndex <= mMaxIndex and mIndex > -1) return false;

  return true;
}

#endif
