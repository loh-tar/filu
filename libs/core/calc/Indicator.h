//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011  loh.tar@googlemail.com
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

#ifndef INDICATOR_HPP
#define INDICATOR_HPP

#include "FClass.h"

class DataTupleSet;
class CalcType;
class TALib;

/***********************************************************************
*
*   Yes, here is the beef! He calculate the data...well, not really.
*   The calculation itself will done by CalcType-Objects
*
*   He is not only used for painting, scanners are also powered by him
*
************************************************************************/

class Indicator : public FClass
{
  public:
                  Indicator(FClass* parent);
    virtual      ~Indicator();

    QStringList*  useFile(const QString& file);
    QString       viewName();
    QString       fileName();

    bool          useIndicator(const QStringList& indicator);
    void          getIndicator(QStringList& indicator);


    DataTupleSet* calculate(BarTuple* bars);
    DataTupleSet* calculate(DataTupleSet* data);

    void          ignorePlot(bool ignore = true);
    int           barsNeeded();
    void          getVariableNames(QSet<QString>* list);
    int           scanFreq(bool trueDays = false);
    bool          hasScan4();

    friend class  CalcType;
    friend class  TALib;

  protected:
    bool          readIndicator(const QString& fileName, QStringList& indicator);
    bool          parse(QStringList& indicator);
    bool          prepare(QStringList& indicator);
    void          readSettings();

    TALib*               mTALib;
    DataTupleSet*        mData;              // Holds the beef, all calculated values
    QHash<QString, int>  mFiIds;             // <fi-alias>,<fiId>
    QStringList          mIndicator;         // Holds the whole (rehashed) indicator file
    QStringList          mOrigIndicator;     // Holds the file as it is
    QList<CalcType*>     mCalcCommands;      // Rehashed plot statements, now objects
    QString              mViewName;
    QString              mIndicatorFileName; // The currend used indicator
    QString              mIndicatorPath;
    QString              mScanFreq;

    bool                 mIgnorePlot;
    bool                 mParseError;

    QSet<QString>*       mAlreadyIncluded;
    QSet<QString>*       mUsedVariables;
    QSet<QString>*       mBuildInVariables;
};

#endif


