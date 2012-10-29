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

#include "PlotHistogram.h"

PlotHistogram::PlotHistogram(Newswire* parent) : PlotType(parent, FUNC)
{
  mType = "Histogram";
}

PlotHistogram::~PlotHistogram()
{}

bool PlotHistogram::prepare(QStringList& command, QStringList& plotDataKeys)
{
  if(command.size() < 2)
  {
    error(FUNC, tr("To less parameter."));
    return false;
  }

  mCommand = command;
  plotDataKeys.append(mCommand.at(1));
  return true;
}

bool PlotHistogram::paint(QPaintDevice* sheet, QRect& chartArea,
                     DataTupleSet* data, Scaler* scaler)
{
  if(!data)
  {
    error(FUNC, tr("No data."));
    return false;
  }

  QPainter painter(sheet);
  painter.translate(chartArea.bottomLeft());

  double value, maxHigh, minLow;
  QPoint oldPoint, newPoint;
  QColor color;

  // Check if the zero-line is visible
  scaler->getHighLow(maxHigh, minLow);
  scaler->valueToPixel(1, 0.0, newPoint);
  if(minLow  > 0) scaler->valueToPixel(1, minLow, newPoint);
  if(maxHigh < 0) scaler->valueToPixel(1, maxHigh, newPoint);
  int zero = newPoint.y();

  // Find first valid value
  int i = 0;
  data->rewind();
  while(data->next())
  {
    if(data->getValue(mCommand.at(1), value)) break;
    ++i;
  }
  scaler->valueToPixel(i, value, oldPoint);

  // Paint the chart
  while(data->next())
  {
    ++i;

    if(!data->getValue(mCommand.at(1), value)) continue;
    if(!data->getColor(mCommand.at(1), color))
      color.setNamedColor("white");

    scaler->valueToPixel(i, value, newPoint);

    QPolygon polygon;
    polygon << QPoint(oldPoint.x(), zero)
            << oldPoint
            << newPoint
            << QPoint(newPoint.x(), zero);

    painter.setPen(color);
    painter.setBrush(color);
    painter.drawPolygon(polygon);

    oldPoint = newPoint;
  }

  return true;
}
