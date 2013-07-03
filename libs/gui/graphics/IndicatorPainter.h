//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
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

#ifndef INDICATORPAINTER_HPP
#define INDICATORPAINTER_HPP

#include <QDate>
#include <QSet>

#include "FWidget.h"

class Indicator;
class PlotSheet;
class PlotType;
class Scaler;
class XScaleTicker;
class IndicatorDataView;
class DataTupleSet;
class COType;
class Grip;

/***********************************************************************
*
*   What else than paint a chart is his task,
*   He paint what we want to see on the screen or printer
*
************************************************************************/

class IndicatorPainter : public FWidget
{
  Q_DECLARE_TR_FUNCTIONS(IndicatorPainter)

  public:
         IndicatorPainter(PlotSheet* parent);
        ~IndicatorPainter();

    bool useIndicatorFile(const QString& file);
    bool useIndicator(QStringList& indicator);

    void useData(BarTuple* bars);
    void useSheet(QPaintDevice* sheet);

    bool densityChanged(int step);
    void setIndicatingRange(int from, int count);
    bool scrollHorizontal(int step);
    bool setMousePos(const QPoint& pos);

    bool paint();

  friend class Scaler;
  friend class XScaleTicker;
  friend class PlotSheet;
  friend class IndiWidgetSimple;
  friend class IndicatorDataView;
  friend class COType;
  friend class Grip;

  protected:
    bool calculate();
    bool parse(const QStringList& indiFile);
    bool prepare(QStringList* indiFile);
    void readSettings();

    void calcSizes();
    void paintXScale();
    void paintYScale();
    void paintCrosshair();

    Indicator*       mIndicator;

    QList<PlotType*> mPlotCommands;      // Rehashed plot statements, now objects
    QStringList      mPlotDataKeys;      // Holds all names of plotted variables
    QString          mPrimaryValue;

    QSet<COType*>    mCObjects;          // Holds all chart objects
    COType*          mCOInProcess;       // User is editing this

    BarTuple*        mBars;              // Never delete them
    DataTupleSet*    mData;              // By the indicator calculated data

    QPaintDevice*    mSheet;             // Where we will paint the result on
    QPixmap*         mStaticSheet;       // For paint chart an scale
    QPixmap*         mVolatileSheet;     // For paint the cursor etc
    bool             mUpdateStaticSheet;
    QRect            mChartArea;         // Available for the chart itself (-scale area)
    Scaler*          mScaler;
    float            mDensity;           // Like qtstalkers "pixelspace"
    int              mPlace4Bars;        // How many bars can be displayed
    int              mFirstBarToShow;    // Index number
    int              mMouseXPos;         // Holds an index number, not pixel
    double           mMouseYValue;       // The value related to the mouse y position
    QDate            mMouseDate;         // Yes, the date related the mouse x position

    XScaleTicker*    mXSTicker;          // Calculate scale ticks and captions
    bool             mShowXScale;
    bool             mShowYScale;
    int              mScaleToScreen;     // 0 == No, > 0 yes, minRange in %
    bool             mShowGrid;
    bool             mShowPercentScale;

    QFont            mPlotFont;
    QColor           mSheetColor;
    QColor           mScaleColor;
    QColor           mGridColor;

    QString          mViewName;
};

#endif
