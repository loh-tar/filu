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

#ifndef PLOTSHEET_HPP
#define PLOTSHEET_HPP

#include "FWidget.h"

class IndicatorPainter;
class COType;

/***********************************************************************
*
* The name is not as well chosen.
* He is almost an event handler. Takes (mouse) events and send them to
* IndicatorPainter.cpp
*
************************************************************************/

enum MyMouseEventType
{
  eDensityChanged = 0x1,
  eScrollChart    = 0x2,
  eMouseMove      = 0x4
};

//typedef struct
class MyMouseEvent
{
  public:
  QObject* sender;
  int      type;
  float    density;
  int      place4Bars;
  int      firstBarToShow;
  QPoint   mousePos;
//}MyMouseEvent;
};

class PlotSheet : public FWidget
{
  Q_OBJECT

  public:
                  PlotSheet(FWidget* parent);
                  PlotSheet(const QString& connectionName, QWidget* parent = 0);
    virtual      ~PlotSheet();

    QSize         minimumSizeHint() const;
    QSize         sizeHint() const;
    QSizePolicy   sizePolicy() const;

    bool          useIndicator(const QString& file);
    void          showBarData(BarTuple* bars);
    void          showFiIdMarketId(int fiId, int marketId);
    void          setDateRange(QDate& from, QDate& toDate);

    void          readSettings(); // not privat, call this to reload settings

    signals:
    void          mouse(MyMouseEvent*);
    void          newIndicator(const QString&);

    friend class  IndicatorWidget;
    friend class  IndicatorDataView;

  public slots:
    void          mouseSlot(MyMouseEvent*);

  protected:
    void          init();

    void          paintEvent(QPaintEvent* event);
    void          calcChartSize();
    void          densityChanged(int step);
    void          scrollChart(QPoint mousePos, int modifiers);
    void          printError();
    bool          checkForCO(QEvent* event);

    // overloaded QWidget functions
    virtual bool  event(QEvent* event);

    IndicatorPainter* mPainter;

    QSize         mSheetSize;   // available for the chart itself

    BarTuple*     mBars;        // for extern suplied bars, never delete them
    BarTuple*     mMyBars;      // for self loaded bars, delete when no longer needed
    QDate         mFromDate;
    QDate         mToDate;

    QFont         mPlotFont;
    QColor        mSheetColor;

    MyMouseEvent  mMouseEvent;
    int           mOldMouseXPos; // holds a pixel value

    QString       mNewCOType;
};

#endif
