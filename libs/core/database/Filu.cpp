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

#include "Filu.h"

#include "RcFile.h"

Filu::Filu(const QString& cn, RcFile* rcFile)
    : Newswire(FUNC)
    , mRcFile(rcFile)
    , mConnectionName(cn)
{
  mToDate = QDate::currentDate().toString(Qt::ISODate); // In case someone forget to set it...
}

Filu::~Filu()
{
  closeDB();
}

int Filu::setMarketName(const QString& name)
{
  const int retVal = searchCaption("market", name);

  if(retVal < eData) return retVal;

  setSqlParm(":marketId", retVal);
  setSqlParm(":market", name);

  return retVal;
}

int Filu::setSymbolCaption(const QString& caption)
{
  // Returns:
  //   >0 if no problem, (the FiId)
  //    0 if Symbol was not found
  //   -1 if Symbol was found more than one time and was associated to different FIs
  //   -2 if Symbol was empty
  //   -3 if any other error

  setSqlParm(":symbol", caption);

  if(!initQuery("GetFiIdBySymbol")) return eInitError;

  QSqlQuery* query = mSQLs.value("GetFiIdBySymbol");

  query->bindValue(":symbol", caption);

  if(execute(query) <= eError)  return eExecError;

  query->next();
  const int retVal = query->value(0).toInt();

  if(retVal >= eData)
  {
    setSqlParm(":fiId", retVal);
    return retVal;
  }
  if(retVal ==  0) error(FUNC, tr("Symbol '%1' not found.").arg(caption));
  if(retVal == -1) error(FUNC, tr("Symbol '%1' associated to different FIs.").arg(caption));
  if(retVal == -2) error(FUNC, tr("Symbol is empty."));

  return (eError + retVal);
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

  retVal = setSymbolCaption(symbol);
  if(retVal < eData) return 0;

  int fiId = retVal;

  retVal = setMarketName(market);
  if(retVal < eData) return 0;

  return getBars(fiId, retVal, fromDate, toDate);
}

BarTuple* Filu::getBars(int fiId, int marketId
                      , const QString& fromDate/* = "1000-01-01"*/
                      , const QString& toDate/* = "3000-01-01"*/)
{
  if(!initQuery("GetBars")) return 0;

  QSqlQuery* query = mSQLs.value("GetBars");

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

BarTuple* Filu::getBars(const QString& symbol, const QString& market, int limit)
{
  int retVal;

  retVal = setSymbolCaption(symbol);
  if(retVal < eData) return 0;

  int fiId = retVal;

  retVal = setMarketName(market);
  if(retVal < eData) return 0;

  return getBars(fiId, retVal, limit);
}

BarTuple* Filu::getBars(int fiId, int marketId, int limit)
{
  if(!limit) return 0;

  if(!initQuery("GetBarsLtd")) return 0;

  QSqlQuery* query = mSQLs.value("GetBarsLtd");

  query->bindValue(":fiId", fiId);
  query->bindValue(":marketId", marketId);
  query->bindValue(":limit", limit);

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

SymbolTuple* Filu::getSymbols(int fiId)
{
  return getSymbols(fiId, "", "", "", "", false);
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
  if(!initQuery("GetMarketByName")) return 0;

  QSqlQuery* query = mSQLs.value("GetMarketByName");

  query->bindValue(":market", name);

  if(execute(query) < eData) return 0;

  // Fill the object to be returned to client
  MarketTuple* market = new MarketTuple(query->size());
  while(market->next())
  {
    query->next();

    int i = market->mIndex;
    market->mId[i]       = query->value(0).toInt();
    market->mName[i]     = query->value(1).toString();
    market->mCurrency[i] = query->value(2).toString();
    //market->mType[i]     = query->value(3).toString();
  }
  market->rewind();

  return market;
}

FiTuple* Filu::getFi(const int fiId)
{
  // Not yet used, but soon...I hope
  if(!initQuery("GetFi")) return 0;

  QSqlQuery* query = mSQLs.value("GetFi");

  query->bindValue(":name", "");
  query->bindValue(":type", "");
  query->bindValue(":fuzzy", false);
  query->bindValue(":fiId", fiId);

  if(execute(query) < eData) return 0;

  return fillFiTuple(query);
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
  if(!initQuery("GetFiBySymbol")) return 0;

  QSqlQuery* query = mSQLs.value("GetFiBySymbol");

  query->bindValue(":symbol", symbol);

  if(execute(query) < eData) return 0;

  return fillFiTuple(query);
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

  QSqlQuery* query = new QSqlQuery(QSqlDatabase::database(mConnectionName));

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

int Filu::searchCaption(const QString& table, const QString& caption)
{
  // The functionality is done by the database function
  // called by GetIdByCaption.sql.

  if(!initQuery("GetIdByCaption")) return eInitError;

  QSqlQuery* query = mSQLs.value("GetIdByCaption");

  query->bindValue(":table", table);
  query->bindValue(":caption", caption);

  if(execute(query) <= eError) return eExecError;

  query->next();
  const int retVal = query->value(0).toInt();

  if(retVal >= eData) return retVal;

  QString errParm = QString(" Table: %1, Caption: %2").arg(table).arg(caption);

  if(retVal ==  0) errInfo(FUNC, tr("Caption '%1' not found.").arg(errParm));
  if(retVal == -1) warning(FUNC, tr("Caption '%1' is more than one times in table '%2'.").arg(errParm, table));
  if(retVal == -2) errInfo(FUNC, tr("Caption was empty."));

  return (eError + retVal);
}

int Filu::addSymbolType(const QString& type
                       , const int& seq
                       , const bool isProvider
                       , const int& id/* = 0*/)
{
  // Add or update a symbol type
  if(!initQuery("AddSymbolType")) return eInitError;

  QSqlQuery* query = mSQLs.value("AddSymbolType");

  query->bindValue(":symbolType", type);
  query->bindValue(":seq", seq);
  query->bindValue(":isProvider", isProvider);
  query->bindValue(":stypeId", id);

  if(execute(query) <= eError) return eExecError;

  query->next();
  const int retVal = query->value(0).toInt();

  if(retVal >= eData) return retVal;

  if(retVal == -1) errInfo(FUNC, tr("Type was empty."));

  return (eError + retVal);
}

int Filu::addMarket(const QString& market
                  , const QString& currency
                  , const QString& currSymbol)
{
  if(!initQuery("AddMarket")) return eInitError;

  QSqlQuery* query = mSQLs.value("AddMarket");

  query->bindValue(":market", market);
  query->bindValue(":currencyName", currency);
  query->bindValue(":currencySymbol", currSymbol);

  if(execute(query) <= eError)
  {
    error(FUNC, tr("Error while add market: %1, %2, %3").arg(market, currency, currSymbol));
    return eExecError;
  }

// FIXME: uncomment when DB function market_insert is modified to return
//        the id or error like the other insert functions
//   query->next();
//
//   return query->value(0).toInt();

  return 1;
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

  QTextStream console(stdout);
//console << "Filu::addEODBarData\n";
  QTime time;
  time.restart();

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

  QSqlDatabase::database(mConnectionName).transaction();
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
      QSqlDatabase::database(mConnectionName).rollback();
      error(FUNC, tr("Error while add EODBar with date '%1'.").arg(values[0]));
      if(barCount) errInfo(FUNC, tr("%1 bars previus added without trouble.").arg(barCount));
      return eExecError;
    }

    if(sqlExecCounter == mCommitBlockSize)
    {
      QSqlDatabase::database(mConnectionName).commit();
      QSqlDatabase::database(mConnectionName).transaction();

      console << ".";

      sqlExecCounter = 0;
    }
    ++barCount;
    j += increment;
  }
 // if(sqlExecCounter > 0)
      QSqlDatabase::database(mConnectionName).commit();

   console /*<< "Filu::addEODBarData: " */<< barCount << " bars added in "
           << time.elapsed() << " ms\n" << flush;

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

    retVal = setSymbolCaption(symbol->caption());
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

    if(retVal < eSuccess) return (eError + retVal);

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
  //
  // AddFi.sql Returns:
  //   >0 if all is fine, the fiId
  //   -1 if fiType is not valid
  //   -2 if fiName was empty
  //   -3 if symbol is empty
  //   -4 if stype is not valid
  //   -5 if market is not valid
  //   -6 if unique violation
  //   -7 if symbol was found more than one time and was associated to different FIs
  //   -8 if foreign key violation (should impossible?)
  //   -9 if other error (should impossible?)

  clearErrors();

  if(name.isEmpty())
  {
    error(FUNC, tr("FI Name is empty."));
    return (eError -2);
  }
  if(type.isEmpty())
  {
    error(FUNC, tr("FI Type  is empty."));
    return (eError -1);
  }

  if(!initQuery("AddFi")) return eInitError;

  QSqlQuery* query = mSQLs.value("AddFi");

  query->bindValue(":fiId", fiId);
  query->bindValue(":name", name);
  query->bindValue(":fType", type);
  query->bindValue(":symbol", symbol);
  query->bindValue(":market", market);
  query->bindValue(":sType", stype);

  if(execute(query) <= eError) return eExecError;

  query->next();
  const int retVal = query->value(0).toInt();

  if(retVal >= eData) return retVal;

  QString errParm = QString(" FI: %1, %2 Symbol: %3, %4, %5").arg(name, type, symbol, market, stype);

  if(retVal == -1) error(FUNC, tr("FiType '%1' unknown.").arg(type));
  if(retVal == -3) error(FUNC, tr("FI not known and Symbol was empty: %1").arg(errParm));
  if(retVal == -4) error(FUNC, tr("Symbol Type '%1' unknown.").arg(stype));
  if(retVal == -5) error(FUNC, tr("Market '%1' unknown.").arg(market));
  if(retVal == -6) error(FUNC, tr("Unique violation: %1").arg(errParm));
  if(retVal == -7) error(FUNC, tr("Symbol was found more than one time and was associated to different FI: %1").arg(errParm));
  if(retVal == -8) error(FUNC, tr("Foreign Key Violation: %1").arg(errParm));
  if(retVal == -9) error(FUNC, tr("Error, No idea what's wrong: %1").arg(errParm));

  return (eError + retVal);
}

int Filu::addSymbol(const QString& symbol
                  , const QString& market
                  , const QString& stype
                  , int fiId/* = 0*/
                  , int symbolId/* = 0*/)
{
  // Returns:
  //   >0 the symbolId, if Symbol exist
  //    0 if Symbol looks good but no fiId was given to add them
  //   -1 SymbolType not valid
  //   -2 Market not valid
  //   -3 unique violation
  //   -4 foreign key violation
  //   -5 other error

  if(!initQuery("AddSymbol")) return eInitError;

  QSqlQuery* query = mSQLs.value("AddSymbol");

  query->bindValue(":fiId", fiId);
  query->bindValue(":symbolId", symbolId);
  query->bindValue(":caption", symbol);
  query->bindValue(":market", market);
  query->bindValue(":sType", stype);

  if(execute(query) <= eError) return eExecError;

  query->next();
  const int retVal = query->value(0).toInt();

  if(retVal >= eData) return retVal;

  QString errParm = QString(" %1, %2, %3").arg(symbol, market, stype);

  if(retVal ==  0) error(FUNC, tr("Symbol looks good, but got no FiId: %1").arg(errParm));
  if(retVal == -1) error(FUNC, tr("Symbol Type '%1' unknown.").arg(stype));
  if(retVal == -2) error(FUNC, tr("Market '%1' unknown.").arg(market));
  if(retVal == -3) error(FUNC, tr("FI has already a Symbol with that Market and Provider: %1").arg(errParm));
  if(retVal == -4) error(FUNC, tr("Foreign Key violation: %1").arg(errParm));

  return (eError + retVal);
}


int Filu::addUnderlying(const QString& mother
                      , const QString& symbol
                      , const double weight)
{
  // Add all underlying FIs holded as symbols in FiTuple.
  // We don't delete all underlyings previous because it fit more to
  // all other addFoo functions.
  // To remove all existing underlyings call execSql("DelAllUnderlyingsFromMother");

  if(!initQuery("AddUnderlying")) return eInitError;

  QSqlQuery* query = mSQLs.value("AddUnderlying");

  query->bindValue(":motherSymbol", mother);
  query->bindValue(":symbol", symbol);
  query->bindValue(":weight", weight);

  if(execute(query) <= eError) return eExecError;

  query-> next();
  const int retVal = query->value(0).toInt();

  if(retVal == eSuccess) return eSuccess;

  QString errParm = QString(" Mother: %1, Underlying: %2").arg(mother).arg(symbol);

  if(retVal == -1) error(FUNC, tr("MotherSymbol not found."));
  if(retVal == -2) error(FUNC, tr("MotherSymbol exist more than one times."));
  if(retVal == -3) error(FUNC, tr("UnderlyingSymbol not found."));
  if(retVal == -4) error(FUNC, tr("UnderlyingSymbol exist more than one times."));

  return (eError + retVal);
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

  query->next();
  const int retVal = query->value(0).toInt();

  if(retVal >= eData) return retVal; // (new or existing)SplitId

  return (eError + retVal);
}

QString Filu::getLastQuery()
{
  return mLastQuery;
}

void Filu::deleteRecord(const QString& schema, const QString& table, int id /*= -1*/)
{
  QString sql;

  if(id == -1)
  {
    sql = QString("DELETE FROM %1.%2 ").arg(schema).arg(table);
  }
  else
  {
    sql = QString("DELETE FROM %1.%2 WHERE %2_id = %3").arg(schema).arg(table).arg(id);
  }

  sql.replace(":filu", schema);
  sql.replace(":user", mUserSchema);
  //qDebug() << "sql=" << sql;

  QSqlQuery query(QSqlDatabase::database(mConnectionName));
  query.prepare(sql);
  query.exec();
}

int Filu::getNextId(const QString& schema, const QString& table)
{
  QString sql;

  sql = QString("SELECT nextval('%1.%2_%2_id_seq')").arg(schema).arg(table);
  sql.replace(":filu", schema);
  sql.replace(":user", mUserSchema);
//qDebug() << "Filu::getNextId, sql=" << sql;
  QSqlQuery query(QSqlDatabase::database(mConnectionName));
  query.prepare(sql);

  if(!query.exec()) return 0;

  query.next();
//qDebug() << "neueid=" << query.value(0).toULongLong();
  return query.value(0).toULongLong();
}

void Filu::openDB()
{
  readSettings();
  QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", mConnectionName);
  db.setHostName(mRcFile->getST("HostName"));
  db.setPort(mRcFile->getIT("HostPort"));
  db.setDatabaseName(mRcFile->getST("DatabaseName"));
  db.setUserName(mRcFile->getST("UserName"));
  db.setPassword(mRcFile->getST("Password"));
  bool ok = db.open();
  if(!ok)
  {
    QSqlError err = db.lastError();
    fatal(FUNC, tr("Can't open DB."));
    errInfo(FUNC, err.databaseText());
    printSettings();
  }
  else
  {
    verbose(FUNC, "Successful connected to Filu :-)");
  }
}

void Filu::closeDB()
{
  QHashIterator<QString, QSqlQuery*> i(mSQLs);
  while(i.hasNext()) delete i.next().value();

  QSqlDatabase::database(mConnectionName).close();
  QSqlDatabase::removeDatabase(mConnectionName);
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

bool Filu::initQuery(const QString& name)
{
  clearErrors();

  verbose(FUNC, name/*, eMax*/);

  if(mSQLs.contains(name)) return true;

  QString sql;

  if(!readSqlStatement(name, sql)) return false;

  QSqlQuery* query = new QSqlQuery(QSqlDatabase::database(mConnectionName));

  bool ok = query->prepare(sql);
  if(!ok)
  {
    fatal(FUNC, tr("Can't prepare sql '%1'.").arg(name));
    QSqlError err = query->lastError();
    errInfo(FUNC, err.databaseText());

    return false;
  }
  //else qDebug() << "Filu::initQuery: okay...";// << sql;

  mSQLs.insert(name, query);

  return true;
}

bool Filu::readSqlStatement(const QString& name, QString& sqlStatement)
{
  // Build the fulpath to the file where the sql is stored
  QString fileName(mSqlPath);
  fileName.append(name + ".sql");

  verbose(FUNC, fileName, eMax);

  // Make sure we have no garbage in the statement
  sqlStatement.clear();

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    error(FUNC, tr("Can't open file '%1'.").arg(fileName));
    return false;
  }

  // Read/fill the statement
  QTextStream in(&file);
  StringSet parameters; // Collect the parameter of the SQL
  while (!in.atEnd())
  {
    QString line = in.readLine();

    // Remove obsolet spaces
    line = line.trimmed();

    if(line.startsWith("*")) continue; // Ignore remarks
    if(line.isEmpty()) continue;

    // Make sure it ends with a whitespace
    line.append("\n");

    if(line.startsWith("--")) line.replace(":", " "); // Oh dear, could make nice trouble

    // Fix the schema and client place holder
    line.replace(":filu", mFiluSchema);
    line.replace(":user", mUserSchema);

    // Extract all parameter ":foo" of the SQL
    QRegExp rx("(:\\w+)");
    int pos = 0;
    while ((pos = rx.indexIn(line, pos)) != -1)
    {
      parameters.insert(rx.cap(1));
      pos += rx.matchedLength();
    }

    sqlStatement.append(line);
  }

  if(verboseLevel() == eMax)
  {
    QString parms;
    foreach(QString parm, parameters.toList()) parms.append(parm + " ");
    verbose(FUNC, QString("Sql '%1' has parameters: %2").arg(name, parms));
  }

  mSqlParmNames.insert(name, parameters);

  file.close();

  return true;
}

int Filu::execute(QSqlQuery* query)
{
  query->exec();

  // Save the query statement, anyway if error or not
  mLastQuery = query->executedQuery();
  mLastError = query->lastError().databaseText();
  bool isError = !query->isActive();

  if(verboseLevel() == eMax) // For heavy debuging print each sql
  {
    verbose(FUNC, QString("ExecutedQuery: %1").arg(mLastQuery));
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

    if(isError) return eError;
  }

  if(isError)
  {
    //verbose(FUNC, tr("Executed query was: %1").arg(mLastQuery));
    error(FUNC, tr("Error text: %1").arg(mLastError));
    return eError;
  }

  if(query->isSelect())
  {
    if(query->size() < 0)
    {
      // Should never read this
      fatal(FUNC, "Size of Select SQL is -1");
      fatal(FUNC, mLastError);
      return eError;
    }
    else if(query->size() == 0)
    {
      return eNoData;
    }
  }
  else
  {
    if(query->numRowsAffected() < 0)
    {
      fatal(FUNC, "Number of rows affected < 0");
      return eNoSuccess;
    }
    else if(query->numRowsAffected() == 0)
    {
      errInfo(FUNC, "No rows affected by sql.");
      return eNoSuccess;
    }
  }

  return eSuccess;
}

void Filu::readSettings()
{
  mSqlPath = mRcFile->getST("SqlPath");
  mFiluSchema = mRcFile->getST("FiluSchema");
  mCommitBlockSize = mRcFile->getIT("CommitBlockSize");
  mDaysToFetchIfNoData = mRcFile->getIT("DaysToFetchIfNoData");

  setLogFile(/*FIXME:FUNC, */mRcFile->getST("LogFile"));
  setVerboseLevel(FUNC, mRcFile->getST("SqlDebugLevel"));

  if(verboseLevel() == eMax) printSettings();
}

void Filu::printSettings()
{
  QString txt = "%1 = %2";
  int width = -15; // Negative value = left-aligned
  print("Filu settings are:");
  print(txt.arg("SqlPath ", width).arg(mSqlPath));
  print(txt.arg("HostName", width).arg(mRcFile->getST("HostName")));
  print(txt.arg("HostPort", width).arg(mRcFile->getIT("HostPort")));
  print(txt.arg("DatabaseName", width).arg(mRcFile->getST("DatabaseName")));
  print(txt.arg("FiluSchema", width).arg(mFiluSchema));
  print(txt.arg("UserName", width).arg(mRcFile->getST("UserName")));
  print(txt.arg("Password", width).arg(mRcFile->getST("Password")));
  print(txt.arg("CommitBlockSize", width).arg(mCommitBlockSize));
  print(txt.arg("DaysToFetchIfNoData", width).arg(mDaysToFetchIfNoData));
  print(txt.arg("SqlDebugLevel", width).arg(verboseLevel()));
}
