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

#include "Exporter.h"
#include "FTool.h"

using namespace std;

Exporter::Exporter(FClass* parent)
        : FClass(parent)
        , mOutput(0)
        , mConsole(stdout)
        , mOutFile(0)
{}


// Exporter::Exporter(FiluU* filu) : FClass(filu)
// {}

Exporter::~Exporter()
{
  if(mOutput)
  {
    mOutput->flush();
    delete mOutput;
  }

  if(mOutFile)
  {
    mOutFile->close();
    delete mOutFile;
  }

}

bool Exporter::exxport(QStringList& command)
{
  mCommandLine = command;

  QStringList paramater;

  // open output file, if needed
  if(getParameter("--toFile", paramater) > 0)
  {
    mOutFile = new QFile(paramater.at(0));

    QFlags<QIODevice::OpenModeFlag> mode = QIODevice::Text | QIODevice::WriteOnly;

    if(paramater.contains("append"))
    {
      mode = QIODevice::Text | QIODevice::WriteOnly | QIODevice::Append;
    }
    else
    {
      mode = QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate;
    }

    if(!mOutFile->open(mode))
    {
      qDebug() << "can't open file";
      return false;
    }
  }
  else
  {
    //no outfile spezified, we write to stdout
    mOutFile = new QFile;
    if(!mOutFile->open(stdout, QIODevice::Text | QIODevice::WriteOnly))
    {
      qDebug() << "can't open stdout"; // could that happens? don't think so'
      return false;
    }
  }

  mOutput = new QTextStream(mOutFile);

  // let us print some info at the beginning
  *mOutput << "***\n";
  *mOutput << "*\n";
  *mOutput << "* This file was generated by Filu at " << QDate::currentDate().toString(Qt::ISODate) << "\n*\n" ;
  *mOutput << "* Given export paramaters were: " << command.join(" ") << "\n";
  *mOutput << "*\n";
  *mOutput << "* Data are filtered and grouped to given FI type and/or symbol type\n";
  *mOutput << "* and/or market and/or group membership.\n";
  *mOutput << "*" << endl;

  // look for filter settings
  if(getParameter("--setFiType", paramater) > 0) mFilu->setSqlParm(":ftype", paramater.at(0));
  else mFilu->setSqlParm(":ftype", "");

  if(getParameter("--setGroup", paramater) > 0) mFilu->setSqlParm(":group", paramater.at(0));
  else mFilu->setSqlParm(":group", "");

  if(getParameter("--setMarket", paramater) > 0) mFilu->setSqlParm(":market", paramater.at(0));
  else mFilu->setSqlParm(":market", "");

  if(getParameter("--setProvider", paramater) > 0) mFilu->setSqlParm(":provider", paramater.at(0));
  else mFilu->setSqlParm(":provider", "");

  // look for export commands, and execute them if found
  if(mCommandLine.contains("--fiTypes"))      if(!expFiTypes()) return false;
  if(mCommandLine.contains("--symbolTypes"))  if(!expSymbolTypes()) return false; // aka provider
  if(mCommandLine.contains("--markets"))      if(!expMarkets()) return false;
//   if(mCommandLine.contains("--offdays")) ; // should always done when -markets given
//   if(mCommandLine.contains("--marketSymbols")) ;
  if(mCommandLine.contains("--fiNames"))      if(!expFiNames()) return false;
  if(mCommandLine.contains("--symbols"))      if(!expSymbols()) return false;
  if(mCommandLine.contains("--eodRaw"))       if(!expEODRaw())  return false;
//   if(mCommandLine.contains("--eodAdjusted")) ;
  if(mCommandLine.contains("--splits"))       if(!expSplits())  return false; // what if -eodAdjusted? after reimport we have a problem
//   if(mCommandLine.contains("--underlyings")) ;
  if(mCommandLine.contains("--co"))           if(!expCOs())     return false;
  if(mCommandLine.contains("--groups"))       if(!expGroups())  return false;;
//   if(mCommandLine.contains("--portfolio")) ;
//   if(mCommandLine.contains("--tstrategy")) ;
//   if(mCommandLine.contains("")) ;

  return true;

}

int Exporter::getParameter(const QString& command, QStringList& parm)
{
  // place the parameter to the command switch "--foo" into parm
  // FIXME: see FTool
  int pos = mCommandLine.indexOf(command);

  if(-1 == pos) return -1; // command not found. was not given on command line

    parm.clear();          // be on the save side

    for(int i = pos + 1; i <= mCommandLine.size() - 1; ++i)
    {
      if(mCommandLine.at(i).startsWith("--")) break;

      parm.append(mCommandLine.at(i));
    }

    return parm.size();
}

bool Exporter::expFiTypes()
{
  mConsole << "* Exporter::expFiTypes:..." << flush;

  QSqlQuery* query = mFilu->execSql("GetAllFiTypes");

  if(!query)
  {
    if(check4FiluError("Exporter::expFiTypes: ERROR while exec GetAllFiTypes.sql")) return false;

    mConsole << " no FI types match settings." << endl;
    *mOutput << "* No FI types match settings." << endl;
    return true;
  }

  // the query result has the format
  // FiType
  // see also GetAllFiTypes.sql

  *mOutput << "***\n";
  *mOutput << "*\n";
  *mOutput << "* All FI types.\n";
  *mOutput << "*\n";

  *mOutput << "[Header]FiType\n\n";

  while(query->next())
  {
    *mOutput << query->value(0).toString() << "\n";
  }

  *mOutput << endl;

  mConsole << "done." << endl;

  return true;
}

bool Exporter::expSymbolTypes()
{
  mConsole << "* Exporter::expSymbolTypes:..." << flush;

  QSqlQuery* query = mFilu->execSql("GetAllSymbolTypes");

  if(!query)
  {
    if(check4FiluError("Exporter::expSymbolTypes: ERROR while exec GetAllSymbolTypes.sql")) return false;

    mConsole << "no symbol types match settings." << endl;
    *mOutput << "* No symbol types match settings." << endl;
    return true;
  }

  // the query result has the format
  // SymbolType, SEQ, IsProvider
  // see also GetAllSymbolTypes.sql

  *mOutput << "***\n";
  *mOutput << "*\n";
  *mOutput << "* All symbol types.\n";
  *mOutput << "*\n";

  *mOutput << "[Header]SymbolType;SEQ;IsProvider\n\n";

  while(query->next())
  {
    *mOutput << query->value(0).toString() << ";";
    *mOutput << query->value(1).toString() << ";";
    *mOutput << query->value(2).toBool() << "\n";
  }

  *mOutput << endl;

  mConsole << "done." << endl;

  return true;
}

bool Exporter::expMarkets()
{
  mConsole << "* Exporter::expMarkets:..." << flush;

  QSqlQuery* query = mFilu->execSql("GetAllMarkets");

  if(!query)
  {
    if(check4FiluError("Exporter::expMarkets: ERROR while exec GetAllMarkets.sql")) return false;

    mConsole << "no markets match settings." << endl;
    *mOutput << "* No markets match settings." << endl;
    return true;
  }

  // the query result has the format
  // MarketId, Market, OpenTime, CloseTime, Currency, CurrencySymbol
  // see also GetAllMarkets.sql

  *mOutput << "***\n";
  *mOutput << "*\n";
  *mOutput << "* All markets with currency and currency symbol.\n";
  *mOutput << "*\n";

  *mOutput << "[Header]Market;OpenTime;CloseTime;Currency;CurrencySymbol\n\n";

  while(query->next())
  {
    // skip marketId, query->value(0).toString();
    *mOutput << query->value(1).toString() << ";";
    *mOutput << query->value(2).toTime().toString(Qt::TextDate) << ";";
    *mOutput << query->value(3).toTime().toString(Qt::ISODate) << ";";
    *mOutput << query->value(4).toString() << ";";
    *mOutput << query->value(5).toString() << ";\n";
  }

  *mOutput << endl;

  mConsole << "done." << endl;

  return true;
}

bool Exporter::expFiNames()
{
  mConsole << "* Exporter::expFiNames:..." << flush;

  QSqlQuery* query = mFilu->execSql("GetAllFi");

  if(!query)
  {
    if(check4FiluError("Exporter::expFiNames: ERROR while exec GetAllFi.sql")) return false;

    mConsole << "no FIs match settings." << endl;
    *mOutput << "* No FIs match settings." << endl;
    return true;
  }

  // the query result has the format
  // Name, Type, DDate, Symbol, Provider, Market, IDate, MDate
  // see also GetAllFi.sql

  *mOutput << "***\n";
  *mOutput << "*\n";
  *mOutput << "* All Fi names with full lovely symbol set as referenz.\n";
  *mOutput << "*\n";

  *mOutput << "[Header]Name;Type;DDate;Symbol;Provider;Market;IDate;MDate\n";

  QString name, type, ddate, symbol, provider, market, idate, mdate;

  bool separator = false;

  while(query->next())
  {
    if(type != query->value(1).toString()) separator = true;
    else if(provider != query->value(4).toString()) separator = true;
    else if(market != query->value(5).toString()) separator = true;

    int i = 0;
    name      = query->value(i++).toString();
    type      = query->value(i++).toString();
    ddate     = query->value(i++).toDate().toString(Qt::ISODate);
    symbol    = query->value(i++).toString();
    provider  = query->value(i++).toString();
    market    = query->value(i++).toString();
    idate     = query->value(i++).toDate().toString(Qt::ISODate);
    mdate     = query->value(i++).toDate().toString(Qt::ISODate);

    if(separator)
    {
      *mOutput << "*\n";
      *mOutput << "* Type:" << type << "  Provider:" << provider << "  Market:" << market << "\n";
      *mOutput << "*\n";

      separator = false;
    }

    *mOutput << name << ";" << type << ";" << ddate << ";";
    *mOutput << symbol << ";" << provider << ";" << market << ";" << idate << ";" << mdate << "\n";
  }

  *mOutput << endl;

  mConsole << "done." << endl;

  return true;
}

bool Exporter::expSymbols()
{
  mConsole << "* Exporter::expSymbols:..." << flush;

  QSqlQuery* query = mFilu->execSql("GetAllSymbols");

  if(!query)
  {
    if(check4FiluError("Exporter::expSymbols: ERROR while exec GetAllSymbols.sql")) return false;

    mConsole << "no symbols match settings." << endl;
    *mOutput << "* No symbols match settings." << endl;
    return true;
  }

  // the query result has the format
  // RefSymbol, FiType, Symbol, Provider, Market, IDate, MDate
  // see also GetAllSymbols.sql.sql

  *mOutput << "***\n";
  *mOutput << "*\n";
  *mOutput << "* All symbols with leading lovely symbol as referenz.\n";
  *mOutput << "*\n";

  *mOutput << "[Header]RefSymbol;Symbol;IDate;MDate\n";

  QString refSymbol, type, symbol, provider, market, idate, mdate;

  bool separator = false;

  while(query->next())
  {
    if(type != query->value(1).toString()) separator = true;
    else if(provider != query->value(3).toString()) separator = true;
    else if(market != query->value(4).toString()) separator = true;

    int i = 0;
    refSymbol = query->value(i++).toString();
    type      = query->value(i++).toString();
    symbol    = query->value(i++).toString();
    provider  = query->value(i++).toString();
    market    = query->value(i++).toString();
    idate     = query->value(i++).toDate().toString(Qt::ISODate);
    mdate     = query->value(i++).toDate().toString(Qt::ISODate);

    if(separator)
    {
      *mOutput << "*\n";
      *mOutput << "* Type:" << type << "  Provider:" << provider << "  Market:" << market << "\n";
      *mOutput << "*\n";
      *mOutput << "[Type]" <<  type << "\n";
      *mOutput << "[Provider]" << provider  << "\n";
      *mOutput << "[Market]" << market  << "\n";
      *mOutput << "\n";
      separator = false;
    }

    *mOutput << refSymbol << ";" << symbol << ";" << idate << ";" << mdate << "\n";
  }

  *mOutput << endl;

  mConsole << "done." << endl;

  return true;
}

bool Exporter::expEODRaw()
{
  mConsole << "* Exporter::expEODRaw:...select, could take a while..." << flush;

  // FIXME: maybe it is better to select the eod data not all at once,
  //        but each fi once the other because it take too much time to do the
  //        select with the hugh amount of data. also, is it possible to run
  //        into a out of memory problem?

  QSqlQuery* query = mFilu->execSql("GetAllEODRawData");

  if(!query)
  {
    if(check4FiluError("Exporter::expEODRaw: ERROR while exec .sql")) return false;

    mConsole << "no eod bar data match settings." << endl;
    *mOutput << "* No eod bar data match settings." << endl;
    return true;
  }

  // the query result has the format
  // Name, Type, DDate, Symbol, Provider, Market, IDate, MDate
  // see also GetAllFi.sql

  *mOutput << "***\n";
  *mOutput << "*\n";
  *mOutput << "* EOD raw data\n";
  *mOutput << "*\n";

  *mOutput << "[Header]RefSymbol;Market;Date;Open;High;Low;Close;Volume;OpenInterest;Quality\n";

  QString market;

  bool separator = false;

  mConsole << "write data..." << flush;

  while(query->next())
  {
    if(market != query->value(1).toString()) separator = true;

    if(separator)
    {
      market = query->value(1).toString();

      *mOutput << "*\n";
      *mOutput << "* Market:" << market << "\n";
      *mOutput << "*\n";

      separator = false;
    }

    *mOutput << query->value(0).toString() << ";"; // ref symbol
    *mOutput << query->value(1).toString() << ";"; // market
    *mOutput << query->value(2).toDate().toString(Qt::ISODate) << ";";
    *mOutput << query->value(3).toDouble() << ";"; // open
    *mOutput << query->value(4).toDouble() << ";";
    *mOutput << query->value(5).toDouble() << ";";
    *mOutput << query->value(6).toDouble() << ";";
    *mOutput << query->value(7).toDouble() << ";"; // volume
    *mOutput << query->value(8).toDouble() << ";"; // open interest
    *mOutput << query->value(9).toInt() << ";\n";  // quality
  }

  *mOutput << endl;

  mConsole << "done."/* << endl*/;

  return true;
}

bool Exporter::expSplits()
{
  mConsole << "* Exporter::expSplits:...";

  QSqlQuery* query = mFilu->execSql("GetAllSplits");

  if(!query)
  {
    if(check4FiluError("Exporter::expSplits: ERROR while exec GetAllSplits.sql")) return false;

    mConsole << "no split data match settings." << endl;
    *mOutput << "* No split data match settings." << endl;
    return true;
  }

  // the query result has the format
  // RefSymbol, SpliDate, SplitComment, SplitRatio
  // see also GetAllSplits.sql

  *mOutput << "***\n";
  *mOutput << "*\n";
  *mOutput << "* Split data\n";
  *mOutput << "*\n";

  *mOutput << "[Header]RefSymbol;SplitDate;SplitComment;SplitRatio\n\n";

  while(query->next())
  {
    *mOutput << query->value(0).toString() << ";"; //
    *mOutput << query->value(1).toDate().toString(Qt::ISODate) << ";";
    *mOutput << query->value(2).toString() << ";"; //
    *mOutput << query->value(3).toDouble() << "\n"; //
  }

  *mOutput << endl;

  mConsole << "done." << endl;

  return true;
}

bool Exporter::expCOs()
{
  mConsole << "* Exporter::expCOs:...";

  QSqlQuery* query = mFilu->execSql("GetAllCOs");

  if(!query)
  {
    if(check4FiluError("Exporter::expCOs: ERROR while exec GetCOs.sql")) return false;

    mConsole << "no chart objects match settings." << endl;
    *mOutput << "* No chart objects match settings." << endl;
    return true;
  }

  // the query result has the format
  // RefSymbol, Market, CODate, Plot, Type, ArgV

  *mOutput << "***\n";
  *mOutput << "*\n";
  *mOutput << "* Chart object data\n";
  *mOutput << "*\n";

  *mOutput << "[Header]RefSymbol;Market;CODate;Plot;Type;ArgV\n\n";

  while(query->next())
  {
    int i = 0;
    *mOutput << query->value(i++).toString() << ";";  // RefSymbol
    *mOutput << query->value(i++).toString() << ";";  // Market
    *mOutput << query->value(i++).toDate().toString(Qt::ISODate) << ";";
    *mOutput << query->value(i++).toString() << ";";  // Plot
    *mOutput << query->value(i++).toString() << ";";  // Type

    // convert before export
    *mOutput << FTool::txtToLine(query->value(i++).toString()) << "\n";
  }

  *mOutput << endl;

  mConsole << "done." << endl;

  return true;
}

bool Exporter::expGroups()
{
  mConsole << "* Exporter::expGroups:...";

  mFilu->setSqlParm(":motherId", 0);

  QSqlQuery* query = mFilu->execSql("GetAllGroups");

  if(!query)
  {
    if(check4FiluError("Exporter::expGroups: ERROR while exec GetAllGroups.sql")) return false;

    mConsole << "no groups match settings." << endl;
    *mOutput << "* No groups match settings." << endl;
    return true;
  }

  // the query result has the format
  // RefSymbol, Market, CODate, Plot, Type, ArgV

  *mOutput << "***\n";
  *mOutput << "*\n";
  *mOutput << "* Group data\n";
  *mOutput << "*\n";

  *mOutput << "[Header]RefSymbol\n\n";

  while(query->next())
  {
    int gid = query->value(0).toInt();
    mFilu->setSqlParm(":groupId", gid);
    QSqlQuery* query = mFilu->execSql("GetGMembers");

    if(!query)
    {
      if(check4FiluError("Exporter::expGroups: ERROR while exec GetAllGroups.sql")) return false;
    }
    else
    {

      QSqlQuery* query2 = mFilu->execSql("GetGroupPathById");
      query2->next();
      QString path =  query2->value(0).toString();

      *mOutput << "*\n";
      *mOutput << "* Group members of " << path << "\n";
      *mOutput << "*\n";
      *mOutput << "[GroupPath]" << path << "\n\n";

      while(query->next())
      {
        *mOutput << query->value(2).toString() << "\n"; // symbol
      }
    }

  }

  *mOutput << endl;

  mConsole << "done." << endl;

  return true;
}