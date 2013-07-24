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

#ifndef XSCALETICKER_HPP
#define XSCALETICKER_HPP

#include <QString>

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
    eHalfYear,
    eYear,
    e2Year,
    e5Year,
    e10Year
  };

  enum TickTreshold
  {
    eDayTick     = 900,
    eDayGrid     = 5000,
    eDayText     = 2800,

    eWeekTick    = 250,
    eWeekGrid    = 1800,
    eWeekText    = 820,

    eMonthTick   = 55,
    eMonthGrid   = 400,
    eMonthText   = 180,

    eQuarterTick = 20,
    eQuarterGrid = 150,
    eQuarterText = 75,

    eHalfYTick   = 5,
    eHalfYGrid   = 50,
    eHalfYText   = 30,

    eYearTick    = 2,
    eYearGrid    = 16,
    eYearText    = 16,

    e2YearTick   = 2,
    e2YearGrid   = 8,
    e2YearText   = 8,

    e5YearTick   = 1,
    e5YearGrid   = 4,
    e5YearText   = 4,

    e10YearTick  = 1,
    e10YearGrid  = 2,
    e10YearText  = 2
  };

  enum TickLength
  {
    eNormalTick  = 4,
    eTextTick    = 6
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
    bool        paintHalfYTick(int& x, int& y);
    bool        paintYearTick(int& x, int& y);
    bool        paint2YearTick(int& x, int& y);
    bool        paint5YearTick(int& x, int& y);
    bool        paint10YearTick(int& x, int& y);

    bool        paintTickText(QString& text);
    bool        paintGrid(int& x, int& y);

  protected:
    IndicatorPainter* mP;

    bool        mPaintText;
    bool        mPaintGrid;

    float       mDensity;
    int         mFontSpace;

    int         mLast10Year;
    int         mLast5Year;
    int         mLast2Year;
    int         mLastYear;
    int         mLastHalfYear;
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
