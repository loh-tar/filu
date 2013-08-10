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

#include <QPainter>
#include <QPen>

#include "Grip.h"

#include "BarTuple.h"
#include "IndicatorPainter.h"
#include "Scaler.h"
#include "Tuple.h"

/***********************************************************************
*
*   Grip stuff
*
************************************************************************/

Grip::Grip( IndicatorPainter* painter,Type type
          , const QPointF& pos/* = QPointF(-1, -1)*/
          , const QSizeF& size/* = QSizeF(8,8)*/ )

    : QRectF(pos, size)
    , mType(type)
    , mIdxPos(-1)  // Mark as unvalid
    , mIdxStatus(Tuple::eNotValid)
    , mP(painter)
{
  if(pos != QPointF(-1, -1)) set(pos);
}

Grip::~Grip()
{}

void Grip::update()
{
  // Calculate mIdxPos and pixel postion out of mDate and mValue

  QPoint pos;
  int    idx;

  idx = mIdxPos;

  if(mIdxStatus < Tuple::eValid) // Aka not valid
  {
    // Fetch the index of mDate in mBars or a status value
    idx = mP->mBars->findDate(mDate);

    if(idx > Tuple::eValid)
    {
      // Good!
      mIdxStatus = Tuple::eValid;
      mIdxPos = idx;
    }
    else
    {
      // mDate is not available in mBars so idx holds status no index
      mIdxStatus = idx;

      // Set idx to a valid value
      if(mIdxStatus == Tuple::eUnderRange) idx = 0;  // Set to first available position
      else idx = mP->mBars->count() - 1;             // Set to last available position
    }
  }

  mP->mScaler->valueToPixel(idx - mP->mFirstBarToShow, mValue, pos);

  moveCenter(pos);
}

void Grip::set(const QPointF& pos)
{
  // Set to pixel position and calculate mIdxPos, mDate and mValue

  moveCenter(pos);

  int  newMouseXPos;
  /*int valid = */mP->mScaler->pixelToValue(pos.toPoint(), newMouseXPos, mValue, mDate);
//FIXME: use valid
  mIdxPos  = mP->mFirstBarToShow + newMouseXPos;
//   qDebug() << "Grip::set()" << pos << mIdxPos << mValue << mDate << valid;
}

void Grip::set(const QDate& date, const double value)
{
  mDate      = date;
  mValue     = value;
  mIdxStatus = Tuple::eNotValid;
//   qDebug() << "Grip::set()" << mValue << mDate ;
  update();
}

void Grip::move(const QPointF& delta, const int deltaIdx, const double deltaValue)
{
  moveCenter(center() + delta);

  move(deltaIdx, deltaValue);
}

void Grip::move(const int deltaIdx, const double deltaValue)
{
  mIdxPos += deltaIdx;
  mValue  += deltaValue;

  mIdxStatus = mP->mBars->rewind(mIdxPos);

  if(mIdxStatus == Tuple::eValid) mDate = mP->mBars->date();
//   qDebug() << "Grip::move()" << mIdxStatus << mDate << mIdxPos;
}

void Grip::paint(QPaintDevice* sheet, const QPointF& origin)
{
  QPen pen("yellow");//(mColor);
  pen.setStyle(Qt::SolidLine);

  QPainter painter(sheet);
  painter.setPen(pen);
  painter.setBrush(QBrush("yellow")/*(mColor)*/);
  painter.translate(origin);
  painter.drawRect(*this);
}
