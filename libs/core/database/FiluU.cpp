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

  QString sql("SELECT nspname FROM pg_namespace WHERE nspname = ':user'");
  sql.replace(":user", mUserSchema);

  QSqlQuery query(mFiluDB);
  query.prepare(sql);
  execute(&query);
  if(query.size() == 0) createTables();
}

void FiluU::createFunctions()
{
  if(!initQuery("CreateUserFunctions")) return;
  QSqlQuery* query = mSQLs.value("CreateUserFunctions");
  int result = execute(query);
  if(result <= eError)
  {
    error(FUNC, tr("Can't create user functions."));
    return;
  }
  delete query;
  mSQLs.remove("CreateUserFunctions");

  verbose(FUNC, tr("User functions successful created."));
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
  if(!mSQLs.contains("_GetGroupId"))
  {
    QString sql("SELECT * FROM :user.group_id_from_path(:path)");
    sql.replace(":user", mUserSchema);

    QSqlQuery* query = new QSqlQuery(mFiluDB);
    query->prepare(sql);
    mSQLs.insert("_GetGroupId", query);
  }

  QSqlQuery* query = mSQLs.value("_GetGroupId");
  query->bindValue(":path", path);
  execute(query);
  query->next();

  return query->value(0).toInt();
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

  mFiluDB.transaction();
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
      mFiluDB.rollback();
      return false;
    }
  }

  mFiluDB.commit();
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
                  , double limit, bool buy, int marketId, int status, const QString& note, int orderId/* = 0*/)
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
  query->bindValue(":note", note);
  query->bindValue(":orderId", orderId);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}

int FiluU::addDepot(const QString& name, const QString& owner, const QString& trader
                  , const QString& currency, const QString& broker, int depotId/* = 0*/)
{
  // Returns Id or error
  if(!initQuery("AddDepot")) return eInitError;

  QSqlQuery* query = mSQLs.value("AddDepot");

  query->bindValue(":depotId", depotId);
  query->bindValue(":name", name);
  query->bindValue(":owner", owner);
  query->bindValue(":trader", trader);
  query->bindValue(":currency", currency);
  query->bindValue(":broker", broker);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}

int FiluU::addDepotPos(int depotId, const QDate& date
                     , int fiId, int pieces, double price
                     , int marketId, const QString& note, int depotPosId/* = 0*/)
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
  query->bindValue(":note", note);
  query->bindValue(":depotPosId", depotPosId);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}

int FiluU::addAccPosting(int depotId, const QDate& date
                       , int type, const QString& text
                       , double value, double accPostingId/* = 0*/)
{
  // Returns Id or error
  if(!initQuery("AddAccountPos")) return eInitError;

  QSqlQuery* query = mSQLs.value("AddAccountPos");

  query->bindValue(":depotId", depotId);
  query->bindValue(":date", date.toString(Qt::ISODate));
  query->bindValue(":type", type);
  query->bindValue(":text", text);
  query->bindValue(":value", value);
  query->bindValue(":accountId", accPostingId);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}

double FiluU::getDepotCash(int depotId, const QDate& date/* = QDate(3000, 01, 01)*/)
{
  if(!initQuery("GetDepotCash")) return eInitError;

  QSqlQuery* query = mSQLs.value("GetDepotCash");

  query->bindValue(":depotId", depotId);
  query->bindValue(":date", date.toString(Qt::ISODate));

  if(execute(query) <= eError) return eExecError;

  query->next();
  return query->value(0).toDouble();
}

double FiluU::getDepotNeededCash(int depotId, const QDate& date/* = QDate(3000, 01, 01)*/)
{
  if(!initQuery("GetDepotNeededCash")) return eInitError;

  QSqlQuery* query = mSQLs.value("GetDepotNeededCash");

  query->bindValue(":depotId", depotId);

  if(date == QDate(3000, 01, 01))
    query->bindValue(":date", QDate::currentDate().toString(Qt::ISODate));
  else
    query->bindValue(":date", date.toString(Qt::ISODate));

  if(execute(query) <= eError) return eExecError;

  query->next();
  return query->value(0).toDouble();
}

double FiluU::getDepotValue(int depotId, const QDate& date/* = QDate(3000, 01, 01)*/)
{
  if(!initQuery("GetDepotValue")) return eInitError;

  QSqlQuery* query = mSQLs.value("GetDepotValue");

  query->bindValue(":depotId", depotId);
  query->bindValue(":date", date.toString(Qt::ISODate));

  if(execute(query) <= eError) return eExecError;

  query->next();
  return query->value(0).toDouble();
}

QSqlQuery* FiluU::getOrders(int depotId, int status/* = -1*/, int fiId/* = -1*/)
{
  if(!initQuery("GetDepotOrders")) return 0;

  QSqlQuery* query = mSQLs.value("GetDepotOrders");

  query->bindValue(":orderId", -1);
  query->bindValue(":depotId", depotId);
  query->bindValue(":status", status);
  query->bindValue(":fiId", fiId);

  if(execute(query) <= eError) return 0;

  return query;
}

int FiluU::FiluU::orderStatus(const QString& status)
{
  bool ok;
  int s = status.toInt(&ok);
  if(ok)
  {
    if((s >= eOrderAdvice) and (s <= eOrderActive)) return s;
  }
  else
  {
    const QString sl = status.toLower();
    if(sl == orderStatus(eOrderAdvice).toLower())   return eOrderAdvice;
    if(sl == orderStatus(eOrderExperied).toLower()) return eOrderExperied;
    if(sl == orderStatus(eOrderExecuted).toLower()) return eOrderExecuted;
    if(sl == orderStatus(eOrderCanceled).toLower()) return eOrderCanceled;
    if(sl == orderStatus(eOrderNeedHelp).toLower()) return eOrderNeedHelp;
    if(sl == orderStatus(eOrderActive).toLower())   return eOrderActive;
  }

  error(FUNC, tr("Order status '%1' is unknown.").arg(status));

  return eError;
}

QString FiluU::orderStatus(int status)
{
  switch(status)
  {
    case eOrderAdvice:   return tr("Advice");
    case eOrderExperied: return tr("Experied");
    case eOrderExecuted: return tr("Executed");
    case eOrderCanceled: return tr("Canceled");
    case eOrderNeedHelp: return tr("Unsure");
    case eOrderActive:   return tr("Active");
    default:             break;
  }

  error(FUNC, tr("Order status number '%1' is unknown.").arg(status));

  return "UnknownStatus";
}

int FiluU::orderType(const QString& type)
{
  bool ok;
  int ot = type.toInt(&ok);
  if(ok)
  {
    if((ot >= eSellOrder) and (ot <= eBuyOrder)) return ot;
  }
  else
  {
    const QString otl = type.toLower();
    if(otl == orderType(eSellOrder).toLower()) return eSellOrder;
    if(otl == orderType(eBuyOrder).toLower())  return eBuyOrder;
  }

  error(FUNC, tr("Order type number '%1' is unknown.").arg(type));

  return eError;
}

QString FiluU::orderType(int type)
{
  switch(type)
  {
    case eSellOrder: return tr("Sell");
    case eBuyOrder:  return tr("Buy");
    default:         break;
  }

  error(FUNC, tr("Order type '%1' is unknown.").arg(type));

  return "Unknown"; // No tr()
}

int FiluU::accPostingType(const QString& type)
{
  bool ok;
  int pt = type.toInt(&ok);
  if(ok)
  {
    if((pt >= ePostCashIn) and (pt <= ePostFee)) return pt;
  }
  else
  {
    const QString ptl = type.toLower();
    if(ptl == accPostingType(ePostCashIn).toLower())  return ePostCashIn;
    if(ptl == accPostingType(ePostCashOut).toLower()) return ePostCashOut;
    if(ptl == accPostingType(ePostFiBuy).toLower())   return ePostFiBuy;
    if(ptl == accPostingType(ePostFiSell).toLower())  return ePostFiSell;
    if(ptl == accPostingType(ePostFee).toLower())     return ePostFee;
  }

  error(FUNC, tr("Posting type '%1' is unknown.").arg(type));

  return eError;
}

QString FiluU::accPostingType(int type)
{
  switch(type)
  {
    case ePostCashIn:  return tr("CashIn");
    case ePostCashOut: return tr("CashOut");
    case ePostFiBuy:   return tr("FiBuy");
    case ePostFiSell:  return tr("FiSell");
    case ePostFee:     return tr("Fee");
    default:           break;
  }

  error(FUNC, tr("Posting type number '%1' is unknown.").arg(type));

  return "Unknown"; // No tr()
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

  verbose(FUNC, tr("User tables successful created."));

  createFunctions();
}
