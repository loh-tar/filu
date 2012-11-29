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

#ifndef PLOTSHEET_HPP
#define PLOTSHEET_HPP

#include <QDate>

#include "FWidget.h"

class IndicatorPainter;
class COType;

/***********************************************************************
*
*   The name is not as well chosen.
*   He is almost an event handler. Takes (mouse) events and send them to
*   IndicatorPainter.cpp
*
************************************************************************/

enum MyMouseEventType
{
  eDensityChanged = 0x01,
  eScrollChart    = 0x02,
  eMouseMove      = 0x04,
  eShowYScale     = 0x08,
  eSyncReq        = 0x10,   // Request
  eSyncDec        = 0x20    // Decree
};

//typedef struct
class MyMouseEvent
{
  public:
  QObject* sender;
  QObject* requester;
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
                  PlotSheet(FClass* parent);
    virtual      ~PlotSheet();

    QSize         minimumSizeHint() const;
    QSize         sizeHint() const;

    bool          useIndicator(const QString& file);
    void          showBarData(BarTuple* bars);
    void          showFiIdMarketId(int fiId, int marketId);
    void          setDateRange(const QDate& from, const QDate& toDate);

    signals:
    void          mouse(MyMouseEvent*);
    void          newIndicator(const QString&);

    friend class  IndiWidgetSimple;
    friend class  IndicatorDataView;

  public slots:
    void          mouseSlot(MyMouseEvent*);
    void          useFont(const QFont& font);
    void          useScaleColor(const QColor& color);
    void          useGridColor(const QColor& color);
    void          useSheetColor(const QColor& color);
    void          setDensity(float density);
    void          setScaleToScreen(int minRange);
    void          showGrid(bool yes = true);
    void          showXScale(bool yes = true);
    void          showYScale(bool yes = true);

  protected:
    void          init();

    void          paintEvent(QPaintEvent* event);
    void          calcChartSize();
    void          densityChanged(int step);
    void          scrollChart(QPoint mousePos, int modifiers);
    void          printError();
    bool          checkForCO(QEvent* event);

    // Overloaded QWidget functions
    virtual bool  event(QEvent* event);

    IndicatorPainter* mPainter;

    QSize         mSheetSize;   // Available for the chart itself

    BarTuple*     mBars;        // For extern suplied bars, never delete them
    BarTuple*     mMyBars;      // For self loaded bars, delete when no longer needed
    QDate         mFromDate;
    QDate         mToDate;

    MyMouseEvent  mMouseEvent;
    int           mOldMouseXPos; // Holds a pixel value

    QString       mNewCOType;

    QAction*      mActShowGrid;
    QAction*      mActShowXScale;
    QAction*      mActShowYScale;
};

#endif
