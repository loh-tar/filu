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

#include "XScaleTicker.h"

#include "BarTuple.h"
#include "IndicatorPainter.h"
#include "Scaler.h"

XScaleTicker::XScaleTicker(IndicatorPainter* painter)
            : mP(painter)
            , mPaintText(false)
            , mPaintGrid(false)
            , mLast10Year(0)
            , mLast5Year(0)
            , mLast2Year(0)
            , mLastYear(0)
            , mLastQuarter(0)
            , mLastMonth(0)
            , mLastWeek(0)
            , mLastDay(0)

{}

XScaleTicker::~XScaleTicker()
{}

void XScaleTicker::prepare()
{
  mP->mBars->rewind(mP->mFirstBarToShow - 1);

  mI = -1;
  mLast10Year = 0;
  mLast5Year = 0;
  mLast2Year = 0;
  mLastYear = 0;
  mLastHalfYear = 0;
  mLastQuarter = 0;
  mLastMonth = 0;
  mLastWeek = 0;
  mLastDay = 0;
  mLastTextWriter = -1;
  mDensity = mP->mDensity * 100.0;
  mLastTextRightEdge = -1000;

  QFontMetrics fm(mP->mPlotFont);
  mFontSpace = fm.width("XX");
}

bool XScaleTicker::nextTick()
{
  ++mI;

  if(mI > mP->mPlace4Bars) return false;

  mPaintGrid = false;

  QPoint newPoint;
  // 2nd parameter "1.0" is only a dummy
  mP->mScaler->valueToPixel(mI, 1.0, newPoint);
  mX = newPoint.x();

  mText.clear();

  if(mLastTextRightEdge < (mX - mFontSpace))
  {
    mLastTextWriter = eNone;
  }

  return mP->mBars->next();
}

void XScaleTicker::setLastTextRightEdge(int x)
{
  mLastTextRightEdge = x;
}

bool XScaleTicker::paintDayTick(int& x, int& y)
{
  if(mDensity > eDayTick)
  {
    y = eNormalTick;
    x = mX;
    if( (mDensity > eDayText) and (mLastTextWriter <= eDay) )
    {
      mText = QString::number(mP->mBars->date().day());
      mLastTextWriter = eDay;
      y = eTextTick;
    }

    if(mDensity > eDayGrid) mPaintGrid = true;

    if(mP->mShowXScale) return true;
  }

  return false;
}

bool XScaleTicker::paintWeekTick(int& x, int& y)
{
  int week  = mP->mBars->date().weekNumber();

  if((week != mLastWeek) and (mDensity > eWeekTick))
  {
    x = mX;
    y = eNormalTick;
    mLastWeek = week;
    int day = mP->mBars->date().day();
    // "and day < .." to avoid ugly overwrites
    if( (mDensity > eWeekText) and (mLastTextWriter <= eWeek) and (day < 29) )
    {
      mText = QString::number(day);
      mLastTextWriter = eWeek;
      y = eTextTick;
    }

    if( (mDensity > eWeekGrid) and (mDensity < eDayGrid) ) mPaintGrid = true;

    if(mP->mShowXScale) return true;
  }

  return false;
}

bool XScaleTicker::paintMonthTick(int& x, int& y)
{
  int month = mP->mBars->date().month();

  if((month != mLastMonth) and (mDensity > eMonthTick))
  {
    x = mX;
    y = eNormalTick;
    mLastMonth = month;
    if( (mDensity > eMonthText) and (mLastTextWriter <= eMonth) )
    {
      mText = QDate::shortMonthName(month);
      mLastTextWriter = eMonth;
      y = eTextTick;
    }

    if( (mDensity > eMonthGrid) and (mDensity < eWeekGrid) ) mPaintGrid = true;

    if(mP->mShowXScale) return true;
  }

  return false;
}

bool XScaleTicker::paintQuarterTick(int& x, int& y)
{
  int month   = mP->mBars->date().month();
  int quarter = (month + 2) / 3;

  if((quarter != mLastQuarter) and (mDensity > eQuarterTick))
  {
    x = mX;
    y = eNormalTick;
    mLastQuarter = quarter;
    if( (mDensity > eQuarterText) and (mLastTextWriter <= eQuarter) )
    {
      mText = QDate::shortMonthName(month);
      mLastTextWriter = eQuarter;
      y = eTextTick;
    }

    if( (mDensity > eQuarterGrid) and (mDensity < eMonthGrid) ) mPaintGrid = true;

    if(mP->mShowXScale) return true;
  }

  return false;
}

bool XScaleTicker::paintHalfYTick(int& x, int& y)
{
  int month    = mP->mBars->date().month();
  int halfYear = (month + 5) / 6;

  if((halfYear != mLastHalfYear) and (mDensity > eHalfYTick))
  {
    x = mX;
    y = eNormalTick;
    mLastHalfYear = halfYear;
    if( (mDensity > eHalfYText) and (mLastTextWriter <= eHalfYear) )
    {
      mText = QDate::shortMonthName(month);
      mLastTextWriter = eHalfYear;
      y = eTextTick;
    }

    if( (mDensity > eHalfYGrid) and (mDensity < eQuarterGrid) ) mPaintGrid = true;

    if(mP->mShowXScale) return true;
  }

  return false;
}

bool XScaleTicker::paintYearTick(int& x, int& y)
{
  int year = mP->mBars->date().year();

  if((year != mLastYear) and (mDensity > eYearTick))
  {
    x = mX;
    y = eNormalTick;
    mLastYear = year;
    if( (mDensity > eYearText) and (mLastTextWriter <= eYear) )
    {
      mText = QString::number(year);
      mLastTextWriter = eYear;
      y = eTextTick;
    }

    if( (mDensity > eYearGrid) and (mDensity < eHalfYGrid) ) mPaintGrid = true;

    if(mP->mShowXScale) return true;
  }

  return false;
}

bool XScaleTicker::paint2YearTick(int& x, int& y)
{
  int year  = mP->mBars->date().year();
  int year2 = year / 2;

  if((year2 != mLast2Year) and (mDensity > e2YearTick))
  {
    x = mX;
    y = eNormalTick;
    mLastYear  = year2;
    mLast2Year = year2;
    if( (mDensity > e2YearText) and (mLastTextWriter <= e2Year) )
    {
      mText = QString::number(year);
      mLastTextWriter = e2Year;
      y = eTextTick;
    }

    if( (mDensity > e2YearGrid) and (mDensity < eYearGrid) ) mPaintGrid = true;

    if(mP->mShowXScale) return true;
  }

  return false;
}

bool XScaleTicker::paint5YearTick(int& x, int& y)
{
  int year  = mP->mBars->date().year();
  int year5 = year / 5;

  if((year5 != mLast5Year) and (mDensity > e5YearTick) and (mDensity < e2YearText))
  {
    x = mX;
    y = eNormalTick;
    mLastYear  = year5;
    mLast2Year = year5;
    mLast5Year = year5;
    if( (mDensity > e5YearText) and (mLastTextWriter <= e5Year) )
    {
      mText = QString::number(year);
      mLastTextWriter = e5Year;
      y = eTextTick;
    }

    if( (mDensity > e5YearGrid) and (mDensity < e2YearGrid) ) mPaintGrid = true;

    if(mP->mShowXScale) return true;
  }

  return false;
}

bool XScaleTicker::paint10YearTick(int& x, int& y)
{
  int year   = mP->mBars->date().year();
  int year10 = year / 10;

  if((year10 != mLast10Year) and (mDensity > e10YearTick))
  {
    x = mX;
    y = eNormalTick;
    mLastYear   = year10;
    mLast2Year  = year10;
    mLast5Year  = year10;
    mLast10Year = year10;
    if( (mDensity > e10YearText) and (mLastTextWriter <= e10Year) )
    {
      mText = QString::number(year);
      mLastTextWriter = e10Year;
      y = eTextTick;
    }

    if( (mDensity > e10YearGrid) and (mDensity < e5YearGrid) ) mPaintGrid = true;

    if(mP->mShowXScale) return true;
  }

  return false;
}

bool XScaleTicker::paintTickText(QString& text)
{
  if(!mP->mShowXScale) return false;
  if(mText.isEmpty()) return false;
  if(mX < 1)  return false;

  text = mText;

  return true;
}

bool XScaleTicker::paintGrid(int& x, int& y)
{
  if(!mP->mShowGrid) return false;
  if(!mPaintGrid) return false;
  if(mX < 1)  return false;

  x = mX;
  y = -mP->mChartArea.height();

  return true;
}
