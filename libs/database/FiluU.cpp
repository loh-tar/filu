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

#include "FiluU.h"

#include "RcFile.h"

FiluU::FiluU(const QString& connectionName, RcFile* rcFile)
     : Filu(connectionName, rcFile)
{
  mUserSchema = "user_" + qgetenv("USER");

  QString sql("select nspname from pg_namespace where nspname = ':user'");
  sql.replace(":user", mUserSchema);

  QSqlQuery query(QSqlDatabase::database(connectionName));
  query.prepare(sql);
  execute(&query);
  if(query.size() == 0) createTables();
}

FiluU::~FiluU()
{}

QSqlQuery* FiluU::searchFi(const QString& name, const QString& type)
{
  if(!initQuery("SearchFi")) return 0;

  QSqlQuery* query = mSQLs.value("SearchFi");

  query->bindValue(":name", name);
  query->bindValue(":type", type);
  int result = execute(query);

  if(result <= eError) return 0;

  return query;
}

QSqlQuery* FiluU::getGroups(int motherId /*= -1*/)
{
  if(!initQuery("GetGroups")) return 0;

  QSqlQuery* query = mSQLs.value("GetGroups");

  query->bindValue(":motherId", motherId);
  int result = execute(query);

  if(result >= eData) return query;

  return 0;
}

QSqlQuery* FiluU::getGMembers(int groupId)
{
  if(!initQuery("GetGMembers")) return 0;

  QSqlQuery* query = mSQLs.value("GetGMembers");

  query->bindValue(":groupId", groupId);
  int result = execute(query);

  if(result <= eError) return 0;

  return query;
}

void FiluU::addToGroup(int groupId, int fiId)
{
  if(!initQuery("AddGMember")) return;

  QSqlQuery* query = mSQLs.value("AddGMember");

  query->bindValue(":groupId", groupId);
  query->bindValue(":fiId", fiId);
  /*int result = */execute(query);
}

void FiluU::putGroup(int groupId, const QString& name, int motherId)
{
  if(!initQuery("PutGroup")) return;

  QSqlQuery* query = mSQLs.value("PutGroup");

  query->bindValue(":groupId", groupId);
  query->bindValue(":name", name);
  query->bindValue(":motherId", motherId);
  /*int result = */execute(query);
}

COTuple* FiluU::getCOs()
{
  if(!initQuery("GetCOs")) return 0;

  QSqlQuery* query = mSQLs.value("GetCOs");

  query->bindValue(":fiId", mFiId);
  query->bindValue(":fromDate", mFromDate);
  query->bindValue(":toDate", mToDate);
  int result = execute(query);
  if(result < Filu::eData) return 0;

  COTuple* co = new COTuple(query->size());
  while(co->next())
  {
    query->next();
    int i = co->Index;
    co->mId[i]         = query->value(0).toInt();
    co->mFiId[i]       = query->value(1).toInt();
    co->mMarketId[i]   = query->value(2).toInt();
    co->mViewName[i]   = query->value(3).toString();
    co->mAnchorDate[i] = query->value(4).toDate();
    co->mType[i]       = query->value(5).toString();
    co->mAttributes[i] = query->value(6).toString();
  }
  co->rewind();
  return co;
}

bool FiluU::putCOs(COTuple& co)
{
  if(!initQuery("PutCOs")) return false;

  QSqlQuery* query = mSQLs.value("PutCOs");

  QSqlDatabase::database(mConnectionName).transaction();
  co.rewind();
  while(co.next())
  {
    if(co.id() < 1)
    {
      co.setId(getNextId(mUserSchema, "co"));
    }

    query->bindValue(":id", co.id());
    query->bindValue(":fiId", co.fiId());
    query->bindValue(":marketId", co.marketId());
    query->bindValue(":plot", co.viewName());
    query->bindValue(":date", co.anchorDate());
    query->bindValue(":type", co.type());
    query->bindValue(":parameters", co.attributes());
    int result = execute(query);
    if(result <= eError)
    {
      qDebug() << "FiluU::putCO: fail";
      QSqlDatabase::database(mConnectionName).rollback();
      return false;
    }
  }

  QSqlDatabase::database(mConnectionName).commit();
  return true;
}

void FiluU::putBacktest(int& backtestId, const QDate& fromDate, const QDate& toDate
                        , double gain, const QString rule, const QString indicator
                        , const QString testName)
{
  if(!initQuery("PutBacktest")) return;

  QSqlQuery* query = mSQLs.value("PutBacktest");

  if(!backtestId) backtestId = getNextId(":user", "backtest");

  query->bindValue(":backtestId", backtestId);
  query->bindValue(":fromDate", fromDate);
  query->bindValue(":toDate", toDate);
  query->bindValue(":gain", gain);
  query->bindValue(":rule", rule);
  query->bindValue(":indicator", indicator);
  query->bindValue(":testName", testName);
  /*int result = */execute(query);
}

QSqlQuery* FiluU::getBacktest()
{
  if(!initQuery("GetBacktest")) return 0;

  QSqlQuery* query = mSQLs.value("GetBacktest");

  //query->bindValue(":", );

  int result = execute(query);

  if(result <= eData) return 0;

  return query;
}

void  FiluU::putBTLog(int backtestId, int fiId, int marketId, const QDate& date
                      , const QString& event, int amount, double money)
{
  if(!initQuery("PutBTLog")) return;

  QSqlQuery* query = mSQLs.value("PutBTLog");

  query->bindValue(":backtestId", backtestId);
  query->bindValue(":fiId", fiId);
  query->bindValue(":marketId", marketId);
  query->bindValue(":date", date);
  query->bindValue(":event", event);
  query->bindValue(":amount", amount);
  query->bindValue(":money", money);

  /*int result = */execute(query);
}

QSqlQuery * FiluU::getBTLog(int backtestId, int fiId, int marketId)
{
  if(!initQuery("GetBTLog")) return 0;

  QSqlQuery* query = mSQLs.value("GetBTLog");

  query->bindValue(":backtestId", backtestId);
  query->bindValue(":fiId", fiId);
  query->bindValue(":marketId", marketId);

  int result = execute(query);

  if(result < eData) return 0;

  return query;
}

int  FiluU::addTradingStrategy( const QString sId,
                                const QString fromDate,
                                const QString toDate,
                                const QString rule,
                                const QString indicator )
{
  if(!initQuery("AddTradingStrategy")) return 0;

  QSqlQuery* query = mSQLs.value("AddTradingStrategy");

  int tsId = getNextId(":user", "ts");
  if(tsId == 0) return 0;

  query->bindValue(":tsId", tsId);
  query->bindValue(":sId", sId);
  query->bindValue(":fromDate", fromDate);
  query->bindValue(":toDate", toDate);
  query->bindValue(":rule", rule);
  query->bindValue(":indicator", indicator);

  int result = execute(query);

  if(result < eSuccess) return 0;

  return tsId;
}

bool FiluU::addTradingResult( int tsId, int fiId, int marketId,
                              double wltp, double lltp, double agwlp,
                              double alltp, double tpp, double score )
{
  if(!initQuery("AddTradingResult")) return false;

  QSqlQuery* query = mSQLs.value("AddTradingResult");

  query->bindValue(":tsId", tsId);
  query->bindValue(":fiId", fiId);
  query->bindValue(":marketId", marketId);
  query->bindValue(":wltp", wltp);
  query->bindValue(":lltp", lltp);
  query->bindValue(":agwlp", agwlp);
  query->bindValue(":alltp", alltp);
  query->bindValue(":tpp", tpp);
  query->bindValue(":score", score);

  int result = execute(query);

  if(result < eSuccess) return false;

  return true;
}

QSqlQuery* FiluU::getBTDetails(const QString& strategy)
{
  if(!initQuery("GetBTDetails")) return 0;

  QSqlQuery* query = mSQLs.value("GetBTDetails");

  query->bindValue(":strategy", strategy);
  int result = execute(query);

  if(result <= eError) return 0;

  return query;
}

void FiluU::createTables()
{
  QString sql;
  readSqlStatement("CreateUserTables", sql);
  QSqlQuery* query = new QSqlQuery(QSqlDatabase::database(mConnectionName));

  bool ok = query->exec(sql);
  if(!ok)
  {
    qDebug() << "FiluU::createTables: error while exec: " << sql;
    QSqlError err = query->lastError();
    qDebug() << ":-(" << err.databaseText();
  }
  delete query;
}
