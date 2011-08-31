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

#ifndef TRADER_HPP
#define TRADER_HPP

#include "FClass.h"

class Indicator;
class DataTupleSet;
class MyParser;

/***********************************************************************
*
*   The Trader produce as output a list with buy/sell orders.
*   To do the job he use an normal indicator file (with suitable commands
*   of course) and a trading rule file.
*
*   He is used by CalcTrade.cpp to illustrade the result of the trading
*   idea, the portfolio manager BookieF and the back tester InspectorF.
*
*   See also in your source directory
*     doc/indicator-file-format.txt
*     doc/trading-rule-file-format.txt
*
************************************************************************/

class Trader : public FClass
{
  public:

    enum SomeDefines
    {
      eNextBlock = true
    };

                Trader(FClass* parent);
    virtual    ~Trader();

    typedef     QPair<MyParser*, QList<QStringList> > Rule;

    bool        useRuleFile(const QString& fileName);
    bool        useRule(const QStringList& rule);                    // BackTester only
    void        getRule(QStringList& rule);                          // InspectorF only
    bool        useIndicator(const QStringList& indicator);          // BackTester only
    void        getIndicator(QStringList& indicator);                // InspectorF only

    // For real Trading
    bool        prepare(const QSqlRecord& depot, const QDate& lastCheck, const QDate& today);
    QDate       needBarsFrom();
    QStringList workOnGroups();
    bool        check(BarTuple* bars, const QDate& lastCheck);
    void        postExecutedOrder(const QSqlRecord& order, const QDate& execDate, double execPrice);

    bool        simulate(DataTupleSet* data);
    int         prepare(const QDate& fromDate, const QDate& toDate); // BackTester only
    int         simulateNext();                                      // BackTester only
    void        getOrders(QList<QStringList>& orders);
    void        getReport(QList<QStringList>& report);
    void        getVariablesList(QSet<QString>* list);

  protected:
    bool        parseRule();
    bool        nextLine(bool nextBlock = false);
    void        readSettings();
    void        readRules();
    void        appendMData(); // Add all output variables to mData
    bool        setFeeFormula(const QString& exp);
    double      calcFee(double& volume);
    bool        initVariables();
    double      inFiCurrency(double money);
    double      inDepotCurrency(double money);
    void        takeActions(const QList<QStringList>& actions);
    void        actionBuy(const QStringList& action);
    void        actionSell(const QStringList& action);
    void        checkOpenOrders();
    void        checkOpenBuyOrder(QStringList& order);
    void        checkOpenSellOrder(QStringList& order);
    void        calcGain();
    inline void setTo(const QString& name, double v);
    inline void addTo(const QString& name, double v);
    void        appendToMData(const QString& name);
    void        appendReport(const QString& txt, const double v, const QString& vname, const int precision = 2);

    QString     mLine;
    QString     mOrigLine;          // For error messages
    int         mLineNumber;        // For error messages
    QStringList mOrigRule;          // Holds the readed rule file as it is

    QHash<QString, QString>      mSettings;
    QHash<QString, double>       mVariable;
    QHash<QString, double>       mRealVar;    // Holds the real status to be overwrite mVariable defaults at real trading
    QStringList                  mDataAdded;  // Holds variable names needs synced between mData and mVariable

    QList<QStringList> mOpenOrders; // As the name implies, intern used
    QList<QStringList> mOrders;     // For export/real trading. includes a date and "human readable" limit
    QList<QStringList> mReport;     // Like mOrders, but includes additional info about execution of orders
                                    // and a statistic report

    QList<Rule>    mRules;
    MyParser*      mFeeCalc;

    Indicator*     mIndicator;
    int            mBarsNeeded;
    DataTupleSet*  mData;
    QSqlQuery*     mFi;
    QDate          mFromDate;
    QDate          mToDate;
    QDate          mToday;          // Hold the current (or simulated) date

    bool           mAutoLoadIndicator;
    bool           mOkSettings;
    bool           mOkRules;
    QString        mTradingRulePath;

    int            mDepotId;
    QSet<int>      mInStock;        // Holds FiIds of FIs in depot at real trading to prevent redundant checks
};

#endif
