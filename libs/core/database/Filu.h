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

#ifndef FIMI_HPP
#define FIMI_HPP

#include <QDate>
#include <QSqlDatabase>
class QSqlQuery;
class QSqlRecord;

#include "Newswire.h"
class BarTuple;
class BrokerTuple;
class FiTuple;
class MarketTuple;
class RcFile;
class SymbolTuple;
class SymbolTypeTuple;

typedef QHash<QString, QDate>     DateRange;
typedef QSet<QString>             StringSet;
typedef QHash<QString, QString>   KeyVal;

class Filu : public Newswire
{
  public:
                Filu(const QString&, RcFile*);
               ~Filu();

    static const bool  eHideNoMarket = true;
    static const bool  eWithNoMarket = false;

    enum Schema
    {
      eNotValid        = -1,
      eFilu            =  1,
      eUser            =  2
    };

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
    void         closeDB();

    bool transaction() { return mFiluDB.transaction(); };
    bool commit()      { return mFiluDB.commit(); };
    bool rollback()    { return mFiluDB.rollback(); };

    // Set Functions
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

    BarTuple*    getBars(const QString& symbol, const QString& market
                       , int limit, const QString& toDate = "3000-01-01");

    BarTuple*    getBars(int fiId, int marketId
                       , int limit, const QString& toDate = "3000-01-01");

    MarketTuple* getMarkets(const QString& name = "");
    MarketTuple* getMarkets(int marketId, const QString& name = "");
    int          getMarketId(const QString& name);        // Returns marketId or < eError

    FiTuple*     getFi(int fiId);
    FiTuple*     getFiLike(const QString& pattern);
    FiTuple*     getFiBySymbol(const QString& symbol);
    int          getFiIdBySymbol(const QString& symbol);  // Returns FiId to symbol or < eError

    SymbolTuple* getSymbols(int fiId);
    SymbolTuple* getSymbol(const QString& symbol
                         , const QString& market
                         , const QString& owner  );

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
    int          addBroker(const QString& name
                         , const QString& currSymbol
                         , const QString& feeFormula
                         , const int quality
                         , const int id = 0);

    // The Big Beef, Indicator Functions
    // These are *not* usual indicators. That's indicators provided by Filu,
    // the ultimate cause of starting the FiMi project by Christian Kindler.
    int         getIndicatorNames(QStringList* names, const QString& like = "");
    int         getIndicatorInfo(KeyVal* info, const QString& name);
    int         prepareIndicator(const QString& name, const QString& call = "");
    QSqlQuery*  callIndicator(const QString& name);

    // Non Of The Above Stuff
    // The 2nd and most filexible way to access data, but a bad style.
    // Should only be used in rare cases or while development.
    QSqlQuery*  execSql(const QString& name);
    void        setSqlParm(const QString& parm, const QVariant& value);
    void        setStaticSqlParm(const QString& parm, const QString& value);

    int         quality(const QString& quality);
    QString     quality(int quality);
    int         convertCurrency(double& money, int sCurrId, int dCurrId, const QDate& date);
    QSqlQuery*  searchFi(const QString& name, const QString& type, bool hideNoMarket = eHideNoMarket);

    bool        hasTable(const QString& table, const Schema type = eFilu);
    bool        hasTableColumn(const QString& column, const QString& table, const Schema type = eFilu);
    QSqlQuery*  searchRows(const QString& table, const QStringList& fieldValueLst, const Schema type = eFilu);
    int         searchCaption(const QString& table, const QString& caption, const Schema type = eFilu);
    void        deleteRecord(const QString& table, int id, const Schema type = eFilu);
    int         updateField(const QString& field, const QVariant& newValue
                          , const QString& table, int id, const Schema type = eFilu);

    QString     schema(const Schema type);
    Schema      schema(const QString& type);
    QSqlQuery*  lastQuery();
    int         result(const QString& func, QSqlQuery* query);
    int         lastResult() { return mLastResult; };
    QString     dbFuncErrText(int errorCode);
    QStringList getTables(const Schema type = eFilu);
    QStringList getTableColumns(const QString& table, const Schema type = eFilu);
    QString     serverVersion();
    QString     devilInfoText();

    void        createFunctions();
    void        createViews();
    void        createFunc(const QString& sql) { executeSql("filu/functions/", sql); }
    void        createView(const QString& sql) { executeSql("filu/views/", sql); }
    void        createMisc(const QString& sql) { executeSql("filu/misc/", sql); }

  protected:
    void        executeSql(const QString& path, const QString& sql);
    bool        executeSqls(const QString& path);
    void        createSchema();
    void        createTables();

    int         getNextId(const QString& table, const Schema type = eFilu);
    bool        initQuery(const QString& name);
    bool        initQuery(const QString& name, const QString& rawSql);
    QString     parseSql(const QString& name, const QString& rawSql);
    bool        loadQuery(const QString& name, QString& sql);
    int         execute(const QString& name, const QString& rawSql);
    int         execute(QSqlQuery* query);

    RcFile*     mRcFile;
    QString     mSqlPath;
    QString     mConnectionName;
    QString     mFromDate;
    QString     mToDate;
    int         mLimit;  // To limit the count of rows at SELECT xyz FROM abc...
    bool        mOnlyProviderSymbols;
    QSqlQuery*  mLastQuery;
    QString     mLastError;
    QString     mExecSql;
    int         mLastResult; // ErrorNo, eNoData, eData or Id

    QSqlDatabase                mFiluDB;
    QHash<QString, QSqlQuery*>  mSQLs;
    QHash<QString, QVariant>    mSqlParm;       // Holds values of ':foo' sql parameters to all SQLs
    QHash<QString, QString>     mSqlStaticParms;// Holds static values of ':foo' sql parameters
    QHash<QString, StringSet>   mSqlParmNames;  // Holds all parameter names to a sql

  private:
    BarTuple*     fillQuoteTuple(QSqlQuery*);
    FiTuple*      fillFiTuple(QSqlQuery*);
    MarketTuple*  fillMarketTuple(QSqlQuery*);
    SymbolTuple*  fillSymbolTuple(QSqlQuery*);

    // Some Filu Settings
    void        readSettings();
    int         mCommitBlockSize;
    int         mDaysToFetchIfNoData;
};

#endif
