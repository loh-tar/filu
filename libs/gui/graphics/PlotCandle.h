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

#ifndef PLOTCANDLES_HPP
#define PLOTCANDLES_HPP

#include "PlotType.h"

/***********************************************************************
*
*   Yes, it plots data as a candles
*
************************************************************************/

class PlotCandle : public PlotType
{
  public:
                  PlotCandle();
    virtual      ~PlotCandle();

    virtual PlotCandle* createNew(const QString& type = "");

    virtual bool  prepare(QStringList& command, QStringList& plotDataKeys);
    virtual bool  paint(QPaintDevice* sheet, QRect& chartArea, DataTupleSet* data, Scaler* scaler);

  protected:
    void replaceFI();
};

#endif