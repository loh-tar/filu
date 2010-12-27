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

#ifndef PLOTTYPE_HPP
#define PLOTTYPE_HPP

#include <QtGui>

// #include not class, each derived need these
#include "DataTupleSet.h"
#include "Scaler.h"

/***********************************************************************
*
*   This is the base class of all kind of PLOT(....) objects
*
************************************************************************/

class PlotType
{
  public:
                  PlotType();
    virtual      ~PlotType();

    virtual       PlotType* createNew(const QString& type = "");
    virtual bool  prepare(QStringList& command, QStringList& plotDataKeys);
    virtual bool  paint(QPaintDevice* sheet, QRect& chartArea, DataTupleSet* data, Scaler* scaler);

    QString       getType();
    const QStringList& errorText() const { return mErrorMessage; };

  protected:
    QString       mType;
    QStringList   mCommand;
    QStringList   mErrorMessage;

  private:
    PlotType*     createNewType(const QString& type);

};

#endif
