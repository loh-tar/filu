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

#include <QDir>
#include <QFile>
#include <QSet>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>

#include "Filu.h"

#include "BarTuple.h"
#include "BrokerTuple.h"
#include "FiTuple.h"
#include "MarketTuple.h"
#include "RcFile.h"
#include "SymbolTuple.h"
#include "SymbolTypeTuple.h"



Filu::Filu(const QString& cn, RcFile* rcFile)
    : Newswire(FUNC)
    , mRcFile(rcFile)
    , mConnectionName(cn)
{
  mToDate = QDate::currentDate().toString(Qt::ISODate); // In case someone forget to set it...
//   setNoErrorLogging();
}

Filu::~Filu()
{
  closeDB();
}

int Filu::getMarketId(const QString& name)
{
  const int retVal = searchCaption("market", name);

  if(retVal < eData) return retVal;

  setSqlParm(":marketId", retVal);
  setSqlParm(":market", name);

  return retVal;
}

int Filu::getFiIdBySymbol(const QString& caption)
{
  setSqlParm(":symbol", caption);

  const QString sql("SELECT * FROM :filu.fiid_from_symbolcaption(:symbol)");

  if(!initQuery("_GetFiIdBySymbol", sql)) return eInitError;

  QSqlQuery* query = mSQLs.value("_GetFiIdBySymbol");

  query->bindValue(":symbol", caption);

  if(execute(query) <= eError) return eExecError;

  int retVal = result(FUNC, query);

  if(retVal >= eData) setSqlParm(":fiId", retVal);

  return retVal;
}

void Filu::setBarsToLoad(int count)
{
  mLimit = count;
}

void Filu::setDaysToFetchIfNoData(int days)
{
  mDaysToFetchIfNoData = days;
}

BarTuple* Filu::getBars(const QString& symbol, const QString& market
                      , const QString& fromDate/* = "1000-01-01"*/
                      , const QString& toDate/* = "3000-01-01"*/)
{
  int retVal;

  retVal = getFiIdBySymbol(symbol);
  if(retVal < eData) return 0;

  int fiId = retVal;

  retVal = getMarketId(market);
  if(retVal < eData) return 0;

  return getBars(fiId, retVal, fromDate, toDate);
}

BarTuple* Filu::getBars(int fiId, int marketId
                      , const QString& fromDate/* = "1000-01-01"*/
                      , const QString& toDate/* = "3000-01-01"*/)
{
  const QString sql("SELECT * FROM :filu.eodbar_get"
                    "( cast(:fiId     as int)"
                    ", cast(:marketId as int)"
                    ", cast(:fromDate as date)"
                    ", cast(:toDate   as date)"
                    ", cast(null      as int) )");

  if(!initQuery("_GetBars", sql)) return 0;

  QSqlQuery* query = mSQLs.value("_GetBars");

  query->bindValue(":fiId", fiId);
  query->bindValue(":marketId", marketId);
  query->bindValue(":fromDate", fromDate);
  query->bindValue(":toDate", toDate);

  if(execute(query) < eData) return 0;

  BarTuple* bars = fillQuoteTuple(query);

  if(bars)
  {
    bars->mFiId     = fiId;
    bars->mMarketId = marketId;
  }

  return bars;
}

BarTuple* Filu::getBars(const QString& symbol, const QString& market
                       , int limit, const QString& toDate/* = "3000-01-01"*/)
{
  int retVal;

  retVal = getFiIdBySymbol(symbol);
  if(retVal < eData) return 0;

  int fiId = retVal;

  retVal = getMarketId(market);
  if(retVal < eData) return 0;

  return getBars(fiId, retVal, limit, toDate);
}

BarTuple* Filu::getBars(int fiId, int marketId
                       , int limit, const QString& toDate/* = "3000-01-01"*/)
{
  if(!limit) return 0;

  if(!initQuery("GetBarsLtd")) return 0;

  QSqlQuery* query = mSQLs.value("GetBarsLtd");

  query->bindValue(":fiId", fiId);
  query->bindValue(":marketId", marketId);
  query->bindValue(":limit", limit);
  query->bindValue(":toDate", toDate);

  if(execute(query) < eData) return 0;

  BarTuple* bars = fillQuoteTuple(query);

  if(bars)
  {
    bars->mFiId     = fiId;
    bars->mMarketId = marketId;
  }

  return bars;
}

SymbolTuple* Filu::getSymbols(int fiId, const QString& fiType
                            , const QString& symbolType
                            , const QString& symbol
                            , const QString& market
                            , bool onlyProviderSymbols)
{
  if(!initQuery("GetSymbols")) return 0;

  QSqlQuery* query = mSQLs.value("GetSymbols");

  query->bindValue(":fiId", fiId);
  query->bindValue(":ftype", fiType);
  query->bindValue(":provider", symbolType);
  query->bindValue(":symbol", symbol);
  query->bindValue(":market", market);
  query->bindValue(":onlyProviderSymbols", onlyProviderSymbols);

  if(execute(query) < eData) return 0;

  return fillSymbolTuple(query);
}

SymbolTuple* Filu::getSymbols(int fiId)
{
  if(!initQuery("GetSymbolsToFiId")) return 0;

  QSqlQuery* query = mSQLs.value("GetSymbolsToFiId");

  query->bindValue(":fiId", fiId);

  if(execute(query) < eData) return 0;

  return fillSymbolTuple(query);
}

SymbolTuple* Filu::getAllProviderSymbols()
{
  return getSymbols(0, "", "", "", "", true);
}

SymbolTuple* Filu::searchSymbol(const QString& symbol
                              , const QString& market /* = "" */
                              , const QString& owner  /* = "" */)
{
  return getSymbols(0, "", owner, symbol, market, false);
}

SymbolTypeTuple* Filu::getSymbolTypes(int filter/* = eAllTypes FIXME, bool orderBySeq = true*/)
{
  if(!initQuery("GetSymbolTypes")) return 0;

  QSqlQuery* query = mSQLs.value("GetSymbolTypes");

  bool allTypes;
  bool isProvider;

  switch(filter)
  {
    case eOnlyProvider:
      allTypes   = false;
      isProvider = true;
      break;

    case eOnlyNonProvider:
      allTypes   = false;
      isProvider = false;
      break;

    case eAllTypes:
    default:
      allTypes   = true;
      isProvider = true;
      break;
  }

  query->bindValue(":all", allTypes);
  query->bindValue(":isProvider", isProvider);
  //query->bindValue(":", orderBySeq);

  if(execute(query) < eData) return 0;

  SymbolTypeTuple* symbolType = new SymbolTypeTuple(query->size());
  while(symbolType->next())
  {
    query->next();

    int i = symbolType->mIndex;
    symbolType->mId[i]         = query->value(0).toInt();
    symbolType->mCaption[i]    = query->value(1).toString();
    symbolType->mSeq[i]        = query->value(2).toInt();
    symbolType->mIsProvider[i] = query->value(3).toInt();
  }
  symbolType->rewind();

  return symbolType;
}

MarketTuple* Filu::getMarkets(const QString& name/* = ""*/)
{
  if(!initQuery("GetMarket")) return 0;

  QSqlQuery* query = mSQLs.value("GetMarket");

  query->bindValue(":market", name);
  query->bindValue(":marketId", 0);  // Don't use id

  if(execute(query) < eData) return 0;

  return fillMarketTuple(query);
}

MarketTuple* Filu::getMarket(int marketId)
{
  if(!initQuery("GetMarket")) return 0;

  QSqlQuery* query = mSQLs.value("GetMarket");

  query->bindValue(":market", "");  // Don't use name
  query->bindValue(":marketId", marketId);

  if(execute(query) < eData) return 0;

  MarketTuple* market = fillMarketTuple(query);

  if(market) market->next();

  return market;
}

FiTuple* Filu::getFi(int fiId)
{
  // Not yet used, but soon...I hope
  if(!initQuery("GetFi")) return 0;

  QSqlQuery* query = mSQLs.value("GetFi");

  query->bindValue(":name", "");
  query->bindValue(":type", "");
  query->bindValue(":fuzzy", false);
  query->bindValue(":fiId", fiId);

  if(execute(query) < eData) return 0;

  FiTuple* fi = fillFiTuple(query);
  if(fi) fi->next();

  return fi;
}

FiTuple* Filu::getFiLike(const QString& pattern)
{
  if(!initQuery("GetFi")) return 0;

  QSqlQuery* query = mSQLs.value("GetFi");

  query->bindValue(":name", pattern);
  query->bindValue(":type", "");
  query->bindValue(":fuzzy", true);
  query->bindValue(":fiId", 0);

  if(execute(query) < eData) return 0;

  return fillFiTuple(query);
}

FiTuple* Filu::getFiBySymbol(const QString& symbol)
{
  int fiId = getFiIdBySymbol(symbol);

  if(fiId < 1) return 0;

  return getFi(fiId);
}

int Filu::getFiTypes(QStringList& type)
{
  type.clear();

  if(!initQuery("GetAllFiTypes")) return eInitError;

  QSqlQuery* query = mSQLs.value("GetAllFiTypes");

  int result = execute(query);
  if(result <= eError)  return eExecError;
  if(result == eNoData)
  {
    error(FUNC, tr("No FiTypes found."));
    return eNoData;
  }

  while(query->next())
  {
    type.append(query->value(0).toString());
  }

  return eData;
}

int Filu::getEODBarDateRange(DateRange& dateRange
                           , int fiId, int marketId, int quality)
{
  // Get the dates of the first and the last bar stored in Filu.

  // If no data in table we want fetch from these date on
  dateRange.insert("first", QDate::currentDate().addDays(mDaysToFetchIfNoData * -1));
  dateRange.insert("last", QDate::currentDate().addDays(mDaysToFetchIfNoData * -1));

  if(!initQuery("GetEODBarDateRange")) return eInitError;

  QSqlQuery* query = mSQLs.value("GetEODBarDateRange");

  query->bindValue(":fiId", fiId);
  query->bindValue(":marketId", marketId);
  query->bindValue(":quality", quality);

  int result = execute(query);
  if(result <= eError)  return eExecError;
  if(result == eNoData) return eNoData;

  query->next();
  dateRange.insert("first", query->value(0).toDate());
  dateRange.insert("last", query->value(1).toDate());

  return eData;
}

BrokerTuple* Filu::getBroker(int brokerId/* = 0*/)
{
  if(!initQuery("GetBroker")) return 0;

  QSqlQuery* query = mSQLs.value("GetBroker");

  query->bindValue(":brokerId", brokerId);

  if(execute(query) < eData) return 0;

  BrokerTuple* broker = new BrokerTuple(query->size());
  while(query->next())
  {
    broker->next();

    int i = broker->mIndex;
    broker->mId[i]         = query->value(0).toInt();
    broker->mName[i]       = query->value(1).toString();
    broker->mFeeFormula[i] = query->value(4).toString();
    broker->mQuality[i]    = query->value(5).toInt();
  }

  if(!brokerId) broker->rewind();

  return broker;
}

int Filu::getIndicatorNames(QStringList* names, const QString& like /* = "" */)
{
  if(!initQuery("GetIndicator")) return eInitError;

  QSqlQuery* query = mSQLs.value("GetIndicator");

  query->bindValue(":name", "fpi_" + like.toLower());

  int result = execute(query);
  if(result <= eError)  return eExecError;
  if(result == eNoData)
  {
    error(FUNC, tr("No Indicator match '%1'.").arg(like));
    return eNoData;
  }

  names->clear();

  while(query->next())
  {
    names->append(query->value(0).toString().toUpper());
  }

  return eData;
}

int Filu::getIndicatorInfo(KeyVal* info, const QString& name)
{
  if(!initQuery("GetIndicator")) return eInitError;

  QSqlQuery* query = mSQLs.value("GetIndicator");

  query->bindValue(":name", "fpi_" + name.toLower());

  int result = execute(query);
  if(result <= eError)  return eExecError;
  if(result == eNoData)
  {
    error(FUNC, tr("Indicator '%1' not found.").arg(name));
    return eNoData;
  }

  query->next();

  info->clear();
  info->insert("Name", query->value(0).toString().toUpper());

  // Extract function parameter out of column "arguments"
  QStringList parm = query->value(1).toString().split(",");
  parm[0].remove("{");
  parm[parm.size() - 1].remove("}");
  for(int i = 0; i < parm.size(); ++i)
  {
    info->insert("Parm" + QString::number(i + 1), ":" + parm.at(i));
  }

  // The SQL to use for calling the indicator
  info->insert("Call", query->value(3).toString());

  // Extract all indicator attributes out of column "source"
  QStringList source = query->value(4).toString().split("\n");
  foreach(QString line, source)
  {
    line = line.trimmed();
    if(line.startsWith("BEGIN", Qt::CaseInsensitive)) break;
    if(!line.startsWith("--")) continue;

    int i = line.indexOf(':');
    QString attrib = line.left(i);
    attrib.remove("--");
    if(attrib.isEmpty()) continue;
    QString value = line.mid(i + 1);

    if(attrib.startsWith("Desc", Qt::CaseInsensitive))
    {
      value.replace("<b>", "\n");
      QString desc = info->value("Desc");
      if(!desc.isEmpty()) desc.append("\n");

      desc.append(value);
      info->insert(attrib, desc);
    }
    else
    {
      info->insert(attrib, value);
    }
  }

// qDebug() << info->keys();
// qDebug() << info->values();
  return eData;
}

int Filu::prepareIndicator(const QString& name, const QString& call /* = "" */)
{
  if(mSQLs.contains("FPI_" + name))
  {
    return eSuccess;
  }

  QString sql = call;
  StringSet parameters; // Collect the parameter of the SQL

  if(sql.isEmpty())
  {
    // No call given, we have to take the info
    KeyVal info;
    getIndicatorInfo(&info, name);
    sql = info.value("Call");
    QString parm;
    for(int i = 1; ; ++i)
    {
      parm = info.value("Parm" + QString::number(i));
      if(parm.isEmpty()) break;
      parameters.insert(parm);
    }
  }
  else
  {
    // To save a little bit time we extract the needed parameter ot of call
    QStringList input = call.split(", ");
    input[0].remove(QRegExp(".+\\("));
    input[input.size() - 1].remove(")");
    for(int i = 0; i < input.size(); ++i)
    {
      parameters.insert(input.at(i));
    }
  }

  QSqlQuery* query = new QSqlQuery(mFiluDB);

  bool ok = query->prepare(sql);
  if(!ok)
  {
    error(FUNC, tr("Can't prepare indicator '%1'.").arg(name));
    return eInitError;
  }

  mSqlParmNames.insert("FPI_" + name, parameters);
  mSQLs.insert("FPI_" + name, query);

  return eSuccess;
}

QSqlQuery* Filu::callIndicator(const QString& name)
{
  return execSql("FPI_" + name);
}

QSqlQuery* Filu::execSql(const QString& name)
{
  // This is a universal access function. The 2nd way
  // to fetch or save information into the DB.
  // All needed parameter has to set previously by setSqlParm()

  if(!initQuery(name)) return 0;

  QSqlQuery* query = mSQLs.value(name);

  // Set the parameter for the SQL, if some
  QSetIterator<QString> i(mSqlParmNames.value(name));
  while (i.hasNext())
  {
    QString parm = i.next();
    query->bindValue(parm, mSqlParm.value(parm));
    //qDebug() << "Filu::execSql:" << parm << mSqlParm.value(parm);
  }

  if(execute(query) <= eError) return 0;

  return query;
}

void Filu::setSqlParm(const QString& parm, const QVariant& value)
{
  // Work together with execSql(). All parameter a SQL need
  // has to set by multiple call of this function

  mSqlParm.insert(parm, value);
}

int Filu::quality(const QString& qual)
{
  bool ok;
  int q = qual.toInt(&ok);
  if(ok)
  {
    if((q >= ePlatinum) and (q <= eTin)) return q;
  }
  else
  {
    const QString ql = qual.toLower();
    if(ql == quality(ePlatinum).toLower()) return ePlatinum;
    if(ql == quality(eGold).toLower())     return eGold;
    if(ql == quality(eBronze).toLower())   return eBronze;
    if(ql == quality(eTin).toLower())      return eTin;
  }

  error(FUNC, tr("Quality '%1' is unknown.").arg(qual));

  return eError;
}

QString Filu::quality(int quality)
{
  switch(quality)
  {                        // Never tr()
    case ePlatinum: return "Platinum";
    case eGold:     return "Gold";
    case eBronze:   return "Bronze";
    case eTin:      return "Tin";
    default:        break;
  }

  error(FUNC, tr("Quality '%1' is unknown.").arg(quality));

  return "Unknown";
}

int Filu::convertCurrency(double& money, int sCurrId, int dCurrId, const QDate& date)
{
  if(!money) return eData; // Nice, nothing todo

  const QString sql("SELECT * FROM :filu.convert_currency(:money, :sCurr, :dCurr, :date)");

  if(!initQuery("_ConvertCurrency", sql)) return eInitError;

  QSqlQuery* query = mSQLs.value("_ConvertCurrency");

  query->bindValue(":money", money);
  query->bindValue(":sCurr", sCurrId);
  query->bindValue(":dCurr", dCurrId);
  query->bindValue(":date", date);

  if(execute(query) <= eError) return eExecError;

  query->next();
  if(-money == query->value(0).toDouble())
  {
    warning(FUNC, tr("Can't convert currency! Probably no  data available."));
    return eError;
  }

  money = query->value(0).toDouble();

  return eData;
}

int Filu::searchCaption(const QString& table, const QString& caption)
{
  const QString sql("SELECT * FROM :filu.id_from_caption(:table, :caption)");

  if(!initQuery("_GetIdByCaption", sql)) return eInitError;

  QSqlQuery* query = mSQLs.value("_GetIdByCaption");

  query->bindValue(":table", table);
  query->bindValue(":caption", caption);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}

int Filu::addFiType(const QString& type, int id/* = 0*/)
{
  const QString sql("SELECT * FROM :filu.ftype_insert(:fiType, :fiTypeId)");

  if(!initQuery("_AddFiType", sql)) return eInitError;

  QSqlQuery* query = mSQLs.value("_AddFiType");

  query->bindValue(":fiType", type);
  query->bindValue(":fiTypeId", id);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}

int Filu::addSymbolType(const QString& type
                       , const int& seq
                       , const bool isProvider
                       , const int& id/* = 0*/)
{
  const QString sql("SELECT * FROM :filu.stype_insert(:stypeId, :symbolType, :seq, :isProvider)");

  if(!initQuery("_AddSymbolType", sql)) return eInitError;

  QSqlQuery* query = mSQLs.value("_AddSymbolType");

  query->bindValue(":symbolType", type);
  query->bindValue(":seq", seq);
  query->bindValue(":isProvider", isProvider);
  query->bindValue(":stypeId", id);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}

int Filu::addMarket(const QString& market
                  , const QString& currency
                  , const QString& currSymbol)
{
  const QString sql("SELECT * FROM :filu.market_insert(:market, :currencyName, :currencySymbol)");

  if(!initQuery("_AddMarket", sql)) return eInitError;

  QSqlQuery* query = mSQLs.value("_AddMarket");

  query->bindValue(":market", market);
  query->bindValue(":currencyName", currency);
  query->bindValue(":currencySymbol", currSymbol);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}

int Filu::addEODBarData(int fiId, int marketId, const QStringList* data)
{
  // The StringList must at first line contain a header,
  // then follow the data.
  //
  // Date;Open;High;Low;Close;Volume;OpenInterest;Quality
  // 2010-04-30;269.31;270.57;261.00;263.54;14683200;;2
  // 2010-04-29;263.02;270.00;262.01;268.64;19943100;;1
  // ...

  if(!data)
  {
    fatal(FUNC, tr("Got no 'data'."));
    return eError;
  }

  if(data->size() < 2)
  {
    warning(FUNC, tr("No data from script."));
    return eNoData;
  }

//   QTextStream console(stdout);
//console << "Filu::addEODBarData\n";
//   QTime time;
//   time.restart();

  QStringList header = data->at(0).split(";");

  // Save the indexes of the data
  int iD  = header.indexOf("Date");
  if(iD == -1) return eError; // Without a date we can't do anything

  int iO  = header.indexOf("Open");
  int iH  = header.indexOf("High");
  int iL  = header.indexOf("Low");
  int iC  = header.indexOf("Close");
  int iV  = header.indexOf("Volume");
  int iOI = header.indexOf("OpenInterest");
  int iQ  = header.indexOf("Quality");

  transaction();
  // Add a list of bars to the DB
  if(!initQuery("AddBars")) return eInitError;

  QSqlQuery* query = mSQLs.value("AddBars");

  query->bindValue(":fiId", fiId);
  query->bindValue(":marketId", marketId);
  int barCount = 0;         // Let's count really committed bars
  int sqlExecCounter = 0;   // To compare with commitBlockSize aka 'commitFreq'
  int increment = 1;
  int j = 1;
  if(data->size() > 2)
  {
    // Not really necessary: we check the order of the data in the list and
    // align the counter j in the loop below
    QDate date1 = QDate::fromString(data->at(1).split(";")[iD], Qt::ISODate);
    QDate date2 = QDate::fromString(data->at(2).split(";")[iD], Qt::ISODate);
    if(date1 > date2)
    {
      increment = -1;
      j = data->size() -1;
    }
  }

  for(int i = 1; i < data->size(); i++)
  {
    // Extract the data
    QStringList values = data->at(j).split(";");
    query->bindValue(":date", values[iD]);

    QString v = (iO == -1) ? "" : values.at(iO);
    query->bindValue(":open", v.isEmpty() ? NULL : v);

    v = (iH == -1) ? "" : values.at(iH);
    query->bindValue(":high", v.isEmpty() ? NULL : v);

    v = (iL == -1) ? "" : values.at(iL);
    query->bindValue(":low", v.isEmpty() ? NULL : v);

    v = (iC == -1) ? "" : values.at(iC);
    query->bindValue(":close", v.isEmpty() ? NULL : v);

    v = (iV == -1) ? "" : values.at(iV);
    query->bindValue(":vol", v.isEmpty() ? NULL : v);

    v = (iOI == -1) ? "" : values.at(iOI);
    query->bindValue(":oi", v.isEmpty() ? NULL : v);

    v = (iQ == -1) ? "1" : values.at(iQ); // If not exist we assume the data are final
    query->bindValue(":status", v.isEmpty() ? NULL : v);

    ++sqlExecCounter;

    if(execute(query) <= eError)
    {
      rollback();
      error(FUNC, tr("Error while add EODBar with date '%1'.").arg(values[0]));
      if(barCount) errInfo(FUNC, tr("%1 bars previus added without trouble.").arg(barCount));
      return eExecError;
    }

    if(sqlExecCounter == mCommitBlockSize)
    {
      commit();
      transaction();

//       console << ".";

      sqlExecCounter = 0;
    }
    ++barCount;
    j += increment;
  }
 // if(sqlExecCounter > 0)
      commit();

//    console /*<< "Filu::addEODBarData: " */<< barCount << " bars added in "
//            << time.elapsed() << " ms\n" << flush;

  return eSuccess;
}

int Filu::addFiCareful(FiTuple& fi)
{
  // Insert or update the FI and add* all* symbols if they are not known.
  // To be sure, test each given Symbol if it is already known.
  // Test the FI name with FiType if it is already known.

  clearErrors();

  // Make sure is set on first entry
  fi.rewind(0);
  if(fi.isInvalid())
  {
    fatal(FUNC, "FI unvalid."); // You should never read this
    return eError;
  }

  SymbolTuple* symbol = fi.symbol();
  if(!symbol)
  {
    error(FUNC, tr("Can't add FI without Symbol."));
    return eError;
  }

  symbol->rewind();         // Make sure its placed at the beginning
  // Before we add anything,
  // we check each symbol until we found a known one
  int retVal = 0;
  while (symbol->next())
  {
    if(symbol->caption().isEmpty()) continue;

    retVal = getFiIdBySymbol(symbol->caption());
    if(retVal > 0) break;
    if(retVal == -3)
    {
       error(FUNC, tr("Error while searching Symbol '%1'.").arg(symbol->caption()));
       return eExecError;
    }
  }

  //if(retVal == -2) return; // We check already if empty

  if(retVal > 0)
  {
    fi.setId(retVal);
  }
  else
  {
    // No symbol found, try to add the FI
    int retVal = eNoData; // Set to "All symbols are bad"
    symbol->rewind();
    while(symbol->next())
    {
      // Search a non RefSymbol
      if(symbol->caption().isEmpty()) continue;
      if(symbol->market().isEmpty()) continue;
      if(symbol->owner().isEmpty()) continue;
      retVal = eData; // Set to "We have a valid symbol"
      break;
    }

    if(retVal == eNoData)
    {
      error(FUNC, tr("No valid symbol to add FI."));
      return eError;
    }

    retVal = addFi(fi.name()
                   , fi.type()
                   , symbol->caption()
                   , symbol->market()
                   , symbol->owner());

    if(retVal < eSuccess) return retVal;

    fi.setId(retVal);

    // Add new added fi to users group "New FIs"
    // FIXME: not possible, because groups are handled by FiluU :-/
  }

  // Add all symbols to the FI
  int count = 0;
  symbol->rewind();
  while (symbol->next())
  {
    // Add only full set Symbols
    if(symbol->caption().isEmpty()) continue;
    if(symbol->market().isEmpty()) continue;
    if(symbol->owner().isEmpty()) continue;

    const int retVal = addSymbol(symbol->caption()
                               , symbol->market()
                               , symbol->owner()
                               , fi.id());
    if(retVal < eSuccess)
    {
      --count;
    }
    ++count;
  }

  if(hasError() and (count > 0))
  {
    errInfo(FUNC, tr("%1 Symbols added without trouble.").arg(count));
  }

  return fi.id();
}

int Filu::addFi(FiTuple& fi)
{
  // Deviating to addFiCareful() don't check anything.
  // Just try to add the FI, but only the FI, no symbols.
  // Also, don't iterate thru the FiTuple add only
  // this one where is actual placed

  if(fi.isInvalid()) return eError;

  return addFi(fi.name(), fi.type(), fi.id());
}

int Filu::addFi(const QString& name
              , const QString& type
              , const int fiId)
{
  return addFi(name, type, "", "", "", fiId);
}

int Filu::addFi(const QString& name
              , const QString& type
              , const QString& symbol
              , const QString& market
              , const QString& stype
              , const int fiId/* = 0 */)
{
  // Insert or update the FI, add the symbol if not known.
  // The database will care about that the FI is not added twice when
  // the symbol is known. In that case the FI name and type will updatet.
  // If the symbol and the FI name is not found, both will inserted.

  clearErrors();

  if(name.isEmpty())
  {
    error(FUNC, tr("FI Name is empty."));
    return (eError);
  }
  if(type.isEmpty())
  {
    error(FUNC, tr("FI Type  is empty."));
    return (eError);
  }

  const QString sql("SELECT * FROM :filu.fi_insert(:fiId, :name, :fType, :symbol, :sType, :market)");

  if(!initQuery("_AddFi", sql)) return eInitError;

  QSqlQuery* query = mSQLs.value("_AddFi");

  query->bindValue(":fiId", fiId);
  query->bindValue(":name", name);
  query->bindValue(":fType", type);
  query->bindValue(":symbol", symbol);
  query->bindValue(":market", market);
  query->bindValue(":sType", stype);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}

int Filu::addSymbol(const QString& symbol
                  , const QString& market
                  , const QString& stype
                  , int fiId/* = 0*/
                  , int symbolId/* = 0*/)
{
  const QString sql("SELECT * FROM :filu.symbol_insert(:fiId, :symbolId, :caption, :market, :sType)");

  if(!initQuery("_AddSymbol", sql)) return eInitError;

  QSqlQuery* query = mSQLs.value("_AddSymbol");

  query->bindValue(":fiId", fiId);
  query->bindValue(":symbolId", symbolId);
  query->bindValue(":caption", symbol);
  query->bindValue(":market", market);
  query->bindValue(":sType", stype);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}


int Filu::addUnderlying(const QString& mother
                      , const QString& symbol
                      , const double weight)
{
  // Add all underlying FIs holded as symbols in FiTuple.
  // We don't delete all underlyings previous because it fit more to
  // all other addFoo functions.
  // To remove all existing underlyings call execSql("DelAllUnderlyingsFromMother");

  const QString sql("SELECT * FROM :filu.underlying_insert(:motherSymbol, :symbol, :weight)");

  if(!initQuery("_AddUnderlying", sql)) return eInitError;

  QSqlQuery* query = mSQLs.value("_AddUnderlying");

  query->bindValue(":motherSymbol", mother);
  query->bindValue(":symbol", symbol);
  query->bindValue(":weight", weight);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}

int Filu::addSplit(const QString& symbol
                 , const QString& date
                 , const double ratio
                 , const QString& comment
                 , const int& quality)
{
  if(!initQuery("AddSplit")) return eInitError;

  QSqlQuery* query = mSQLs.value("AddSplit");

  query->bindValue(":fiId", 0);        // The DB will search the FiId by symbol
                                       // FIXME:expand function here to use them too
  query->bindValue(":symbol", symbol);
  query->bindValue(":date", date);
  query->bindValue(":ratio", ratio);
  query->bindValue(":comment", comment);
  query->bindValue(":quality", quality);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}

int Filu::addBroker(BrokerTuple& bt)
{
  if(bt.isInvalid()) return eError;

  int retVal = addBroker(bt.name(), "fixme", bt.feeFormula(), bt.quality(), bt.id());

  if(retVal >= eData) bt.setId(retVal);

  return retVal;
}

int Filu::addBroker(const QString& name
                  , const QString& currSymbol
                  , const QString& feeFormula
                  , const int quality
                  , const int id/* = 0*/)
{

  const QString sql("SELECT * FROM :filu.broker_insert"
                    "( :name, :currSymbol, :feeFormula"
                    ", cast(:quality as int2)"
                    ", cast(:brokerId as int4))");

  if(!initQuery("_AddBroker", sql)) return eInitError;

  QSqlQuery* query = mSQLs.value("_AddBroker");

  query->bindValue(":brokerId", id);
  query->bindValue(":name", name);
  query->bindValue(":currSymbol", currSymbol);
  query->bindValue(":feeFormula", feeFormula);
  query->bindValue(":quality", quality);

  if(execute(query) <= eError) return eExecError;

  return result(FUNC, query);
}

void Filu::deleteRecord(const QString& schema, const QString& table, int id /*= -1*/)
{
  if(id == -1)
  {
    execute("_DelRec", QString("DELETE FROM %1.%2 ").arg(schema).arg(table));
  }
  else
  {
    execute("_DelOneRec", QString("DELETE FROM %1.%2 WHERE %2_id = %3").arg(schema).arg(table).arg(id));
  }
}

int Filu::updateField(const QString& field, const QVariant& newValue
                    , const QString& schema, const QString& table, int id)
{
  execute("_DelOneRec", QString("UPDATE %1.%2  SET %3 = '%4'  WHERE %2_id = %5")
                           .arg(schema, table, field, newValue.toString()).arg(id));

}

QString Filu::dbFuncErrText(int errorCode)
{
  if(!initQuery("GetDbError")) return "";

  QSqlQuery* query = mSQLs.value("GetDbError");

  query->bindValue(":errCode", -errorCode);

  if(execute(query) <= eError) return "";
  if(query->size() < 1) return "FATAL: Filu::dbFuncErrText: ErrorCode not found."; // Yes, no tr() and no error(...)

  query->next();
  return query->value(0).toString();
}

int Filu::getNextId(const QString& schema, const QString& table)
{
  const QString sql = QString("SELECT nextval('%1.%2_%2_id_seq')").arg(schema).arg(table);

  if(execute("_NextVal", sql) <= eError) return eExecError;

  return result(FUNC, mLastQuery);
}

void Filu::openDB()
{
  readSettings();
  mFiluDB = QSqlDatabase::addDatabase("QPSQL", mConnectionName);

  mFiluDB.setHostName(mRcFile->getST("HostName"));
  mFiluDB.setPort(mRcFile->getIT("HostPort"));
  mFiluDB.setDatabaseName(mRcFile->getST("DatabaseName"));
  mFiluDB.setUserName(mRcFile->getST("PgUserRole"));
  mFiluDB.setPassword(mRcFile->getST("Password"));
  bool ok = mFiluDB.open();
  if(!ok)
  {
    QSqlError err = mFiluDB.lastError();
    error(FUNC, tr("Can't open DB."));
    errInfo(FUNC, err.databaseText());
    if(verboseLevel() < eMax) printSettings(); // readSettings() has printed if eMax
    return;
  }

  // Test if the driver works properly
  setSqlParm(":foo", 123);
  execute("_TestDriver", "SELECT :foo = :foo");
  mLastQuery->next();
  if(!mLastQuery->value(0).toBool())
  {
    fatal(FUNC, tr("The PSql Driver works not properly."));
    errInfo(FUNC, tr("Please take a look at doc/qt-postgres-driver-bug.txt"));
    errInfo(FUNC, tr("PluginPath is set to: %1").arg(mRcFile->getST("PluginPath")));
    return;
  }

  execute("_FiluExist", "SELECT nspname FROM pg_namespace WHERE nspname = ':filu'");

  if(mLastResult == eSuccess)
  {
    verbose(FUNC, tr("Successful connected to %1 :-)").arg(mRcFile->getST("DatabaseName")), eAmple);
  }
  else
  {
    createSchema();
  }
}

void Filu::closeDB()
{
  QHashIterator<QString, QSqlQuery*> i(mSQLs);
  while(i.hasNext()) delete i.next().value();

  mFiluDB.close();
  mFiluDB = QSqlDatabase(); // http://lists.trolltech.com/qt-interest/2005-11/thread00735-0.html
  QSqlDatabase::removeDatabase(mConnectionName);
}

void Filu::createSchema()
{
  execute("_Psst!", "SET client_min_messages TO WARNING");

  execSql("filu/misc/schemata");
  if(hasError()) return;
  verbose(FUNC, tr("New filu schema '%1' successful created.").arg(mFiluSchema));

  execSql("filu/misc/languages");
  if(hasError()) return;
  verbose(FUNC, tr("Languags successful created."));

  execSql("filu/misc/data_types");
  if(hasError()) return;
  verbose(FUNC, tr("Data types successful created."));


  createTables();
  createFunctions();
  createViews();

  if(hasError()) return;

  execSql("filu/misc/table_entries");
  if(hasError()) return;
  verbose(FUNC, tr("Default table entries successful insert."));
}

void Filu::createTables()
{
  if(!executeSqls("filu/tables/")) return;

  verbose(FUNC, tr("Filu tables successful created."));
}

void Filu::createFunctions()
{
  if(!executeSqls("filu/functions/")) return;

  verbose(FUNC, tr("Filu functions successful created."));
}

void Filu::createViews()
{
  if(!executeSqls("filu/views/")) return;

  verbose(FUNC, tr("Filu views successful created."));
}

bool Filu::executeSqls(const QString& path)
{
  if(hasError()) return false;

  execute("_Psst!", "SET client_min_messages TO WARNING");
  QStringList sqls = QDir(mSqlPath + path).entryList(QDir::Files, QDir::Name);

  foreach(QString sql, sqls)
  {
    sql.chop(4); // Remove .sql suffix which will again added by execSql()
    execSql(path + sql);
    if(hasError()) return false;
  }

  return true;
}

BarTuple* Filu::fillQuoteTuple(QSqlQuery* tuple)
{
  int count = tuple->size();
  if(!count) return 0;

  // Fill the object to be returned to client
  BarTuple* bars = new BarTuple(count);
  while(bars->next())
  {
    tuple->next();

    int i = bars->mIndex;
    bars->mDate[i]   = tuple->value(0).toDate();
    bars->mTime[i]   = tuple->value(1).toTime();
    bars->mOpen[i]   = tuple->value(2).toDouble();
    bars->mHigh[i]   = tuple->value(3).toDouble();
    bars->mLow[i]    = tuple->value(4).toDouble();
    bars->mClose[i]  = tuple->value(5).toDouble();
    bars->mVolume[i] = tuple->value(6).toDouble();
  }

  bars->rewind();

  return bars;
}

FiTuple* Filu::fillFiTuple(QSqlQuery* tuple)
{
  int count = tuple->size();
  if(!count) return 0;

  // Fill the object to be returned to client
  FiTuple* fi = new FiTuple(count);
  while(fi->next())
  {
    tuple->next();

    int i = fi->mIndex;
    fi->mId[i]     = tuple->value(0).toInt();
    fi->mTypeId[i] = tuple->value(1).toInt();
    fi->mName[i]   = tuple->value(2).toString();
    fi->mType[i]   = tuple->value(3).toString();
    fi->mSymbol[i] = 0;
    //    fi->IssueDate[i] = tuple->value(3).toString();
    //    fi->MaturityDate[i] = tuple->value(3).toString();
  }

  fi->rewind();

  return fi;
}

MarketTuple* Filu::fillMarketTuple(QSqlQuery* query)
{
  int count = query->size();
  if(!count) return 0;

  // Fill the object to be returned to client
  MarketTuple* market = new MarketTuple(count);
  while(market->next())
  {
    query->next();

    int i = market->mIndex;
    market->mId[i]         = query->value(0).toInt();
    market->mName[i]       = query->value(1).toString();
    market->mCurrId[i]     = query->value(4).toInt();
    market->mCurrName[i]   = query->value(5).toString();
    market->mCurrSymbol[i] = query->value(6).toString();
  }

  market->rewind();

  return market;
}

SymbolTuple* Filu::fillSymbolTuple(QSqlQuery* query)
{
  int count = query->size();
  if(!count) return 0;

  // Fill the object to be returned to client
  SymbolTuple* symbols= new SymbolTuple(query->size());
  while(symbols->next())
  {
    query->next();

    int i = symbols->mIndex;
    symbols->mFiId[i]     = query->value(0).toInt();
    symbols->mMarketId[i] = query->value(1).toInt();
    symbols->mCaption[i]  = query->value(2).toString();
    symbols->mMarket[i]   = query->value(3).toString();
    symbols->mOwner[i]    = query->value(4).toString();
  }
  symbols->rewind();

  return symbols;
}

bool Filu::initQuery(const QString& name)
{
  clearErrors();

  if(mSQLs.contains(name)) return true;

  verbose(FUNC, name, eAmple);

  mLastResult = eInitError; // The glass is always half-empty

  QString sql;
  if(!loadQuery(name, sql)) return false;

  return initQuery(name, sql);
}

bool Filu::initQuery(const QString& name, const QString& rawSql)
{
  clearErrors();

  if(mSQLs.contains(name)) return true;

  verbose(FUNC, name, eAmple);

  mLastResult = eInitError; // The glass is always half-empty

  const QString sql = parseSql(name, rawSql);

  QSqlQuery* query = new QSqlQuery(mFiluDB);
  bool ok = query->prepare(sql);
  if(!ok)
  {
    fatal(FUNC, QString("Can't prepare sql '%1'.").arg(name));
    errInfo(FUNC, query->lastError().databaseText());
    return false;
  }

  mSQLs.insert(name, query);

  return true;
}

QString Filu::parseSql(const QString& name, const QString& rawSql)
{
  QString sql = rawSql;

  // Fix the schema and client place holder
  sql.replace(":schema", mFiluSchema);
  sql.replace(":dbuser", mRcFile->getST("PgUserRole"));
  sql.replace(":filu", mFiluSchema);
  sql.replace(":user", mUserSchema);

  // Extract all parameter ":foo" of the SQL
  StringSet parms;
  QRegExp rx("(:\\w+)");
  int pos = 0;
  while ((pos = rx.indexIn(rawSql, pos)) != -1)
  {
    parms.insert(rx.cap(1));
    pos += rx.matchedLength();
  }

  mSqlParmNames.insert(name, parms);

  if(verboseLevel(eAmple))
  {
    QStringList parmLst = parms.toList();
    verbose(FUNC, QString("SQL '%1' has parameters: %2").arg(name, parmLst.join(" ")));
    verbose(FUNC, sql);
  }

  return sql;
}

bool Filu::loadQuery(const QString& name, QString& sql)
{
  // Build the fulpath to the file where the sql is stored
  QString fileName(mSqlPath);
  fileName.append(name + ".sql");

  verbose(FUNC, fileName, eAmple);

  // Make sure we have no garbage in the statement
  sql.clear();

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    error(FUNC, tr("Can't open file '%1'.").arg(fileName));
    return false;
  }

  // Read/fill the statement
  QTextStream in(&file);
  while(!in.atEnd())
  {
    QString line = in.readLine();

    // Remove obsolet spaces
    QString helpLine = line.trimmed();

    if(helpLine.startsWith("*"))  continue; // Ignore remarks
    if(helpLine.startsWith("/*"))
    {
      // Ignore remarks of form /* comment */
      if(helpLine.contains("*/")) continue;

      while(!in.atEnd())
      {
        QString c = in.read(1);
        if(!c.startsWith("*")) continue;
        c = in.read(1);
        if(c.startsWith("/")) break;
      }
      continue;
    }
    if(helpLine.isEmpty()) continue;

    // Make sure it ends with a whitespace
    line.append("\n");

    // FIXME: Use a RegExp so that also remarks at the end of a line are cleaned
    //        and the from  /* comment */ is detected too
    if(helpLine.startsWith("--")) line.replace(":", " "); // Oh dear, could make nice trouble

    sql.append(line);
  }

  file.close();

  return true;
}

int Filu::execute(const QString& name, const QString& rawSql)
{
  // All needed parameter has to set previously by setSqlParm()

  QString sql = parseSql(name, rawSql);

  QSqlQuery* query = new QSqlQuery(mFiluDB);
  bool ok = query->prepare(sql);
  if(!ok)
  {
    fatal(FUNC, QString("Can't prepare sql '%1'.").arg(name));
    errInfo(FUNC, query->lastError().databaseText());
    return false;
  }

  // Set the parameter for the SQL, if some
  QSetIterator<QString> i(mSqlParmNames.value(name));
  while (i.hasNext())
  {
    QString parm = i.next();
    query->bindValue(parm, mSqlParm.value(parm));
    //qDebug() << "Filu::execSql:" << parm << mSqlParm.value(parm);
  }

  return execute(query);
}

int Filu::execute(QSqlQuery* query)
{
  query->exec();
  mLastQuery = query;

  // Save the query statement, anyway if error or not
  mExecSql = query->executedQuery();
  mLastError = query->lastError().databaseText();
  bool isError = !query->isActive();
  mLastResult = eError; // The glass is always half-empty

  if(verboseLevel(eMax)) // For heavy debuging print each sql
  {
    verbose(FUNC, QString("ExecutedQuery: %1").arg(mExecSql));
    verbose(FUNC, QString("DatabaseText: %1").arg(query->lastError().databaseText()));
    verbose(FUNC, QString("DriverText: %1").arg(query->lastError().driverText()));
    verbose(FUNC, QString("ErrorNo: %1").arg(query->lastError().number()));
    verbose(FUNC, QString("ErrorType: %1").arg(query->lastError().type()));
    verbose(FUNC, QString("QueryIsAktiv: %1").arg(query->isActive()));
    verbose(FUNC, QString("NumRowsAffected: %1").arg(query->numRowsAffected()));

    /*
    qDebug() << "BEGIN debug code for qt4.5 bug :-(";
    for(int i = 0; i < 6; ++i)
    {
      qDebug() << "boundValue at: " << i << query->boundValue(i);
    }

    QMapIterator<QString, QVariant> i(query->boundValues());
    while (i.hasNext())
    {
      i.next();
      qDebug() << "'" << i.key().toAscii().data() << "' = "
               << "'" << i.value().toString().toAscii().data() << "'";
    }

    QList<QVariant> list = query->boundValues().values();
    for (int i = 0; i < list.size(); ++i)
        qDebug() << i << ": " << list.at(i).toString().toAscii().data();

    qDebug() << "END debug code for qt4.5 bug :-(";
    */

    //if(isError) return eError;
  }

  if(isError)
  {
    fatal(FUNC, mLastError);
    errInfo(FUNC, tr("Executed query was '%1'.").arg(mSQLs.key(query)));
    return eError;
  }
  else if(verboseLevel(eAmple))
  {
    verbose(FUNC, tr("No error after execute query '%1'.").arg(mSQLs.key(query)));
  }

  if(query->isSelect())
  {
    if(query->size() < 0)
    {
      // Should never read this
      fatal(FUNC, "Size of Select SQL is -1");
      fatal(FUNC, mLastError);
      errInfo(FUNC, tr("Executed query was '%1'.").arg(mSQLs.key(query)));
      return eError;
    }
    else if(query->size() == 0)
    {
      //error(FUNC, "No data."); //FIXME Incomment after Newswire is improved
      mLastResult = eNoData;
      return eNoData;
    }
  }
  else
  {
    mLastResult = eNoSuccess;
    // FIXME Does not work, e.g. an INSERT sql doesn't say'Yes, inserted'
    // numRowsAffected() is always 0 :-(

//     if(query->numRowsAffected() < 0)
//     {
//       fatal(FUNC, "Number of rows affected < 0");
//       return eNoSuccess;
//     }
//     else if(query->numRowsAffected() == 0)
//     {
//       verbose(FUNC, "No rows affected by sql.", eAmple);
//       return eNoSuccess;
//     }
  }

  mLastResult = eSuccess;
  return eSuccess;
}

QSqlQuery* Filu::lastQuery()
{
  return mLastQuery;
}

int Filu::result(const QString& func, QSqlQuery* query)
{
  if(!query) return eExecError;
  if(lastResult() == eNoData)
  {
     //FIXME Incomment after Newswire is improved
    //clearMessages(); // remove the "No data." from execute(query)
    //error(func, "No data.");
    return eNoData;
  }

  query->next();
  mLastResult = query->value(0).toInt();
  if(mLastResult > eNoData) return mLastResult;

  int save = mLastResult;
  error(func, dbFuncErrText(mLastResult));
  mLastResult = save;

  return mLastResult;
}

void Filu::readSettings()
{
  QCoreApplication::addLibraryPath(mRcFile->getST("PluginPath"));
  mSqlPath = mRcFile->getST("SqlPath");
  mFiluSchema = mRcFile->getST("FiluSchema");
  mCommitBlockSize = mRcFile->getIT("CommitBlockSize");
  mDaysToFetchIfNoData = mRcFile->getIT("DaysToFetchIfNoData");

  setLogFile(/*FIXME:FUNC, */mRcFile->getST("LogFile"));
  setVerboseLevel(FUNC, mRcFile->getST("SqlDebug"));

  if(verboseLevel(eMax)) printSettings();
}

void Filu::printSettings()
{
  QString dbVersion = "Not connected";
  if(mFiluDB.isOpen())
  {
    execute("_DBVersion", "SELECT version()");
    if(mLastQuery->isActive())
    {
      mLastQuery->next();
      dbVersion = mLastQuery->value(0).toString();
    }
  }

  QString txt = "%1 = %2";
  int width = -20; // Negative value = left-aligned
  print("Filu settings are:");
  print(txt.arg("Using QtVersion", width).arg(qVersion()));
  print(txt.arg("Postgres version", width).arg(dbVersion));
  print(txt.arg("HostName", width).arg(mRcFile->getST("HostName")));
  print(txt.arg("HostPort", width).arg(mRcFile->getIT("HostPort")));
  print(txt.arg("PgUserRole", width).arg(mRcFile->getST("PgUserRole")));
  print(txt.arg("Password", width).arg(mRcFile->getST("Password")));
  print(txt.arg("DatabaseName", width).arg(mRcFile->getST("DatabaseName")));
  print(txt.arg("FiluSchema", width).arg(mFiluSchema));
  print(txt.arg("SqlPath ", width).arg(mSqlPath));
  print(txt.arg("CommitBlockSize", width).arg(mCommitBlockSize));
  print(txt.arg("DaysToFetchIfNoData", width).arg(mDaysToFetchIfNoData));
  print(txt.arg("SqlDebug", width).arg(verboseLevel()));
}
