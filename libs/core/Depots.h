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

#ifndef DEPOTS_HPP
#define DEPOTS_HPP

#include "FClass.h"

class CmdHelper;


/***********************************************************************
*
*   This class is like a wrapper around Trader
*
************************************************************************/

class Depots : public FClass
{
  public:
                  Depots(FClass* parent);
    virtual      ~Depots();

    bool          exec(CmdHelper* cmd);
    static void   briefIn(CmdHelper* cmd);

  protected:
    typedef QHash<QString, QString>   HashStrStr;

    struct DepotStatus
    {
      int    id;
      double cash;
      double neededCash;
      double value;
      double availCash;
      double balance;
    };

    QDate         nextCheckday(const QList<int>& checking);

    void          simtrade();
    void          cancelOrder();
    void          changeOrder();
    void          check();
    void          checkDepots(QSqlQuery* depots);
    void          clearOrders();
    void          deleteDepots();
    void          listDepots();
    void          listDepot(const QSqlRecord& depot);
    void          listOrders();
    void          listOrders(const QSqlRecord& depot);
    void          listOrders(QSqlQuery* orders, int status);
    void          printPosition(const QSqlRecord& pos);
    void          printOrder(const QSqlRecord& order);

    bool          getOrder(int id, QSqlRecord& order);
    QSqlQuery*    getDepots(const QString& owner);
    QSqlQuery*    getDepots(int id);
    QSqlQuery*    getDepots();
    QSqlQuery*    getDepots2();
    void          printDepotHeader(const QSqlRecord& depot);
    QString       isin(int fiId);

    CmdHelper*    mCmd;          // Never delete, not your own
    DepotStatus   mDP;
    int           mLineNo;
    QDate         mToday;
    QDate         mLastCheck;
    HashStrStr    mOptions;

  private:
};

#endif
