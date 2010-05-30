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

    QSqlQuery*  searchFi(const QString& name, const QString& type);

    QSqlQuery*  getGroups(int motherId = -1); // -1 returns all groups
    QSqlQuery*  getGMembers(int groupId);

    void        addToGroup(int group, int fiId);
    void        putGroup(int groupId, const QString& name, int motherId);

    COTuple*    getCOs();
    bool        putCOs(COTuple &);

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

  private:
    void        createTables();
};

#endif
