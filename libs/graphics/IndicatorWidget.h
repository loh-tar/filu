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

#ifndef INDICATORWIDGET_HPP
#define INDICATORWIDGET_HPP

#include "IndiWidgetSimple.h"

class IndicatorPicker;
class IndicatorDataView;

/***********************************************************************
*
*   He is the same as IndiWidgetSimple with the improvement of
*   additional IndicatorPicker and IndicatorDataView.
*
************************************************************************/

class IndicatorWidget : public IndiWidgetSimple
{
  Q_OBJECT

  public:
                  IndicatorWidget(const QString& name, FWidget* parent);
                  IndicatorWidget(const QString& name, const int number, FWidget* parent);
    virtual      ~IndicatorWidget();

    signals:
    void          newSize(QList<int>*);

  public slots:
    void          showBarData(BarTuple* bars);
    void          showFiIdMarketId(int fiId, int marketId);
    void          setSize(QList<int>& size);
    void          mouseSlot(MyMouseEvent*);

  protected slots:
    void          splitterMoved();

  protected:
    void          init();
    void          readSettings();
    void          saveSettings();

    IndicatorPicker*   mPicker;
    IndicatorDataView* mDataView;
    QSplitter*         mSplitter;

};

#endif
