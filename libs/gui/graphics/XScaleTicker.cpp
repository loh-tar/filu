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
            , mPaintTick(false)
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
  mI = -1;
  mP->mBars->rewind(mP->mFirstBarToShow - 1);
  mDensity = mP->mDensity * 100.0;
  mLastTextRightEdge = -1000;
  mLastTextWriter = -1;

  QFontMetrics fm(mP->mPlotFont);
  mFontSpace = fm.width("XX");
}

bool XScaleTicker::nextTick()
{
  ++mI;

  if(mI > mP->mPlace4Bars) return false;

  mLastWeek     = mP->mBars->date().weekNumber();
  mLastMonth    = mP->mBars->date().month();
  mLastQuarter  = (mMonth + 2) / 3;
  mLastHalfYear = (mMonth + 5) / 6;
  mLastYear     = mP->mBars->date().year();
  mLast2Year    = mYear / 2;
  mLast5Year    = mYear / 5;
  mLast10Year   = mYear / 10;

  mPaintTick = false;
  mPaintGrid = false;
  mText.clear();

  QPoint point;
  mP->mScaler->valueToPixel(mI, 1.0, point); // Parameter "1.0" is only a dummy
  mX = point.x();

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

bool XScaleTicker::paintTick(int& x, int& y)
{
  mWeek     = mP->mBars->date().weekNumber();
  mMonth    = mP->mBars->date().month();
  mQuarter  = (mMonth + 2) / 3;
  mHalfYear = (mMonth + 5) / 6;
  mYear     = mP->mBars->date().year();
  m2Year    = mYear / 2;
  m5Year    = mYear / 5;
  m10Year   = mYear / 10;
  mY        = eNormalTick;

  checkDayTick();
  checkWeekTick();
  checkMonthTick();
  checkQuarterTick();
  checkHalfYTick();
  checkYearTick();
  check2YearTick();
  check5YearTick();
  check10YearTick();

  x = mX;
  y = mY;

  if(mI < 1) return false; // Avoid ugly outer left painting
  if(mP->mShowXScale) return mPaintTick;

  return false;
}

void XScaleTicker::checkDayTick()
{
  if(mDensity < eDayTick) return;

  mPaintTick = true;

  if(mDensity > eDayText)
  {
    mY = eTextTick;
    if(mLastTextWriter <= eDay)
    {
      mText = QString::number(mP->mBars->date().day());
      mLastTextWriter = eDay;
    }
  }

  if(mDensity > eDayGrid) mPaintGrid = true;
}

void XScaleTicker::checkWeekTick()
{
  if((mWeek == mLastWeek) or (mDensity < eWeekTick)) return;

  mPaintTick = true;

  if(mDensity > eWeekText)
  {
    mY = eTextTick;
    int day = mP->mBars->date().day();
    if((mLastTextWriter <= eWeek) and (day < 29)) // "and day < .." to avoid ugly overwrites
    {
      mText = QString::number(day);
      mLastTextWriter = eWeek;
    }
  }

  if(mDensity > eWeekGrid) mPaintGrid = true;
}

void XScaleTicker::checkMonthTick()
{
  if((mMonth == mLastMonth) or (mDensity < eMonthTick)) return;

  mPaintTick = true;

  if(mDensity > eMonthText)
  {
    mY = eTextTick;
    if(mLastTextWriter <= eMonth)
    {
      mText = QDate::shortMonthName(mMonth);
      mLastTextWriter = eMonth;
    }
  }

  if( (mDensity > eMonthGrid) and (mDensity < eWeekGrid) ) mPaintGrid = true;
}

void XScaleTicker::checkQuarterTick()
{
  if((mQuarter == mLastQuarter) or (mDensity < eQuarterTick)) return;

  mPaintTick = true;

  if(mDensity > eQuarterText)
  {
    mY = eTextTick;
    if(mLastTextWriter <= eQuarter)
    {
      mText = QDate::shortMonthName(mMonth);
      mLastTextWriter = eQuarter;
    }
  }

  if( (mDensity > eQuarterGrid) and (mDensity < eMonthGrid) ) mPaintGrid = true;
}

void XScaleTicker::checkHalfYTick()
{
  if((mHalfYear == mLastHalfYear) or (mDensity < eHalfYTick)) return;

  mPaintTick = true;

  if(mDensity > eHalfYText)
  {
    mY = eTextTick;
    if(mLastTextWriter <= eHalfYear)
    {
      mText = QDate::shortMonthName(mMonth);
      mLastTextWriter = eHalfYear;
    }
  }

  if( (mDensity > eHalfYGrid) and (mDensity < eQuarterGrid) ) mPaintGrid = true;
}

void XScaleTicker::checkYearTick()
{
  if((mYear == mLastYear) or (mDensity < eYearTick)) return;

  mPaintTick = true;

  if(mDensity > eYearText)
  {
    mY = eTextTick;
    if(mLastTextWriter <= eYear)
    {
      mText = QString::number(mYear);
      mLastTextWriter = eYear;
    }
  }

  if( (mDensity > eYearGrid) and (mDensity < eHalfYGrid) ) mPaintGrid = true;
}

void XScaleTicker::check2YearTick()
{
  if((m2Year == mLast2Year) or (mDensity < e2YearTick)) return;

  mPaintTick = true;

  if(mDensity > e2YearText)
  {
    mY = eTextTick;
    if(mLastTextWriter <= e2Year)
    {
      mText = QString::number(mYear);
      mLastTextWriter = e2Year;
    }
  }

  if( (mDensity > e2YearGrid) and (mDensity < eYearGrid) ) mPaintGrid = true;
}

void XScaleTicker::check5YearTick()
{
  if((m5Year == mLast5Year) or (mDensity < e5YearTick) or (mDensity > e2YearText)) return;

  mPaintTick = true;

  if(mDensity > e5YearText)
  {
    mY = eTextTick;
    if(mLastTextWriter <= e5Year)
    {
      mText = QString::number(mYear);
      mLastTextWriter = e5Year;
    }
  }

  if( (mDensity > e5YearGrid) and (mDensity < e2YearGrid) ) mPaintGrid = true;
}

void XScaleTicker::check10YearTick()
{
  if((m10Year == mLast10Year) or (mDensity < e10YearTick)) return;

  mPaintTick = true;

  if(mDensity > e10YearText)
  {
    mY = eTextTick;
    if(mLastTextWriter <= e10Year)
    {
      mText = QString::number(mYear);
      mLastTextWriter = e10Year;
    }
  }

  if( (mDensity > e10YearGrid) and (mDensity < e5YearGrid) ) mPaintGrid = true;
}

bool XScaleTicker::paintTickText(QString& text)
{
  if(!mP->mShowXScale) return false;
  if(mText.isEmpty()) return false;
  if(mI < 1)  return false;

  text = mText;

  return true;
}

bool XScaleTicker::paintGrid(int& x, int& y)
{
  if(!mP->mShowGrid) return false;
  if(!mPaintGrid) return false;
  if(mI < 1)  return false;

  x = mX;
  y = -mP->mChartArea.height();

  return true;
}
