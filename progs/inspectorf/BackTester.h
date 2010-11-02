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

#ifndef BACKTESTER_HPP
#define BACKTESTER_HPP

#include "FClass.h"
#include "Indicator.h"
#include "Trader.h"

class BackTester : public QThread, public FClass
{
  Q_OBJECT

  enum SomeEnums
  {
    eRule,
    eIndicator
  };

  public:
                    BackTester();
                   ~BackTester();

    void            prepare(const QString& rule,
                            const QString& indicator,
                            const QDate& fdate, const QDate tdate);

    void            setDates(const QDate& fdate, const QDate tdate);
    void            calc();

  signals:
    void            loopsNeedet(int);
    void            loopDone(int);
    void            strategyDone();
    void            error();

  protected slots:

  protected:
    void            init();
    void            run();
    bool            backtest();
    bool            detectConstants();
    void            buildConstants(const QString& constExp, QStringList& constList);
    void            buildFiles();
    void            buildStrategyId();
    void            buildReport(QList<QStringList> &report);

    QMutex          mMutex;
    QWaitCondition  mWaitCondition;

    bool            mRun;
    bool            mNewJob;
    bool            mGoAndDie;

    Trader*         mTrader;
    QString         mOrigRule;
    QString         mOrigIndicator;
    QStringList     mTestRule;
    QStringList     mTestIndicator;
    QString         mStrategyId;        // a checksum for rule+indicator
    int             mTsId;              // the strategy id returned by the DB (primary key)

    QRegExp         mConstMatcher;
    QRegExp         mEditedMatcher;
    QRegExp         mErrorMatcher;

    QList<QStringList> mConst; // hold the constants. one constant per list, each variety per stringlist
    QList<int>      mCPos;     // holds the postion of each constant in the file
    QList<int>      mCType;    // hold where the constant belonged to, 1=rule 2=indicator
    QList<int>      mIdx;      // we need for each constant an own index counter
    QList<bool>     mOneMoreLoop; // and we need for each constant an marker if he reach the end
    int             mLoopsNeeded; // used for progress bar

    QDate           mFromDate;
    QDate           mToDate;
};

#endif

