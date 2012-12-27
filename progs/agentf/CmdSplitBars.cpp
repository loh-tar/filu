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

#include "CmdSplitBars.h"

#include "CmdHelper.h"
#include "FiluU.h"

static const QString cCmd1 = "splitBars";
static const QString cCmd1Brief = QObject::tr("To correct faulty data of the provider");

CmdSplitBars::CmdSplitBars(AgentF* agent)
            : CmdClass(agent, FUNC)
{}

CmdSplitBars::~CmdSplitBars()
{}

bool CmdSplitBars::isCmd(const QString& cmd)
{
  return cCmd1 == cmd;
}

QString CmdSplitBars::regCmd(CmdHelper* ch)
{
  if(!ch) return "";

  ch->regCmds(cCmd1);

  return cCmd1;
}

void CmdSplitBars::briefIn(CmdHelper* ch)
{
  if(!ch) return;

  ch->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdSplitBars::exec(CmdHelper* ch)
{
  if(!init(ch)) return false;

  if(mCmd->isMissingParms(5))
  {
    if(mCmd->printThisWay("<Symbol> <Market> <FromDate> <ToDate> <SplitPre:Post>")) return true;

    mCmd->printNote(tr("To accomplish a split event use the 'add split' command."));
    mCmd->printForInst("KO NewYork 2011-06-01 2012-08-13 1:2");
    mCmd->aided();
    return true;
  }

  double  pre;
  double  post;
  double  ratio;
  bool    ok;

  QStringList sl = mCmd->argStr(5).split(":");
  if(sl.size() < 2)
  {
    error(FUNC, "Ratio must be <SplitPre:Post>.");
    return false;
  }

  pre = sl[0].toDouble(&ok);
  if(pre == 0.0)
  {
    error(FUNC, "<SplitPre:Post>, Pre must not 0.");
    return false;
  };

  post = sl[1].toDouble(&ok);
  if(post == 0.0)
  {
    error(FUNC, "<SplitPre:Post>, Post must not 0.");
    return false;
  };

  ratio = pre / post;

  mFilu->setSqlParm(":fromDate", mCmd->argDate(3).toString(Qt::ISODate));
  mFilu->setSqlParm(":toDate",   mCmd->argDate(4).toString(Qt::ISODate));

  if(mCmd->hasError()) return false;

  mFilu->setSqlParm(":symbol",   mCmd->argStr(1));
  mFilu->setSqlParm(":market",   mCmd->argStr(2));
  mFilu->setSqlParm(":ratio",    ratio);
  mFilu->setSqlParm(":quality",  0);

  QSqlQuery* query = mFilu->execSql("SplitBars");
  if(check4FiluError(FUNC)) return false;

  int nra = query->numRowsAffected();
  if(!nra) warning(FUNC, tr("NO bars adjusted!"));
  else verbose(FUNC, tr("%1 bars adjusted.").arg(nra));

  return !hasError();
}
