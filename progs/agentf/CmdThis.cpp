//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
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

#include <QDate>

#include "CmdThis.h"

#include "CmdHelper.h"
#include "FiluU.h"
#include "Scanner.h"
#include "Script.h"
#include "SymbolTuple.h"

static const QString cCmd1 = "this";
static const QString cCmd1Brief = QObject::tr("Download eod bars of one defined FI");

CmdThis::CmdThis(AgentF* agent)
       : CmdClass(agent, FUNC)
       , mScanner(0)
       , mScript(0)
{}

CmdThis::~CmdThis()
{
  if(mScanner)  delete mScanner;
  if(mScript)   delete mScript;
}

bool CmdThis::isCmd(const QString& cmd)
{
  return cCmd1 == cmd;
}

QString CmdThis::regCmd(CmdHelper* ch)
{
  if(!ch) return "";

  ch->regCmds(cCmd1);

  return cCmd1;
}

void CmdThis::briefIn(CmdHelper* ch)
{
  if(!ch) return;

  ch->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdThis::exec(CmdHelper* ch)
{
  if(!init(ch)) return false;

  if(mCmd->isMissingParms(5))
  {
    if(mCmd->printThisWay("<Symbol> <Market> <Provider> <FromDate> <ToDate> [<FiId> <MarketId>]")) return true;

    mCmd->printComment(tr("It's used by 'full'. You should use 'fetch' instead, "
                          "because the time is coming where it will removed."));

    mCmd->aided();
    return true;
  }

  QDate fromDate   = mCmd->argDate(4, QDate(1000, 1, 1));
  QDate toDate     = mCmd->argDate(5, QDate::currentDate());
  int fiId         = mCmd->argInt(6);
  int marketId     = mCmd->argInt(7);

  if(mCmd->hasError()) return false;

  if(!fiId or !marketId)
  {
    SymbolTuple* st = mFilu->getSymbol(mCmd->argStr(1), mCmd->argStr(2), mCmd->argStr(3));
    if(!st)
    {
      error(FUNC, tr("Symbol not found: %1-%2-%3").arg(mCmd->argStr(1), mCmd->argStr(2), mCmd->argStr(3)));
      return false;
    }

    st->next();

    fiId     = st->fiId();
    marketId = st->marketId();

    delete st;
  }

  addEODBarData(mCmd->argStr(1)
              , mCmd->argStr(2)
              , mCmd->argStr(3)
              , fromDate
              , toDate
              , fiId
              , marketId);

  return !hasError();
}

void CmdThis::addEODBarData(const QString& symbol, const QString& market, const QString& provider
                          , const QDate& fromDate, const QDate& toDate, int fiId, int marketId)
{
  DateRange dateRange;
  mFilu->getEODBarDateRange(dateRange, fiId, marketId, Filu::eBronze);
  // FIXME: To find out if anything is todo we have to check more smarter.
  //        We have to use the 'offday' table, but its not implemented yet.

  QDate from = fromDate;
  QDate to   = toDate;

  // Avoid 'holes' in the data table
  if(from > dateRange.value("last")) from = dateRange.value("last").addDays(1);
  else if(from == QDate(1000, 1, 1)) from = dateRange.value("last").addDays(1);

  if(from.dayOfWeek() == Qt::Saturday) from = from.addDays(2);
  else if(from.dayOfWeek() == Qt::Sunday) from = from.addDays(1);

  if(from > QDate::currentDate())
  {
    verbose(FUNC, tr("Nothing todo, last bar is up to date: %1 %2")
                    .arg(mCmd->argStr(1), mCmd->argStr(2)), eAmple);
    return;
  }

  // Once more, avoid 'holes' in the data table
  if(to < dateRange.value("first")) to = dateRange.value("first").addDays(-1);
  else if(to == QDate(3000, 1, 1)) to = QDate::currentDate();

  // Could happens if you update bars on weekend but there is nothing todo
  if(from > to) return;

  // Build the parameter list needed by the script
  QStringList scriptParms;
  scriptParms.append(from.toString(Qt::ISODate));
  scriptParms.append(to.toString(Qt::ISODate));
  scriptParms.append(symbol);
  scriptParms.append(market);

  if(!mScript) mScript = new Script(this);

  QStringList* data = mScript->askProvider(provider, "fetchBar", scriptParms);

  if(!data)
  {
    addErrors(mScript->errors());
    return;
  }

  if(data->size() < 2)
  {
    warning(FUNC, tr("No bars from %1 for: %2").arg(provider, scriptParms.join(" ")));
    return;
  }

  // Here is the beef...
  mFilu->addEODBarData(fiId, marketId, data);
  delete data; // No longer needed
  if(check4FiluError(FUNC)) return;
  // ...and as dessert check for events
  if(!mScanner)
  {
    mScanner = new Scanner(this);
    mScanner->autoSetup();
  }

  mScanner->scanThis(fiId, marketId);
}
