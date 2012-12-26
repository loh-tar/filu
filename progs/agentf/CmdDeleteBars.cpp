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

#include "CmdDeleteBars.h"

#include "CmdHelper.h"
#include "FiluU.h"

static const QString cCmd1 = "deleteBars";
static const QString cCmd1Brief = QObject::tr("Delete one or a range of eod bars of one FI");

CmdDeleteBars::CmdDeleteBars(FClass* parent)
      : CmdClass(parent, FUNC)
{}

CmdDeleteBars::~CmdDeleteBars()
{}

void CmdDeleteBars::regCmd(CmdHelper* ch)
{
  if(!ch) return;

  ch->regCmds(cCmd1);
}

void CmdDeleteBars::briefIn(CmdHelper* ch)
{
  if(!ch) return;

  ch->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdDeleteBars::exec(CmdHelper* ch)
{
  if(!init(ch)) return false;

  if(mCmd->isMissingParms(3))
  {
    if(mCmd->printThisWay("<Symbol> <Market> <FromDate> [<ToDate>]")) return true;

    mCmd->printComment(tr("Use dot-dot '..' for most old/new date. "
                          "Without <ToDate> only one bar will delete."));

    mCmd->printForInst("AAPL NewYork 2012-01-01 2012-06-01");
    mCmd->printForInst("AAPL NewYork 2012-01-01 ..");
    mCmd->aided();
    return true;
  }

  QDate fromDate = mCmd->argDate(3, QDate(1000, 1, 1));
  QDate toDate   = mCmd->argDate(4, fromDate, QDate(3000, 1, 1));

  if(mCmd->hasError()) return false;

  mFilu->setSqlParm(":symbol",   mCmd->argStr(1));
  mFilu->setSqlParm(":market",   mCmd->argStr(2));
  mFilu->setSqlParm(":fromDate", fromDate.toString(Qt::ISODate));
  mFilu->setSqlParm(":toDate",   toDate.toString(Qt::ISODate));

  QSqlQuery* query = mFilu->execSql("DeleteBars");
  if(check4FiluError(FUNC)) return false;

  int nra = query->numRowsAffected();
  if(!nra) warning(FUNC, tr("NO bars deleted!"));
  else verbose(FUNC, tr("%1 bars deleted.").arg(nra));

  return !hasError();
}
