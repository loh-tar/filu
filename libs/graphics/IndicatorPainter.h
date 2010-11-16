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

#ifndef INDICATORPAINTER_HPP
#define INDICATORPAINTER_HPP

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
* What else than paint a chart is his task,
* He paint what we want to see on the screen or printer
*
************************************************************************/

class IndicatorPainter : public FWidget
{
  public:
         IndicatorPainter(PlotSheet* parent);
        ~IndicatorPainter();

    bool useIndicatorFile(const QString& file);
    bool useIndicator(QStringList& indicator);

    void useData(BarTuple* bars);
    void useSheet(QPaintDevice* sheet);

    void useFont(const QFont& font);
    void useScaleColor(const QColor& color);
    void useGridColor(const QColor& color);

    void showGrid(bool yes);
    void showXScale(bool yes);
    void setScaleToScreen(int minRange);

    void setDensity(float density);
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

    QList<PlotType*> mPlotCommands;      // rehashed plot statements, now objects
    PlotType*        mPlotType;          // base type to create special types
    QStringList      mPlotDataKeys;      // holds all names of plotted variables
    QString          mPrimaryValue;
    bool             mPrepareError;

    QSet<COType*>    mCObjects;          // holds all chart objects
    COType*          mCOInProcess;       // user is editing this

    BarTuple*        mBars;              // never delete them
    DataTupleSet*    mData;              // by the indicator calculated data

    QPaintDevice*    mSheet;             // where we will paint the result on
    QPixmap*         mStaticSheet;       // for paint chart an scale
    QPixmap*         mVolatileSheet;     // for paint the cursor etc
    bool             mUpdateStaticSheet;
    QRect            mChartArea;         // available for the chart itself (-scale area)
    Scaler* mScaler;
    float            mDensity;           // like qtstalkers "pixelspace"
    int              mPlace4Bars;        // how many bars can be displayed
    int              mFirstBarToShow;    // index number
    int              mMouseXPos;         // holds an index number, not pixel
    double           mMouseYValue;       // the value related to the mouse y position
    QDate            mMouseDate;         // yes, the date related the mouse x position

    XScaleTicker*    mXSTicker;          // calculate scale ticks and captions
    bool             mShowXScale;
    bool             mShowYScale;
    int              mScaleToScreen;     // 0 == No, > 0 yes, minRange in %
    bool             mShowGrid;

    QFont            mPlotFont;
    QColor           mSheetColor;
    QColor           mScaleColor;
    QColor           mGridColor;

    QString          mViewName;
};

#endif
