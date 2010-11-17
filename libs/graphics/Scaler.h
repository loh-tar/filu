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

#ifndef SCALER_HPP
#define SCALER_HPP

#include <QtCore>

/***********************************************************************
*
* This class calc (real)values to pixel and vice versa
*
************************************************************************/

class IndicatorPainter;

class Scaler
{
  public:

    enum SomeDefines
    {
      eNotValid = 0,
      eXValid   = 1,
      eYValid   = 2
    };

                Scaler(IndicatorPainter* painter);
    virtual    ~Scaler();

    void        setHighLow(const double maxHigh, const double minLow);
    void        getHighLow(double& maxHigh, double& minLow);

    int         valueToPixel(int i, const double& val, QPoint& pos);
    int         pixelToValue(const QPoint& pos, int& idx, double& value, QDate& date);

    int         topEdge();
    int         intDensity();

    bool        beginYTicking();
    bool        nextYTick(int& y, QString& text);
    void        getValueText(const double& val, QString& text);

    bool        beginYPercentTicking();
    bool        nextYPercentTick(int& y, QString& text);

    void        getErrorText(QStringList& errorMessage);

  protected:
    int         calcToPixel(const double& val); // FIXME: take in these two functions
    double      calcToValue(const int& pixel);  // care of log scaling

    IndicatorPainter* mP;

    int         mXAdjustment;

    double      mIndicatingRange;
    double      mFactor;             // = mChartHeight / mIndicatingRange

    double      mMaxHigh;            // the min/max-extrema of all all values...
    double      mMinLow;             // ...in the currend display range
    double      mValuePerTick;
    double      mTickValue;

    QList<double>  mYTickValue;      // Holds Y-Values for (percent) ticking
    QStringList    mYTickText;       // The corresponding text of mYTickValue
    QList<double>  mYValuePerTick;   // Holds thresholds

    QStringList    mErrorMessage;
};

#endif
