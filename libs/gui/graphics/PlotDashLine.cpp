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

#include "PlotDashLine.h"

PlotDashLine::PlotDashLine(Newswire* parent) : PlotLine(parent)
{
  mType = "DashLine";

  QVector<qreal> dashes;
  qreal space = 6;
  dashes << 3 << space;
  mPen.setDashPattern(dashes);
}

PlotDashLine::~PlotDashLine()
{}

bool PlotDashLine::paint(QPaintDevice* sheet, QRect& chartArea,
                     DataTupleSet* data, Scaler* scaler)
{
  if(!data)
  {
    error(FFI_, tr("No data."));
    return false;
  }

  QPainter painter(sheet);
  painter.translate(chartArea.bottomLeft());

  QPoint oldPoint(0,0);

  QPoint newPoint;
  data->rewind();

  double value;
  int i = 0;
  // Find first valid value
  while(data->next())
  {
    if(data->getValue(mCommand.at(1), value)) break;
    ++i;
  }
  scaler->valueToPixel(i, value, oldPoint);

  QColor color;
  while(data->next())
  {
    ++i;

    if(!data->getValue(mCommand.at(1), value)) continue;
    if(!data->getColor(mCommand.at(1), color))
      color.setNamedColor("white");

    scaler->valueToPixel(i, value, newPoint);

    mPen.setColor(color);
    painter.setPen(mPen);
    painter.drawLine(oldPoint, newPoint);
    oldPoint = newPoint;
  }

  return true;
}
