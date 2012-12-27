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

#include <QSqlQuery>
#include <QVariant>

#include "CmdFetch.h"

#include "CmdHelper.h"
#include "FiluU.h"

static const QString cCmd1 = "fetch";
static const QString cCmd1Brief = QObject::tr("To fetch data (currently only eodBars) from providers");

CmdFetch::CmdFetch(FClass* parent)
      : CmdClass(parent, FUNC)
{}

CmdFetch::~CmdFetch()
{}

bool CmdFetch::isCmd(const QString& cmd)
{
  return cCmd1 == cmd;
}

QString CmdFetch::regCmd(CmdHelper* ch)
{
  if(!ch) return "";

  ch->regCmds(cCmd1);

  return cCmd1;
}

void CmdFetch::briefIn(CmdHelper* ch)
{
  if(!ch) return;

  ch->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdFetch::exec(CmdHelper* ch)
{
  if(!init(ch)) return false;

  if(mCmd->isMissingParms(1))
  {
    if(mCmd->printThisWay("<FuzzyString> [<FromDate> [<ToDate>]]")) return true;

    mCmd->printComment(tr("The FuzzyString can be part of the FI name or symbol. All matching FIs will updated. "
                          "Without a given date, or if dot-dot '..', AgentF take a look at the database "
                          "which bars could be missing."));

    mCmd->printForInst("apple");
    mCmd->printForInst("aapl 2007-04-01");
    mCmd->aided();
    return true;
  }

  QDate fromDate = mCmd->argDate(2, QDate(1000, 1, 1));
  QDate toDate   = mCmd->argDate(3, QDate::currentDate());

  if(mCmd->hasError()) return false;

  QSqlQuery* query = mFilu->searchFi(mCmd->argStr(1), ""); // The empty string "" says: All kind of fType

  if(!query)
  {
    check4FiluError(FUNC);
    return false;
  }

  if(!query->size())
  {
    verbose(FUNC, tr("No FIs match '%1'.").arg(mCmd->argStr(1)));
    return true;
  }

  if(query->size() > 1)
  {
    verbose(FUNC, tr("Found %1 provider symbol(s) to use for eodBar update.").arg(query->size()));
  }

  while(query->next())
  {
    verbose(FUNC, tr("Update bars of %1 for %2").arg(query->value(6).toString()     // Market
                                                   , query->value(3).toString()));  // FiName

//     addEODBarData(query->value(5).toString()  // Symbol
//                 , query->value(6).toString()  // Market
//                 , query->value(7).toString()  // Provider
//                 , fromDate
//                 , toDate
//                 , query->value(0).toInt()     // FiId
//                 , query->value(1).toInt());   // MarketId
  }

  return !hasError();
}
