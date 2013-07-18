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

#ifndef FITUPLE_HPP
#define FITUPLE_HPP

class QString;
class QDate;

#include "Tuple.h"

class SymbolTuple;

class FiTuple : public Tuple
{
  public:
                  FiTuple(int size);
    virtual      ~FiTuple();

    int             typeId()  const { return mTypeId[mIndex]; }
    const QString&  type()    const { return mType[mIndex]; }
    const QString&  name()    const { return mName[mIndex]; }
    const QDate&    expiryDate() const;
    SymbolTuple*    symbol()  const { return mSymbol[mIndex]; }

    void          set(const QString& name, const QString& type, SymbolTuple* symbol);

    friend class  Filu;
    friend class  FiTableModel;

  protected:
    // Holds the beef
    int*          mTypeId;
    QString*      mType;
    QString*      mName;
    QDate*        mExpiryDate;
    SymbolTuple** mSymbol;
};

#endif
