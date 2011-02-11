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

#ifndef FIMIU_HPP
#define FIMIU_HPP

#include "Filu.h"

// #include here and not class all stuff interested by FiluU.
// So all classes which use FiluU will knows the data types
#include "COTuple.h"

class RcFile;

class FiluU : public Filu
{
  public:
                FiluU(const QString& connectionName, RcFile* rcFile);
    virtual    ~FiluU();

    void        openDB();

    QSqlQuery*  searchFi(const QString& name, const QString& type);

    QSqlQuery*  getGroups(int motherId = -1); // -1 returns all groups
    QSqlQuery*  getGMembers(int groupId);
    int         getGroupId(const QString& path);
    void        addToGroup(int group, int fiId);
    int         addGroup(const QString& path);  // Returns GroupId or error
    void        putGroup(int groupId, const QString& name, int motherId);

    COTuple*    getCOs(int fiId, int marketId
                     , const QString& fromDate = "1000-01-01"
                     , const QString& toDate   = "3000-01-01");

    bool        putCOs(COTuple&);

    void        putBacktest(int& backtestId, const QDate& fromDate, const QDate& toDate
                          , double gain, const QString rule, const QString indicator
                          , const QString testName);

    QSqlQuery*  getBacktest();

    void        putBTLog(int backtestId, int fiId, int marketId, const QDate& date
                       , const QString& event, int amount, double money);

    QSqlQuery*  getBTLog(int backtestId, int fiId, int marketId);

    int         addTradingStrategy(const QString sId,
                                   const QString fromDate,
                                   const QString toDate,
                                   const QString rule,
                                   const QString indicator);

    bool        addTradingResult(int tsId, int fiId, int marketId,
                                 double wltp, double lltp, double agwlp,
                                 double alltp, double tpp, double score);

    QSqlQuery*  getBTDetails(const QString& strategy);

    int         addOrder(int depotId, const QDate& oDate, const QDate& vDate, int fiId, int pieces
                       , double limit, bool buy, int marketId, int status, int orderId = 0);

    int         addDepotPos(int depotId, const QDate& date
                          , int fiId, int pieces, double price, int marketId, int depotPosId = 0);

    int         addAccountPos(int depotId, const QDate& date, int type
                            , const QString& text, double value, double accountPosId = 0);

    double      getDepotCash(int depotId, const QDate& date = QDate(3000, 01, 01));
    double      getDepotNeededCash(int depotId, const QDate& date = QDate(3000, 01, 01));
    double      getDepotValue(int depotId, const QDate& date = QDate(3000, 01, 01));
    QSqlQuery*  getOrders(int depotId, int status = 5, int fiId = -1); // 5=aktive, -1=all Fi

  private:
    void        createTables();
};

#endif
