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

#ifndef COTUPLE_HPP
#define COTUPLE_HPP

#include "Tuple.h"

class COTuple : public Tuple
{
  public:
              COTuple(int size);
    virtual  ~COTuple();

    int       fiId();
    int       marketId();
    QString   viewName();
    QDate     anchorDate();
    QString   type();
    QString   attributes();

    void      setId(int);
    void      setFiId(int);
    void      setMarketId(int);
    void      setViewName(const QString&);
    void      setAnchorDate(const QDate&);
    void      setType(const QString&);
    void      setAttributes(const QString&);

    friend class FiluU;

  protected:
    int*      mFiId;
    int*      mMarketId;
    QString*  mViewName;
    QDate*    mAnchorDate;
    QString*  mType;
    QString*  mAttributes;
};

inline int COTuple::fiId()
{
  return mFiId[mIndex];
}

inline int COTuple::marketId()
{
  return mMarketId[mIndex];
}

inline QString COTuple::viewName()
{
  return mViewName[mIndex];
}

inline QDate COTuple::anchorDate()
{
  return mAnchorDate[mIndex];
}

inline QString COTuple::type()
{
  return mType[mIndex];
}

inline QString COTuple::attributes()
{
  return mAttributes[mIndex];
}

inline void COTuple::setId(int id)
{
  mId[mIndex] = id;
}

inline void COTuple::setFiId(int id)
{
  mFiId[mIndex] = id;
}

inline void COTuple::setMarketId(int id)
{
  mMarketId[mIndex] = id;
}

void COTuple::setViewName(const QString& name)
{
  mViewName[mIndex] = name;
}

inline void COTuple::setAnchorDate(const QDate& date)
{
  mAnchorDate[mIndex] = date;
}

inline void COTuple::setType(const QString& type)
{
  mType[mIndex] = type;
}

inline void COTuple::setAttributes(const QString& attr)
{
  mAttributes[mIndex] = attr;
}

#endif
