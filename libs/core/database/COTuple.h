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

#include "Tuple.h"

class COTuple : public Tuple
{
  public:
              COTuple(int size);
    virtual  ~COTuple();

          int       fiId() const;
          int       marketId() const;
    const QString&  viewName() const;
    const QDate&    anchorDate() const;
    const QString&  type() const;
    const QString&  attributes() const;

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

inline int COTuple::fiId() const
{
  return mFiId[mIndex];
}

inline int COTuple::marketId() const
{
  return mMarketId[mIndex];
}

inline const QString& COTuple::viewName() const
{
  return mViewName[mIndex];
}

inline const QDate& COTuple::anchorDate() const
{
  return mAnchorDate[mIndex];
}

inline const QString& COTuple::type() const
{
  return mType[mIndex];
}

inline const QString& COTuple::attributes() const
{
  return mAttributes[mIndex];
}

inline void COTuple::setFiId(int id)
{
  mFiId[mIndex] = id;
}

inline void COTuple::setMarketId(int id)
{
  mMarketId[mIndex] = id;
}

inline void COTuple::setViewName(const QString& name)
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
