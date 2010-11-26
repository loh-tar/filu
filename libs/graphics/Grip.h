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

#ifndef GRIP_HPP
#define GRIP_HPP

#include <QtGui>

class Tuple;
class IndicatorPainter;
class Grip;

/***********************************************************************
*
* Grip is a mouse grip, when editing a chart object.
* Used by COType.cpp and sub classes COxxx.cpp
*
************************************************************************/

class Grip : public QRectF
{
  public:

    typedef enum
    {
      // must each chart object has
      eAnchor,
      // these positions are usual, some of them could a CO has
      eTopLeft,
      eTop,
      eTopRight,
      eRight,
      eBottomRight,
      eBottom,
      eBottomLeft,
      eLeft,
      eCenter,
      // used by COLine and may useful for others
      eFarLeft,
      eFarRight
    }Type;

                Grip( IndicatorPainter* painter, Type type
                    , const QPointF& pos = QPointF(-1, -1)
                    , const QSizeF& size = QSizeF(8,8) );

    virtual    ~Grip();

    QPoint      pos() const { return center().toPoint(); };
    QDate       date() const { return mDate; };
    double      value() const { return mValue; };
    void        update();
    void        set(const QPointF& pos);
    void        set(const QDate& date, const double value);
    void        move(const QPointF& delta, const int deltaIdx, const double deltaValue);
    void        move(const int deltaIdx, const double deltaValue);
    void        paint(QPaintDevice* sheet, const QPointF& origin);

    Type        mType;
    QDate       mDate;
    double      mValue;
    int         mIdxPos;     // the index position of IndicatorPainters mBars
    int         mIdxStatus;

    IndicatorPainter* mP;
};

#endif
