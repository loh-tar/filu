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

#include <math.h>

#include "Scaler.h"
#include "IndicatorPainter.h"

Scaler::Scaler(IndicatorPainter* painter)
      : mP(painter)
{}

Scaler::~Scaler()
{}

void Scaler::setHighLow(const double maxHigh, const double minLow)
{
  mMaxHigh = maxHigh;
  mMinLow  = minLow;

  double average = (fabs(maxHigh) + fabs(minLow)) / 2.0;
  mIndicatingRange = fabs(maxHigh - minLow);

  // adjust high/low to fit mScaleToScreen minumum
  double  percentRange = (mIndicatingRange / average) * 100.0;
  if(percentRange < mP->mScaleToScreen)
  {
    percentRange = mP->mScaleToScreen / 2.0;
    mMaxHigh = average + (average * percentRange) / 100.0;
    mMinLow  = average - (average * percentRange) / 100.0;
    mIndicatingRange = mMaxHigh - mMinLow;
  }

  mXAdjustment = mP->mChartArea.size().width() - (int)(mP->mPlace4Bars * mP->mDensity);
  mXAdjustment -= intDensity();

  mFactor = mP->mChartArea.size().height() / mIndicatingRange;
}

void Scaler::getHighLow(double& maxHigh, double& minLow)
{
  maxHigh = mMaxHigh;
  minLow  = mMinLow;
}

int Scaler::valueToPixel(int i, const double& val, QPoint& pos)
{
  int quality = Tuple::eValid;

  if((mP->mFirstBarToShow + i) > (mP->mBars->count() - 1))
  {
    quality = Tuple::eOverRange;
    //i = mP->mBars->count() - 1;
  }
  if((mP->mFirstBarToShow + i) < 0)
  {
    quality = Tuple::eUnderRange;
    //i = 0;
  }

  pos.setX((int)((i * mP->mDensity) + mXAdjustment));
  pos.setY(calcToPixel(val));

  return quality;
}

int Scaler::pixelToValue(const QPoint& pos, int& idx, double& value, QDate& date)
{
  // Returns true/fals as bit-flag if idx/value is in a valid range.
  // Does* not* correct these values because its needed by CO/Grip.
  // Only date is adjusted to a valid value.

  // FIXME: The case xValid=false needs improvements -> date/idx is wrong.
  //        Calc the intersection to last available date/idx.
  //        hm, is that all still valid?

  int valid = eXValid | eYValid; // set x and y to valid

  // calc the relating data index number
  idx = (int)((pos.x() + (mP->mDensity / 2.0) - mXAdjustment) / mP->mDensity);

  // and check if the result is in the visible area
  if(idx > mP->mPlace4Bars)
  {
    valid ^= eXValid; // set x to not valid
    //idx =  mP->mPlace4Bars;
  }
  else if(idx > (mP->mBars->count() - 1))
  {
    valid ^= eXValid;
    //idx = mP->mBars->count() - 1;
  }
  else if(idx < 0)
  {
    valid ^= eXValid;
    //idx = 0;
  }

  // calc the relating value but notice if the result is visible
  int y = pos.y();
  if(-y < 0)
  {
    valid ^= eYValid; // set y to not valid
    //y = 0;
  }
  if(-y > mP->mChartArea.height() - 1)
  {
    valid ^= eYValid;
    //y = mP->mChartArea.height() - 1;
  }

  value = calcToValue(y);

  // and last, fetch the date but take care we are not out of available data
  int i = mP->mFirstBarToShow + idx;
  if(i > (mP->mBars->count() - 1)) i = mP->mBars->count() - 1;
  if(i < 0) i = 0;

  mP->mBars->rewind(i);
  date = mP->mBars->date();

  return valid;
}

int Scaler::topEdge()
{
  return -mP->mChartArea.size().height();
}

int Scaler::intDensity()
{
  int d = (int)mP->mDensity;
  if(!d) d = 1;

  return d;
}

int Scaler::calcToPixel(const double& val)
{
  int y;

  y  = (val - mMinLow) * mFactor;

  y *= -1;

  return y;
}

double Scaler::calcToValue(const int& y)
{
  //return mMaxHigh - (pixel / mFactor);
  return mMinLow + (-y / mFactor);
}

bool Scaler::beginYTicking()
{
  if(mMaxHigh < mMinLow) return false; // hm, only unvalid data, nothing to plot

  int tickDensity = 50;
  //int tickCount = mP->mChartArea.height() / tickDensity;
  int tickCount = mP->mChartArea.size().height() / tickDensity;
  if(tickCount == 0) return false;

  double valueRange = mMaxHigh - mMinLow;
  if(valueRange == 0.0)
  {
    //addErrorText("IndicatorPainter::paintYScale: can't paint, mMaxHigh = mMinLow");
    return false;
  }

  mValuePerTick = valueRange / tickCount;
  double power = floor(log10(mValuePerTick));
  mValuePerTick = ceil(mValuePerTick / pow(10, power - 2));
  valueRange = ceil(valueRange / pow(10, power - 2));

  if(tickCount * 100.0 < valueRange) mValuePerTick = 100.0;
  if(tickCount * 125.0 < valueRange) mValuePerTick = 125.0;
  if(tickCount * 200.0 < valueRange) mValuePerTick = 200.0;
  if(tickCount * 250.0 < valueRange) mValuePerTick = 250.0;
  if(tickCount * 300.0 < valueRange) mValuePerTick = 300.0;
  if(tickCount * 400.0 < valueRange) mValuePerTick = 400.0;
  if(tickCount * 500.0 < valueRange) mValuePerTick = 500.0;
  if(tickCount * 600.0 < valueRange) mValuePerTick = 600.0;
  if(tickCount * 700.0 < valueRange) mValuePerTick = 700.0;
  if(tickCount * 750.0 < valueRange) mValuePerTick = 750.0;
  if(tickCount * 800.0 < valueRange) mValuePerTick = 800.0;
  if(tickCount * 900.0 < valueRange) mValuePerTick = 900.0;
  if(tickCount * 1000.0 < valueRange) mValuePerTick = 1000.0;

  mValuePerTick = mValuePerTick * pow(10, power - 2);

  // calc first tick value
  mTickValue = ((int)(mMinLow / mValuePerTick) -1) * mValuePerTick;
  if(mTickValue < mMinLow) mTickValue += mValuePerTick;

  return true;
}

bool Scaler::nextYTick(int& y, QString& text)
{
  if(mValuePerTick == 0.0)  return false;

  mTickValue += mValuePerTick;

  if(mTickValue > mMaxHigh) return false;

  y = calcToPixel(mTickValue);
  getValueText(mTickValue, text);

  return true;
}

void Scaler::getValueText(const double& val, QString& text)
{
  if(val >= 100000000)
  {
    text = QLocale().toString(val / 1000000000, 'f', 2);
    text.append(QObject::tr("bill"));
  }

  else if(val >= 100000)
  {
    text = QLocale().toString(val / 1000000, 'f', 2);
    text.append(QObject::tr("mill"));
  }
  else if(val >= 1000)
  {
    text = QLocale().toString(val / 1000, 'f', 2);
    text.append(QObject::tr("thou"));
  }
  else
  {
    text = QLocale().toString(val, 'f', 2);
  }

}

void Scaler::getErrorText(QStringList& errorMessage)
{
  for(int i = 0; i < mErrorMessage.size(); i++)
    errorMessage.append(mErrorMessage.at(i));

  mErrorMessage.clear();
}
