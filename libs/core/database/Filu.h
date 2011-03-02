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

#ifndef FIMI_HPP
#define FIMI_HPP

#include <QtCore>
#include <QtSql>

#include "Newswire.h"

// Here "#include" and not "class" all stuff interested by Filu.
// So all classes which use Filu will knows the data types.
#include "BarTuple.h"
#include "SymbolTuple.h"
#include "MarketTuple.h"
#include "FiTuple.h"
#include "SymbolTypeTuple.h"
#include "BrokerTuple.h"

class RcFile;

typedef QHash<QString, QDate> DateRange;
typedef QSet<QString> StringSet;
typedef QHash<QString, QString> KeyVal;

class Filu : public Newswire
{
  public:
         Filu(const QString&, RcFile*);
        ~Filu();

    enum SomeEnums
    {
      // Return values for functions returning an intenger, mostly a data Id
      // Do *not* use it for functions returning fooTuple
      // FIXME: Did you know how to solve this with usual && || operators?
      eData        =      1,  // Use it like if(retVal >= eData) ...
      eSuccess     =      1,  // Use it like if(retVal == eSuccess) ...
      eNotFound    =      0,  // Use it like if(id == eNotFound) ...
      eNoSuccess   =      0,  // Use it like if(retVal == eNoSuccess) ...
      eNoData      =      0,  // Use it like if(id == eNoData) ...
      eError       =     -1,  // Use it like if(id <= eError ) ...
      eNotUnique   = -10000,  // Use it like if(id == eNotUnique) ...
      eInitError   = -10001,
      eExecError   = -10002,

      // Data Quality Definitions
      ePlatinum    =  0, // Modified by the user, don't change automatically
      eGold        =  1, // As final classified data by script
      eBronze      =  2, // As temporary classified data by script
      eTin         =  3, // Means there was an logical error in the data
                         // detected by the database

      // Filter for getSymbolTypes(...)
      eAllTypes        = 1,
      eOnlyProvider    = 2,
      eOnlyNonProvider = 3
    };

    virtual void openDB();
            void closeDB();

    // Set Functions
    int  setMarketName(const QString&);    // Returns marketId or < eError
    int  getFiIdBySymbol(const QString&);  // Returns FiId to symbol or < eError
    void setBarsToLoad(int);
    void setDaysToFetchIfNoData(int);      // Unused

    void printSettings();

    // Get Tuple Functions
    BarTuple*    getBars(const QString& symbol, const QString& market
                       , const QString& fromDate = "1000-01-01"
                       , const QString& toDate   = "3000-01-01");

    BarTuple*    getBars(int fiId, int marketId
                       , const QString& fromDate = "1000-01-01"
                       , const QString& toDate   = "3000-01-01");

    BarTuple*    getBars(const QString& symbol, const QString& market, int limit);
    BarTuple*    getBars(int fiId, int marketId, int limit);

    MarketTuple* getMarkets(const QString& name = "");
    MarketTuple* getMarket(int marketId);

    FiTuple*     getFi(int fiId);
    FiTuple*     getFiLike(const QString& pattern);
    FiTuple*     getFiBySymbol(const QString& symbol);

    SymbolTuple* getSymbols(int fiId, const QString& fiType
                            , const QString& symbolType
                            , const QString& symbol
                            , const QString& market
                            , bool onlyProviderSymbols);  // getSymbol stuff needs rethinking

    SymbolTuple* getSymbols(int fiId);
    SymbolTuple* getAllProviderSymbols();

    SymbolTuple* searchSymbol(const QString& symbol
                            , const QString& market = ""
                            , const QString& owner  = "");

    SymbolTypeTuple* getSymbolTypes(int filter = eAllTypes/* FIXME, bool orderBySeq = true*/);

    int          getFiTypes(QStringList& type);

    int          getEODBarDateRange(DateRange& dateRange
                                  , int fiId, int marketId, int quality);

    BrokerTuple* getBroker(int brokerId = 0);

    // Add Functions
    int          addFiType(const QString& type, int id = 0);

    int          addSymbolType(const QString& type
                             , const int& seq
                             , const bool isProvider
                             , const int& id = 0);

    int          addMarket(const QString& market
                         , const QString& currency
                         , const QString& currSymbol);

    int          addEODBarData(int fiId, int marketId, const QStringList* data);

    int          addFiCareful(FiTuple& fi);
    int          addFi(FiTuple& fi);

    int          addFi(const QString& name
                     , const QString& type
                     , const int fiId);

    int          addFi(const QString& name
                     , const QString& type
                     , const QString& symbol
                     , const QString& market
                     , const QString& stype
                     , const int fiId = 0);

    int          addSymbol(const QString& symbol
                         , const QString& market
                         , const QString& stype
                         , const int fiId = 0
                         , const int symbolId = 0);

    int          addUnderlying(const QString& mother
                             , const QString& symbol
                             , const double weight);

    int          addSplit(const QString& symbol
                        , const QString& date
                        , const double ratio
                        , const QString& comment
                        , const int& quality = 2);

    int          addBroker(BrokerTuple& broker);

    // The Big Beef, Indicator Functions
    // These are *not* usual indicators. That's indicators provided by Filu,
    // the ultimate cause of starting the FiMi project by Christian Kindler.
    int         getIndicatorNames(QStringList* names, const QString& like = "");
    int         getIndicatorInfo(KeyVal* info, const QString& name);
    int         prepareIndicator(const QString& name, const QString& call = "");
    QSqlQuery*  callIndicator(const QString& name);

    // Non Of The Above Stuff
    // The 2nd and most filexible way to access data
    QSqlQuery*  execSql(const QString& name);
    void        setSqlParm(const QString& parm, const QVariant& value);

    int         quality(const QString& quality);
    QString     quality(int quality);
    int         convertCurrency(double& money, int sCurrId, int dCurrId, const QDate& date);
    int         searchCaption(const QString& table, const QString& caption);
    QString     getLastQuery(); // Unused
    void        deleteRecord(const QString& schema, const QString& table, int id);
    int         updateField(const QString& field, const QVariant& newValue
                          , const QString& schema, const QString& table, int id);

    int         result(const QString& func, QSqlQuery* query);
    int         lastResult() { return mLastResult; };
    QString     dbFuncErrText(int errorCode);

  protected:
    int         getNextId(const QString& schema, const QString& table);
    bool        initQuery(const QString& name);
    bool        readSqlStatement(const QString& name, QString& sqlStatement);
    int         execute(QSqlQuery* query);

    RcFile*     mRcFile;
    QString     mSqlPath;
    QString     mUserSchema;
    QString     mConnectionName;
    QString     mFromDate;
    QString     mToDate;
    int         mLimit;  // To limit the count of rows at SELECT xyz FROM abc...
    bool        mOnlyProviderSymbols;
    QString     mLastError;
    QString     mLastQuery;
    int         mLastResult; // ErrorNo, eNoData, eData or Id

    QHash<QString, QSqlQuery*>  mSQLs;
    QHash<QString, QVariant>    mSqlParm;      // Holds all values of ':foo' sql parameters
                                               // to all SQLs
    QHash<QString, StringSet>   mSqlParmNames; // Holds all parameter names to a sql

  private:
    BarTuple*     fillQuoteTuple(QSqlQuery*);
    FiTuple*      fillFiTuple(QSqlQuery*);
    MarketTuple*  fillMarketTuple(QSqlQuery*);

    // Some Filu Settings
    void        readSettings();
    QString     mFiluSchema;
    int         mCommitBlockSize;
    int         mDaysToFetchIfNoData;
};

#endif
