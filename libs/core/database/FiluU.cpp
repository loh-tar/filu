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

#include "FiluU.h"

#include "RcFile.h"

FiluU::FiluU(const QString& connectionName, RcFile* rcFile)
     : Filu(connectionName, rcFile)
{}

FiluU::~FiluU()
{}

void FiluU::openDB()
{
  Filu::openDB();

  if(hasError()) return;

  mUserSchema = "user_" + qgetenv("USER");

  QString sql("select nspname from pg_namespace where nspname = ':user'");
  sql.replace(":user", mUserSchema);

  QSqlQuery query(QSqlDatabase::database(mConnectionName));
  query.prepare(sql);
  execute(&query);
  if(query.size() == 0) createTables();
}

QSqlQuery* FiluU::searchFi(const QString& name, const QString& type)
{
  if(!initQuery("SearchFi")) return 0;

  QSqlQuery* query = mSQLs.value("SearchFi");

  query->bindValue(":name", name);
  query->bindValue(":type", type);

  if(execute(query) < eData) return 0;

  return query;
}

QSqlQuery* FiluU::getGroups(int motherId /*= -1*/)
{
  if(!initQuery("GetGroups")) return 0;

  QSqlQuery* query = mSQLs.value("GetGroups");

  query->bindValue(":motherId", motherId);

  if(execute(query) < eData) return 0;

  return query;
}

QSqlQuery* FiluU::getGMembers(int groupId)
{
  if(groupId < 0) return 0;

  if(!initQuery("GetGMembers")) return 0;

  QSqlQuery* query = mSQLs.value("GetGMembers");

  query->bindValue(":groupId", groupId);

  if(execute(query) < eData) return 0;

  return query;
}

int FiluU::getGroupId(const QString& path)
{
  QStringList groups = path.split("/", QString::SkipEmptyParts);

  int groupId  = 0; // Set to root group
  foreach(QString group, groups)
  {
    QSqlQuery* query = getGroups(groupId);
    groupId = -1; // Set to not valid
    while(query->next())
    {
      if(query->value(1).toString() == group)
      {
        groupId = query->value(0).toInt();
        break;
      }
    }

    if(groupId < 0) break; // Not found
  }

  return groupId;
}

void FiluU::addToGroup(int groupId, int fiId)
{
  if(!initQuery("AddGMember")) return;

  QSqlQuery* query = mSQLs.value("AddGMember");

  query->bindValue(":groupId", groupId);
  query->bindValue(":fiId", fiId);
  /*int result = */execute(query);
}

int FiluU::addGroup(const QString& path)
{
  // Returns GroupId or error
  if(!initQuery("AddGroup")) return eInitError;

  QSqlQuery* query = mSQLs.value("AddGroup");

  query->bindValue(":groupPath", path);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
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

COTuple* FiluU::getCOs(int fiId, int /*marketId*/
                     , const QString& fromDate/* = "1000-01-01"*/
                     , const QString& toDate  /* = "3000-01-01"*/)
{
  if(!initQuery("GetCOs")) return 0;

  QSqlQuery* query = mSQLs.value("GetCOs");

  query->bindValue(":fiId", fiId);
  query->bindValue(":fromDate", fromDate);
  query->bindValue(":toDate", toDate);

  if(execute(query) < Filu::eData) return 0;

  COTuple* co = new COTuple(query->size());
  while(co->next())
  {
    query->next();
    int i = co->mIndex;
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
    if(execute(query) <= eError)
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

  if(execute(query) < eData) return 0;

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

QSqlQuery* FiluU::getBTLog(int backtestId, int fiId, int marketId)
{
  if(!initQuery("GetBTLog")) return 0;

  QSqlQuery* query = mSQLs.value("GetBTLog");

  query->bindValue(":backtestId", backtestId);
  query->bindValue(":fiId", fiId);
  query->bindValue(":marketId", marketId);

  if(execute(query) < eData) return 0;

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

  if(execute(query) < eSuccess) return 0;

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

  if(execute(query) < eSuccess) return false;

  return true;
}

QSqlQuery* FiluU::getBTDetails(const QString& strategy)
{
  if(!initQuery("GetBTDetails")) return 0;

  QSqlQuery* query = mSQLs.value("GetBTDetails");

  query->bindValue(":strategy", strategy);

  if(execute(query) <= eError) return 0;

  return query;
}

int FiluU::addOrder(int depotId, const QDate& oDate, const QDate& vDate, int fiId, int pieces
                       , double limit, bool buy, int marketId, int status, int orderId/* = 0*/)
{
  // Returns Id or error
  if(!initQuery("AddDepotOrder")) return eInitError;

  QSqlQuery* query = mSQLs.value("AddDepotOrder");

  query->bindValue(":depotId", depotId);
  query->bindValue(":oDate", oDate);
  query->bindValue(":vDate", vDate);
  query->bindValue(":fiId", fiId);
  query->bindValue(":pieces", pieces);
  query->bindValue(":limit", limit);
  query->bindValue(":buy", buy);
  query->bindValue(":marketId", marketId);
  query->bindValue(":status", status);
  query->bindValue(":orderId", orderId);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}

int FiluU::addDepotPos(int depotId, const QDate& date
                     , int fiId, int pieces, double price
                     , int marketId, int depotPosId/* = 0*/)
{
  // Returns Id or error
  if(!initQuery("AddDepotPos")) return eInitError;

  QSqlQuery* query = mSQLs.value("AddDepotPos");

  query->bindValue(":depotId", depotId);
  query->bindValue(":date", date.toString(Qt::ISODate));
  query->bindValue(":fiId", fiId);
  query->bindValue(":pieces", pieces);
  query->bindValue(":price", price);
  query->bindValue(":marketId", marketId);
  query->bindValue(":depotPosId", depotPosId);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}

int FiluU::addAccountPos(int depotId, const QDate& date
                       , int type, const QString& text
                       , double value, double accountPosId/* = 0*/)
{
  // Returns Id or error
  if(!initQuery("AddAccountPos")) return eInitError;

  QSqlQuery* query = mSQLs.value("AddAccountPos");

  query->bindValue(":depotId", depotId);
  query->bindValue(":date", date.toString(Qt::ISODate));
  query->bindValue(":type", type);
  query->bindValue(":text", text);
  query->bindValue(":value", value);
  query->bindValue(":accountId", accountPosId);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}

double FiluU::getDepotCash(int depotId, const QDate& date/* = QDate(3000, 01, 01)*/)
{
  // Returns cash
  if(!initQuery("GetDepotCash")) return eInitError;

  QSqlQuery* query = mSQLs.value("GetDepotCash");

  query->bindValue(":depotId",depotId );
  query->bindValue(":date", date.toString(Qt::ISODate));

  if(execute(query) <= eError) return eExecError;

  query->next();
  return query->value(0).toInt();
}

QSqlQuery* FiluU::getOrders(int depotId, int status/* = 5*/, int fiId/* = -1*/)
{
  if(!initQuery("GetDepotOrders")) return 0;

  QSqlQuery* query = mSQLs.value("GetDepotOrders");

  query->bindValue(":depotId", depotId);
  query->bindValue(":status", status);
  query->bindValue(":fiId", fiId);

  if(execute(query) <= eError) return 0;

  return query;
}

void FiluU::createTables()
{
  if(!initQuery("CreateUserTables")) return;
  QSqlQuery* query = mSQLs.value("CreateUserTables");
  int result = execute(query);
  if(result <= eError)
  {
    error(FUNC, tr("Can't create user tables."));
    return;
  }
  delete query;
  mSQLs.remove("CreateUserTables");

  if(!initQuery("CreateUserFunctions")) return;
  query  = mSQLs.value("CreateUserFunctions");
  result = execute(query);
  if(result <= eError)
  {
    error(FUNC, tr("Can't create user functions."));
    return;
  }
  delete query;
  mSQLs.remove("CreateUserFunctions");
}
