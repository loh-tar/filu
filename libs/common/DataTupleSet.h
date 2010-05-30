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

#ifndef DATATUPLESET_HPP
#define DATATUPLESET_HPP

#include <QString>
#include <QColor>
#include <QHash>
#include "BarTuple.h"
#include "DataTuple.h"


/***********************************************************************
*
* As the name implies hold this class a group/set of DataTuples.
* This class is the output/result of an Indicator calculation and is used
* by IndicatorPainter to paint something
*
************************************************************************/

class DataTuple;

class DataTupleSet
{
  public:
            DataTupleSet();
           ~DataTupleSet();

    bool    next();
    void    rewind(int start = -1);
    int     findDate(const QDate& date);

    bool    append(const QString& key);
    bool    append(DataTuple* dt);
    bool    appendBarTuple(BarTuple* bars, const QString& fiName = "THIS");
    bool    getValue(const QString& name, double& value);
    bool    setValue(const QString& name, double value);
    bool    getColor(const QString& name, QColor& color);
    void    setColor(const QString& name, const QString& color);
    void    setEvenColor(const QString& name, const QString& color);
    void    getDate(QDate& date);
    void    getTime(QTime& time);
    void    getDateRange(QDate& fromDate, QDate& toDate);

    double* valueArray(const QString& name);
    void    setValidRange(const QString& name, int first, int count);
    void    getValidRange(const QString& name, int& first, int& count);

    void    setRange(int from = -1, int count = -1);
    void    getExtrema(const QString& name, double& min, double& max);

    int     getVariableNames(QStringList& list);
    int     dataTupleSize();

    void    setNeededBars(const QString& name, int value);
    int     neededBars(const QString& name);

    bool    getIDs(const QString& alias, int& fiId, int& marketId);

    friend class DataTuple;

  protected:
    QHash<QString, DataTuple*> DataSet;
    QHash<QString, int>        mFiIds;
    QHash<QString, int>        mMarketIds;

    int  Index;
    int  MaxIndex;
    int  RangeFrom;        // used for the section shown on the chart
    int  RangeTo;

    BarTuple* mThisBars;

};

#endif
