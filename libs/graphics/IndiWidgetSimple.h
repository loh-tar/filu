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

#ifndef INDIWIDGETSIMPLE_HPP
#define INDIWIDGETSIMPLE_HPP

#include "FWidget.h"

class PlotSheet;
class MyMouseEvent;

/***********************************************************************
*
*  This class is something like a wrapper for the PlotSheet class.
*  He save and restore all settings PlotSheet need. To do the job he
*  has to forward all mouse events to PlotSheet.
*
************************************************************************/

class IndiWidgetSimple : public FWidget
{
  Q_OBJECT

  public:
                  IndiWidgetSimple(const QString& name, FWidget* parent);
                  IndiWidgetSimple(const QString& name, const int number, FWidget* parent);
    virtual      ~IndiWidgetSimple();

    void          setName(const QString& name);

    signals:
    void          mouse(MyMouseEvent*);

  public slots:
    virtual void  showBarData(BarTuple* bars);
    virtual void  showFiIdMarketId(int fiId, int marketId);
            void  useIndicator(const QString& file);
    virtual void  mouseSlot(MyMouseEvent*);
    void          chartObjectChosen(const QString& type);

  protected:
    void          init();
    void          readSettings();
    void          saveSettings();

    PlotSheet*    mSheet;
    QString       mName;
    QString       mSetName;
    QString       mFullIndiSetsPath;
};

#endif