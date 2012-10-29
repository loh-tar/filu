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

#ifndef SYMBOLTYPETUPLE_HPP
#define SYMBOLTYPETUPLE_HPP

#include "Tuple.h"

class SymbolTypeTuple : public Tuple
{
  public:
                SymbolTypeTuple(int size) : Tuple(size)
                                          , mCaption(new QString[size])
                                          , mSeq(new int[size])
                                          , mIsProvider(new bool[size]) {};

    virtual    ~SymbolTypeTuple() { delete []mCaption;
                                    delete []mSeq;
                                    delete []mIsProvider; };

    const QString&  caption() const;
    int             seq() const;
    int             isProvider() const;

    void        setCaption(const QString&);
    void        setSeq(const int id);
    void        setIsProvider(const bool id);

    friend class Filu;

  protected:
    // Holds the beef
    QString*    mCaption;
    int*        mSeq;
    bool*       mIsProvider;
};

inline const QString& SymbolTypeTuple::caption() const
{
  return mCaption[mIndex];
}

inline int SymbolTypeTuple::seq() const
{
  return mSeq[mIndex];
}

inline int SymbolTypeTuple::isProvider() const
{
  return mIsProvider[mIndex];
}

inline void SymbolTypeTuple::setCaption(const QString& caption)
{
  mCaption[mIndex] = caption;
}

inline void SymbolTypeTuple::setSeq(const int seq)
{
  mSeq[mIndex] = seq;
}

inline void SymbolTypeTuple::setIsProvider(const bool yes)
{
  mIsProvider[mIndex] = yes;
}

#endif
