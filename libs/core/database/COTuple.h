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

#ifndef COTUPLE_HPP
#define COTUPLE_HPP

class QDate;
class QString;

#include "Tuple.h"

class COTuple : public Tuple
{
  public:
              COTuple(int size);
    virtual  ~COTuple();

          int       fiId()        const { return mFiId[mIndex]; }
          int       marketId()    const { return mMarketId[mIndex]; }
    const QString&  viewName()    const { return mViewName[mIndex]; }
    const QDate&    anchorDate()  const { return mAnchorDate[mIndex]; }
    const QString&  type()        const { return mType[mIndex]; }
    const QString&  attributes()  const { return mAttributes[mIndex]; }

    void set(int id, int fiId, int marketId, const QString& name
           , const QDate& date, const QString& type, const QString& attr);

    friend class FiluU;

  protected:
    int*      mFiId;
    int*      mMarketId;
    QString*  mViewName;
    QDate*    mAnchorDate;
    QString*  mType;
    QString*  mAttributes;
};

#endif
