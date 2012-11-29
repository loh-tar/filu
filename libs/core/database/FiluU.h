//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012  loh.tar@googlemail.com
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
class COTuple;
class RcFile;

class FiluU : public Filu
{
  public:
                FiluU(const QString& connectionName, RcFile* rcFile);
    virtual    ~FiluU();

    enum OrderType
    {
      // Has to fit with order table logic and orderType(...)
      eSellOrder = 0,
      eBuyOrder  = 1
    };

    enum OrderStatus
    {
      // If you change here something take a look at orderStatus(...), Depots, Trader, GetDepotOrders.sql
      eOrderExecuted  =  10,
      eOrderCanceled  =  20,
      eOrderExperied  =  30,
      eOrderAdvice    =  50,
      eOrderActive    = 100,
      eOrderAdvCancel = 110,
      eOrderNeedHelp  = 120
    };

    enum AccPostingType
    {
      // If you change here something take a look at accPostingType(...)
      // FIXME: You are a bookkeeper? Make suggestions!
      ePostCashIn  = 1,
      ePostCashOut = 2,
      ePostFiBuy   = 3,
      ePostFiSell  = 4,
      ePostFee     = 5
    };

    void        openDB();
    void        createFunctions();

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
                       , double limit, bool buy, int marketId, int status, const QString& note, int orderId = 0);

    int         addDepot(const QString& name, const QString& owner, const QString& trader
                       , const QString& broker, int depotId = 0);

    int         addDepotPos(int depotId, const QDate& date, int fiId, int pieces, double price
                          , int marketId, const QString& note, int depotPosId = 0);

    int         addAccPosting(int depotId, const QDate& date, int type
                            , const QString& text, double value, double accPostingId = 0);

    double      getDepotCash(int depotId, const QDate& date = QDate(3000, 01, 01));
    double      getDepotNeededCash(int depotId, const QDate& date = QDate(3000, 01, 01));
    double      getDepotValue(int depotId, const QDate& date = QDate(3000, 01, 01));
    QSqlQuery*  getOrders(int depotId, int status = -1, int fiId = -1); // -1=Any status, All Fi

    int         orderStatus(const QString& type);
    QString     orderStatus(int status);
    int         orderType(const QString& type);
    QString     orderType(int type);
    int         accPostingType(const QString& type);
    QString     accPostingType(int type);

  private:
    void        createTables();
};

#endif
