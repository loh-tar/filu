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

#include <QFile>

#include "CmdSummon.h"

#include "CmdHelper.h"
#include "FTool.h"
#include "FiluU.h"
#include "RcFile.h"

static const QString cCmd1 = "sum";
static const QString cCmd1Brief = QObject::tr("Summon the devil");

CmdSummon::CmdSummon(FClass* parent)
      : CmdClass(parent, FUNC)
{}

CmdSummon::~CmdSummon()
{}

bool CmdSummon::isCmd(const QString& cmd)
{
  return cCmd1 == cmd;
}

void CmdSummon::regCmd(CmdHelper* ch)
{
  if(!ch) return;

  ch->regCmds(cCmd1);
}

void CmdSummon::briefIn(CmdHelper* ch)
{
  if(!ch) return;

  ch->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdSummon::exec(CmdHelper* ch)
{
  if(!init(ch)) return false;

  if(mCmd->isMissingParms(1))
  {
    if(mCmd->printThisWay("<Devil>")) return true;

    mCmd->printComment(tr("Switch to (or create new) development schemata for the current user. "
                          "The name should only contain 'valid' word characters, otherwise will "
                          "they replaced by underscroes."));
    mCmd->printNote(tr("The config key 'SqlPath' is set to 'FiluSource/database/sqls/' in the user "
                       "settings file if you run AgentF from your 'FiluSource/[build/]' directory. "
                       "In fact is that test poor. It's only tested if '[../build/]database/sqls/' exist."
                       "These applies correspondingly to 'ProviderPath'."));
    mCmd->printNote(tr("To list existing development schemata use the 'exo' command."));
    mCmd->printForInst("newidea");
    mCmd->aided();
    return true;
  }

  QString devil = FTool::makeValidWord(mCmd->parmStr(1));
  if(FTool::makeValidWord(mRcFile->getST("Devil")) == devil)
  {
    verbose(FUNC, tr("I'm already '%1' devilish").arg(devil));
    return true;
  }

  verbose(FUNC, tr("I summon the devil '%1'").arg(devil));
  mRcFile->set("Devil", devil);
  QString devilPath(getenv("PWD"));
  devilPath.remove(QRegExp("/build$"));
  devilPath.append("/database/sqls/");
  if(QFile::exists(devilPath))
  {
    verbose(FUNC, tr("Change SqlPath to: %1").arg(devilPath));
    mRcFile->set("SqlPath", devilPath);

    devilPath.remove(QRegExp("/database/sqls/$"));
    devilPath.append("/scripts/provider/");
    verbose(FUNC, tr("Change ProviderPath to: %1").arg(devilPath));
    mRcFile->set("ProviderPath", devilPath);
  }
  else
  {
    verbose(FUNC, tr("Let SqlPath untouched: %1").arg(mRcFile->getST("SqlPath")));
    verbose(FUNC, tr("Let ProviderPath untouched: %1").arg(mRcFile->getST("ProviderPath")));
  }

  mFilu->closeDB();
  mFilu->openDB();

  return !hasError();
}
