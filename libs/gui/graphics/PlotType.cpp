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

#include "PlotType.h"
#include "PlotLine.h"
#include "PlotCandle.h"
#include "PlotHistogram.h"
#include "PlotHistogramBar.h"
#include "PlotDashLine.h"

PlotType::PlotType(Newswire* parent)
        : Newswire(parent)
{
  mType = "BaseType";
}

PlotType::~PlotType()
{}

PlotType* PlotType::createNew(const QString& type)
{
  if(type.isEmpty()) return new PlotType(this);
  else return createNewType(type);
}

bool PlotType::prepare(QStringList &/*command*/, QStringList &/*plotDataKeys*/)
{
  error(FFI_, tr("Oops!? Base type never can prepare."));
  return false;
}

bool PlotType::paint(QPaintDevice */*sheet*/, QRect &/*chartArea*/,
                     DataTupleSet */*data*/, Scaler */*scaler*/)
{
  error(FFI_, tr("Oops?! Base type nerver can paint."));
  return false;
}

QString PlotType::getType()
{
  return mType;
}

PlotType* PlotType::createNewType(const QString& type)
{
  if(!type.compare("LINE"))         return new PlotLine(this);
  if(!type.compare("CANDLE"))       return new PlotCandle(this);
  if(!type.compare("HISTOGRAM"))    return new PlotHistogram(this);
  if(!type.compare("HISTOGRAMBAR")) return new PlotHistogramBar(this);
  if(!type.compare("DASHLINE"))     return new PlotDashLine(this);

  /*FIXME: these plot types has to be implemented

  if(!type.compare("BAR")) return new PlotBars;
  if(!type.compare("CO")) return new PlotCOs;


  if(!type.compare("DOT")) return new PlotDots;
  if(!type.compare("LABELS")) return new PlotLables;

  */

  // TA-Lib does sometime not say what kind auf plot should be use.
  // In that case delivers TALib.cpp "???" and we use "Line" instead.
  if(!type.compare("???")) return new PlotLine(this);

  error(FFI_, tr("Type not found: %1").arg(type));
  return 0;
}
