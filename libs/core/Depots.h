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

#ifndef DEPOTS_HPP
#define DEPOTS_HPP

#include "FClass.h"


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

    bool          exec(const QStringList& command);

  protected:
    struct DepotStatus
    {
      int    id;
      double cash;
      double neededCash;
      double value;
      double availCash;
      double balance;
    };

    QDate         optionDate(const QStringList& parm, const QString& optName);
    QDate         nextCheckday(const QList<int>& checking);

    void          simtrade(const QStringList& parm);
    void          check(const QStringList& parm);
    void          checkDepots(QSqlQuery* depots);
    void          clearOrders(const QStringList& parm);
    void          deleteDepots(const QStringList& parm);
    void          listDepots(const QStringList& parm);
    void          listDepot(const QSqlRecord& depot);
    void          listOrders(const QStringList& parm);
    void          listOrders(const QSqlRecord& depot);
    void          listOrders(QSqlQuery* orders, int status);
    void          printPosition(const QSqlRecord& pos);
    void          printOrder(const QSqlRecord& order);

    QSqlQuery*    getDepots(const QStringList& parm);
    void          printDepotHeader(const QSqlRecord& depot);
    QString       isin(int fiId);

    DepotStatus   mDP;
    int           mLineNo;
    QDate         mToday;
    QDate         mLastCheck;

  private:
};

#endif
