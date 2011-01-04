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

#ifndef FIMI_HPP
#define FIMI_HPP

#include <QtCore>
#include <QtSql>

// Here "#include" and not "class" all stuff interested by Filu.
// So all classes which use Filu will knows the data types.
#include "BarTuple.h"
#include "SymbolTuple.h"
#include "MarketTuple.h"
#include "FiTuple.h"
#include "SymbolTypeTuple.h"

class RcFile;

typedef QHash<QString, QDate> DateRange;
typedef QSet<QString> StringSet;
typedef QHash<QString, QString> KeyVal;

class Filu
{
  public:
         Filu(const QString&, RcFile*);
        ~Filu();

    enum SomeEnums
    {
      // Error Message Quality
      eNotice,
      eWarning,
      eCritical,

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
    void setFiName(const QString& name);   // Deprecated, only at FiPage.cpp:129
    void setFiType(const QString& type);   // Deprecated, only like mFilu->setFiType("");
    void setFiId(int);
    bool setIdsByNameSettings();           // Deprecated, I hate it, only used intern
    void setMarketId(int);
    int  setMarketName(const QString&);    // Returns marketId or < eError
    int  setSymbolCaption(const QString&); // Returns FiId to symbol or < eError
    void setProviderId(int);               // Unusued
    void setProviderName(const QString&);  // Deprecated, only like mFilu->setProviderName("");
    void setOnlyProviderSymbols(bool);     // Deprecated, make extra function getProviderSymbols
    void setFromDate(const QString& = "1000-01-01");  // Deprecated
    void setToDate(const QString& = "3000-01-01");    // Deprecated
    void setBarsToLoad(int);
    void setDaysToFetchIfNoData(int);      // Unused
    void printSettings();

    // Get Tuple Functions
    BarTuple*    getBars(const QString& symbol, const QString& market
                       , const QString& fromDate = "1000-01-01"
                       , const QString& toDate = "3000-01-01");

    BarTuple*    getBars(int fiId, int marketId
                       , const QString& fromDate = "1000-01-01"
                       , const QString& toDate = "3000-01-01");

    BarTuple*    getBars(const QString& symbol, const QString& market, int limit);
    BarTuple*    getBars(int fiId, int marketId, int limit);
    BarTuple*    getBars(); // Deprecated

    MarketTuple* getMarket();

    FiTuple*     getFi(const int fiId);
    FiTuple*     getFi(const bool fuzzy = false);  // Rename to getFiLike(const QString& foo)
    FiTuple*     getFi(const QString& symbol);     // Rename to getFiBySymbol(...)

    SymbolTuple* getSymbols();          // getSymbol stuff needs rethinking
    SymbolTuple* getSymbols(int fiId);
    SymbolTuple* getAllProviderSymbols();

    SymbolTuple* searchSymbol(const QString& symbol
                            , const QString& market = ""
                            , const QString& owner  = "");

    SymbolTypeTuple* getSymbolTypes(int filter = eAllTypes/* FIXME, bool orderBySeq = true*/);

    int          getFiType(QStringList& type);

    int          getEODBarDateRange(DateRange& dateRange
                                  , int fiId, int marketId, int quality);

    // Add Functions
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

    int           addSplit(const QString& symbol
                        , const QString& date
                        , const double ratio
                        , const QString& comment
                        , const int& quality = 2);

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

    int         searchCaption(const QString& table, const QString& caption);

    bool        hadTrouble();   // Calling this clears error flag
    QString     errorText();    // Calling this clears error text
    QString     getLastQuery(); // Unused

    void        deleteRecord(const QString& schema, const QString& table, int id);

  protected:
    int         getNextId(const QString& schema, const QString& table);
    bool        initQuery(const QString& name);
    bool        readSqlStatement(const QString& name, QString& sqlStatement);
    void        addErrorText(const QString& errorText, int type = eNotice);
    int         execute(QSqlQuery* query);

    RcFile*     mRcFile;
    QString     mSettingsFile;
    QString     mSqlPath;
    QString     mUserSchema;
    QString     mConnectionName;
    int         mFiId;
    QString     mFiType;
    QString     mFiLongName;
    QString     mSymbolCaption;
    int         mMarketId;
    QString     mMarketName;
    int         mProviderId;
    QString     mProviderName;
    QString     mFromDate;
    QString     mToDate;
    int         mLimit;  // To limit the count of rows at SELECT xyz FROM abc...
    bool        mOnlyProviderSymbols;
    int         mSqlDebugLevel;
    QString     mLastError;
    QString     mLastQuery;
    bool        mHasError;
    QStringList mErrorText;
    QHash<QString, QSqlQuery*>  mSQLs;
    QHash<QString, QVariant>    mSqlParm;      // Holds all values of ':foo' sql parameters
                                               // to all SQLs
    QHash<QString, StringSet>   mSqlParmNames; // Holds all parameter names to a sql

  private:
    BarTuple*   fillQuoteTuple(QSqlQuery*);
    FiTuple*    fillFiTuple(QSqlQuery*);

    // Some Filu Settings
    void        readSettings();
    QString     mFiluSchema;
    int         mCommitBlockSize;
    int         mDaysToFetchIfNoData;
};

#endif
