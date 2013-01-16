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

#ifndef DATATUPLE_HPP
#define DATATUPLE_HPP

class QColor;
class QDate;
class DataTupleSet;

/***********************************************************************
*
*   This class holds only one value (well two, value and color) of data.
*   They is used to plot a line and is also the base of each indicator
*   file variable.
*
*   Take a look at http://ta-lib.org/d_api/d_api.html for some info
*
************************************************************************/

class DataTuple
{
  public:
    DataTuple(int Size, const QString& name);
    DataTuple(const QString& name, double* value, DataTupleSet* dts);
    ~DataTuple();

    bool next();
    void rewind();
    int  size();

    // Both returns false if try to read an unvalid value
    bool getValue(double& value);
    bool getColor(QColor& color);

    void setValue(const double& value);
    void setColor(const QString& color);
    void setEvenColor(const QString& color);

    void setRange(int from = -1, int count = -1);
    void getExtrema(double& min, double& max);

    void quitAutonomy(DataTupleSet* dts);

    QString Name;     // Variable name, "open", "high" etc
    QString Label;    // Used for the name on the chart

    double* Value;
    QColor* Color;

    int mNeedsBars;   // The amound of bars needed to calc the result
    int firstValid;   // ta-libs "outBeg"
    int countValid;   // ta-libs "outNbElement"

  protected:
    int MaxIndex;

    double mMaxHigh;  // Holds the min/max values
    double mMinLow;   // of the RangeFrom/RangeTo

    int* Index;
    int* RangeFrom;  // Used for the section shown on the chart
    int* RangeTo;    // Used for the section shown on the chart
    int  mOldRangeFrom;  // Used for check if MaxHigh/Low are still valid
    int  mOldRangeTo;

    bool mIsAutonom;
    bool mMyOwnData;
};

#endif
