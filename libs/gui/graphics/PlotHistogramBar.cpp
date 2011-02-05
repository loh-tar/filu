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

#include "PlotHistogramBar.h"

PlotHistogramBar::PlotHistogramBar(Newswire* parent) : PlotType(parent, FUNC)
{
  mType = "HistogramBar";
}

PlotHistogramBar::~PlotHistogramBar()
{}

bool PlotHistogramBar::prepare(QStringList& command, QStringList& plotDataKeys)
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

bool PlotHistogramBar::paint(QPaintDevice* sheet, QRect& chartArea,
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
  QPoint topLeft, bottomRight;
  QColor color;
  int barWidth;

  // Find out how width we should paint a bar
  barWidth = scaler->intDensity() / 2;

  // Check if the zero-line is visible
  scaler->getHighLow(maxHigh, minLow);
  scaler->valueToPixel(1, 0.0, bottomRight);
  if(minLow  > 0) scaler->valueToPixel(1, minLow, bottomRight);
  if(maxHigh < 0) scaler->valueToPixel(1, maxHigh, bottomRight);
  int zero = bottomRight.y();

  int i = -1;
  data->rewind();
  // Paint the chart
  while(data->next())
  {
    ++i;

    if(!data->getValue(mCommand.at(1), value)) continue;
    if(!data->getColor(mCommand.at(1), color))
      color.setNamedColor("white");

    scaler->valueToPixel(i, value, topLeft);
    bottomRight = topLeft;
    bottomRight.setY(zero);

    QRect bar(topLeft, bottomRight);
    bar.setWidth(barWidth);
    bar.translate(barWidth / -2, 0);
    if(!bar.width()) bar.setWidth(1);

    painter.setPen(color);
    painter.setBrush(color);
    painter.drawRect(bar);
  }

  return true;
}
