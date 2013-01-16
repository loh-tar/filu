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

#ifndef SYMBOLTUPLE_HPP
#define SYMBOLTUPLE_HPP

class QDate;
class QString;

#include "Tuple.h"

class SymbolTuple : public Tuple
{
  public:
                SymbolTuple(int size);
    virtual    ~SymbolTuple();

    int             ownerId()       const { return mOwnerId[mIndex]; }
    int             fiId()          const { return mFiId[mIndex]; }
    int             marketId()      const { return mMarketId[mIndex]; }
    const QString&  caption()       const { return mCaption[mIndex]; }
    const QString&  market()        const { return mMarket[mIndex]; }
    const QString&  owner()         const { return mOwner[mIndex]; }
    const QDate&    issueDate()     const;
    const QDate&    maturityDate()  const;

    void        set(const QString& s, const QString& m, const QString& o
                  , const QDate* idate = 0 /*QDate(1000, 1, 1)*/
                  , const QDate* mdate = 0 /*QDate(3000, 1, 1)*/);

    friend class Filu;
    friend class SymbolTableModel;

  protected:
    // Holds the beef
    int*        mOwnerId;
    int*        mFiId;
    int*        mMarketId;
    QString*    mCaption;
    QString*    mMarket;
    QString*    mOwner;
    QDate*      mIssueDate;
    QDate*      mMaturityDate;
};

#endif
