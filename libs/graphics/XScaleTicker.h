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

#ifndef XSCALETICKER_HPP
#define XSCALETICKER_HPP

#include <QtCore>

class IndicatorPainter;

/***********************************************************************
*
*   He calculate if ticks should be plottet an delivers the scale text
*
************************************************************************/

class XScaleTicker
{
  enum TimeFrames
  {
    eNone,
    eDay,
    eWeek,
    eMonth,
    eQuarter,
    eYear
  };

  enum TickTreshold
  {
    eDayTick     = 900,
    eDayGrid     = 5000,
    eDayText     = 2800,
    eWeekTick    = 250,
    eWeekGrid    = 1200,
    eWeekText    = 820,
    eMonthTick   = 55,
    eMonthGrid   = 400,
    eMonthText   = 220,
    eQuarterTick = 5,
    eQuarterGrid = 150,
    eQuarterText = 75,
    eYearTick    = 0,
    eYearGrid    = 20
  };

  public:
                XScaleTicker(IndicatorPainter* painter);
    virtual    ~XScaleTicker();

    void        prepare();
    bool        nextTick();
    void        setLastTextRightEdge(int x);
    bool        paintDayTick(int& x, int& y);
    bool        paintWeekTick(int& x, int& y);
    bool        paintMonthTick(int& x, int& y);
    bool        paintQuarterTick(int& x, int& y);
    bool        paintYearTick(int& x, int& y);
    bool        paintTickText(QString& text);
    bool        paintGrid(int& x, int& y);

  protected:
    IndicatorPainter* mP;

    bool        mPaintText;
    bool        mPaintGrid;

    float       mDensity;
    int         mFontSpace;

    int         mLastYear;
    int         mLastQuarter;
    int         mLastMonth;
    int         mLastWeek;
    int         mLastDay;

    int         mI;                   // Index counter
    int         mX;                   // X position in pixel
    int         mLastTextRightEdge;   // ...in pixel

    int         mLastTextWriter;
    QString     mText;
};

#endif
