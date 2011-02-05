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

#ifndef SYMBOLTUPLE_HPP
#define SYMBOLTUPLE_HPP

#include "Tuple.h"

class SymbolTuple : public Tuple
{
  public:
                SymbolTuple(int size);
    virtual    ~SymbolTuple();

    //int         typeId() const;
    int         fiId() const;
    int         marketId() const;
    const QString&  caption() const;
    //const QString&  issueDate() const;
    //const QString&  maturityDate() const;
    const QString&  market() const;
    const QString&  owner() const;

    void        setFiId(const int id);
    void        setMarketId(const int id);
    void        setCaption(const QString&);
    void        setMarket(const QString&);
    void        setOwner(const QString&);

    friend class Filu;
    friend class SymbolTableModel;

  protected:
    // Holds the beef
    //int*        mStypeId;
    int*        mFiId;
    int*        mMarketId;
    QString*    mCaption;
    //QString*    mIssuedate;
    //QString*    mMaturitydate;
    QString*    mMarket;
    QString*    mOwner;
};

inline int SymbolTuple::fiId() const
{
  return mFiId[mIndex];
}

inline int SymbolTuple::marketId() const
{
  return mMarketId[mIndex];
}

inline const QString& SymbolTuple::caption() const
{
  return mCaption[mIndex];
}

inline const QString& SymbolTuple::market() const
{
  return mMarket[mIndex];
}

inline const QString& SymbolTuple::owner() const
{
  return mOwner[mIndex];
}

inline void SymbolTuple::setFiId(const int id)
{
  mFiId[mIndex] = id;
}

inline void SymbolTuple::setMarketId(const int id)
{
  mMarketId[mIndex] =  id;
}

inline void SymbolTuple::setCaption(const QString& caption)
{
  mCaption[mIndex] = caption;
}

inline void SymbolTuple::setMarket(const QString& market)
{
  mMarket[mIndex] = market;
}

inline void SymbolTuple::setOwner(const QString& owner)
{
  mOwner[mIndex] = owner;
}

#endif
