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

#ifndef INDICATORDATAVIEW_HPP
#define INDICATORDATAVIEW_HPP

#include <QTabWidget>
#include <QTableWidget>

#include "FClass.h"
#include "IndicatorPainter.h"
class PlotSheet;
class MyMouseEvent;

/***********************************************************************
*
*   This class ...
*
************************************************************************/

class IndicatorDataView : public QTabWidget, public FClass
{
  Q_OBJECT

  public:
                      IndicatorDataView(PlotSheet* parent);
    virtual          ~IndicatorDataView();

    signals:

  public slots:
    void              mouseSlot(MyMouseEvent*);
    void              initView();
    void              initView(const QString& indicator);

  protected slots:
    void              filterChanged(int);
    void              tabChanged(int index);

  protected:
    void              readSettings();
    void              saveSettings();

    IndicatorPainter* mPainter;
    QTableWidget      mDataView;
    QTableWidget      mFilterView;
    bool              mFilterChanged;
    QString           mIndicator;      // ...name
};

#endif
