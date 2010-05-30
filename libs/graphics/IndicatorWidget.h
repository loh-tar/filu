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

#include "FWidget.h"

class PlotSheet;
class MyMouseEvent;
class IndicatorPicker;
class IndicatorDataView;

/***********************************************************************
*
* This class ...
*
************************************************************************/

class IndicatorWidget : public FWidget
{
  Q_OBJECT

  public:
                  IndicatorWidget(const QString& name, FWidget* parent);
                  IndicatorWidget(const QString& name, const int number, FWidget* parent);
    virtual      ~IndicatorWidget();

    void          setName(const QString& name);

    signals:
  //    void changed(QString);
    void          newSize(QList<int>*);
    void          mouse(MyMouseEvent*);

  public slots:
    void          showBarData(BarTuple* bars);
    void          showFiIdMarketId(int fiId, int marketId);
    void          useIndicator(const QString& file);
    void          setSize(QList<int> &size);
    void          mouseSlot(MyMouseEvent*);
    void          chartObjectChosen(const QString& type);

  protected slots:
    void          splitterMoved(/*int pos, int idx*/);

  protected:
    void          init(FWidget* parent);
    void          readSettings();
    void          saveSettings();

    IndicatorPicker*   mPicker;
    PlotSheet*         mSheet;
    IndicatorDataView* mDataView;
    QSplitter*         mSplitter;
    QString            mName;
    QString            mSetName;
    QString            mFullIndiSetsPath;

//    QString mIndicatorPath;
};

#endif
