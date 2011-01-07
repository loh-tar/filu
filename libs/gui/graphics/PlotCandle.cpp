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

#include "PlotCandle.h"

PlotCandle::PlotCandle(Newswire* parent) : PlotType(parent)
{
  mType = "Candles";
}

PlotCandle::~PlotCandle()
{}

bool PlotCandle::prepare(QStringList& command, QStringList& plotDataKeys)
{
  if(command.size() < 2)
  {
    error(FFI_, tr("To less parameter."));
    return false;
  }

  mCommand = command;
  // mCommand looks...
  // like: "CANDLE", "FI([<foo>])"
  //   or: "CANDLE", "<open>", "<high>", "<low>", "<close>", "<vol>"
  QString help = mCommand.at(1);
  help.remove(QRegExp("^FI\\(.*"));
  if(help.isEmpty()) replaceFI();
  // Now it looks like the 2nd one

  plotDataKeys.append(mCommand.at(2));  // HIGH
  plotDataKeys.append(mCommand.at(3));  // LOW

  // Add CLOSE here only that we can set mMouseYValue
  // in IndicatorPainter::useData(...)
  plotDataKeys.append(mCommand.at(4));

  return true;
}

bool PlotCandle::paint(QPaintDevice* sheet, QRect& chartArea,
                     DataTupleSet* data, Scaler* scaler)
{
  if(!data)
  {
    error(FFI_, tr("No data."));
    return false;
  }

  QPainter painter(sheet);
  painter.translate(chartArea.bottomLeft());

  QPoint topLeft, bottomRight;
  double open, high, low, close, volume, previousClose;
  QColor openC, highC, lowC, closeC, /*volumeC,*/ bodyColor, lineColor;
  int cdlWidth; // Candle width

  data->rewind();
  // Take the first close to check if the first painted candle
  // is a win-candle
  int i = -1;
  while(data->next())
  {
    ++i;
    if(!data->getValue(mCommand.at(4), previousClose)) continue;
    break;
  }

  // Find out how width we should paint a candle
  cdlWidth = scaler->intDensity() / 2;

  while(data->next())
  {
    ++i;

    if(!data->getValue(mCommand.at(1), open)) continue;
    if(!data->getColor(mCommand.at(1), openC))
      openC.setNamedColor("black");

    if(!data->getValue(mCommand.at(2), high)) continue;
    if(!data->getColor(mCommand.at(2), highC))
      highC.setNamedColor("green");

    if(!data->getValue(mCommand.at(3), low)) continue;
    if(!data->getColor(mCommand.at(3), lowC))
      lowC.setNamedColor("red");

    if(!data->getValue(mCommand.at(4), close)) continue;
    if(!data->getColor(mCommand.at(4), closeC))
      closeC = QColor(); // Set to an invalid color!

    if(!data->getValue(mCommand.at(5), volume)) continue;
//    if(!data->getColor(mCommand.at(5), volumeC))
//      volumeC.setNamedColor("white");

    if(!closeC.isValid())
    {
      // Normal mode
      if(close < previousClose) lineColor = lowC;
      else lineColor = highC;

      if(close > open) bodyColor = openC;
      else bodyColor = lineColor;
    }
    else
    {
      // Special mode
      if(close < previousClose) lineColor = lowC;
      else lineColor = highC;

      if(close > open) bodyColor = openC;
      else bodyColor = closeC;
    }

    scaler->valueToPixel(i, high, topLeft);
    scaler->valueToPixel(i, low, bottomRight);

    painter.setPen(lineColor);
    painter.drawLine(topLeft, bottomRight);

    scaler->valueToPixel(i, open, topLeft);
    scaler->valueToPixel(i, close, bottomRight);

    QRect body;//(topLeft, bottomRight);
    body.setTopLeft(topLeft);
    body.setHeight(bottomRight.y() - topLeft.y());
    body.setWidth(cdlWidth);
    body.translate(cdlWidth / -2, 0);
    if(!body.height()) body.setHeight(1);

    painter.setBrush(bodyColor);
    painter.drawRect(body);

    previousClose = close;
  }

  return true;
}

void PlotCandle::replaceFI()
{
  // We replace FI(<foo>) with foo.OPEN, foo.HIGH...
  QString fi = mCommand.at(1);
  fi.remove("FI(");
  fi.remove(")");
  if(!fi.isEmpty()) fi.append(".");

  mCommand.removeAt(1);  // The old FI(foo)
  mCommand.insert(1, fi + "OPEN");
  mCommand.insert(2, fi + "HIGH");
  mCommand.insert(3, fi + "LOW");
  mCommand.insert(4, fi + "CLOSE");
  mCommand.insert(5, fi + "VOLUME");

}
