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

#include "CmdClass.h"

#include "CmdHelper.h"
#include "AgentF.h"

// List here all sub classes
#include "CmdAdd.h"
#include "CmdDB.h"
#include "CmdImp.h"
#include "CmdSummon.h"
#include "CmdExorcise.h"
#include "CmdConfig.h"
#include "CmdSplitBars.h"
#include "CmdDeleteBars.h"
#include "CmdThis.h"
#include "CmdFetch.h"
#include "CmdDo.h"
#include "CmdDoc.h"

static const QString cCmd1 = "CmdClass";
static const QString cCmd1Brief = "You should never read this. "
                                  "Hey hacker: Derived class need 'briefIn' function.";

CmdClass::CmdClass(AgentF* agent, const QString& func)
        : FClass(agent, func)
        , mAgent(agent)
        , mCmd(0)
{}

CmdClass::~CmdClass()
{}

CmdClass* CmdClass::createNew(const QString& type, AgentF* agent)
{
  // Append these command list when you add a new sub class
  if(CmdAdd::isCmd(type))           return new CmdAdd(agent);
  if(CmdDB::isCmd(type))            return new CmdDB(agent);
  if(CmdImp::isCmd(type))           return new CmdImp(agent);
  if(CmdSummon::isCmd(type))        return new CmdSummon(agent);
  if(CmdExorcise::isCmd(type))      return new CmdExorcise(agent);
  if(CmdConfig::isCmd(type))        return new CmdConfig(agent);
  if(CmdSplitBars::isCmd(type))     return new CmdSplitBars(agent);
  if(CmdDeleteBars::isCmd(type))    return new CmdDeleteBars(agent);
  if(CmdThis::isCmd(type))          return new CmdThis(agent);
  if(CmdFetch::isCmd(type))         return new CmdFetch(agent);
  if(CmdDo::isCmd(type))            return new CmdDo(agent);
  if(CmdDoc::isCmd(type))           return new CmdDoc(agent);

  // FIXME Not possible because this here is a static function
  //   fatal(FUNC, QString("Unknown CmdClass type: '%1'").arg(type));

  return 0;
}

QSet<QString> CmdClass::allRegCmds(CmdHelper* ch)
{
  // Append these command list when you add a new sub classe
  QSet<QString> cmds;

  cmds.insert(CmdAdd::regCmd(ch));
  cmds.insert(CmdDB::regCmd(ch));
  cmds.insert(CmdImp::regCmd(ch));
  cmds.insert(CmdSummon::regCmd(ch));
  cmds.insert(CmdExorcise::regCmd(ch));
  cmds.insert(CmdConfig::regCmd(ch));
  cmds.insert(CmdSplitBars::regCmd(ch));
  cmds.insert(CmdDeleteBars::regCmd(ch));
  cmds.insert(CmdThis::regCmd(ch));
  cmds.insert(CmdFetch::regCmd(ch));
  cmds.insert(CmdDo::regCmd(ch));
  cmds.insert(CmdDoc::regCmd(ch));

  return cmds;
}

void CmdClass::allBriefIn(CmdHelper* ch)
{
  // Append these command list when you add a new sub classe
  CmdAdd::briefIn(ch);
  CmdDB::briefIn(ch);
  CmdImp::briefIn(ch);
  CmdSummon::briefIn(ch);
  CmdExorcise::briefIn(ch);
  CmdConfig::briefIn(ch);
  CmdSplitBars::briefIn(ch);
  CmdDeleteBars::briefIn(ch);
  CmdThis::briefIn(ch);
  CmdFetch::briefIn(ch);
  CmdDo::briefIn(ch);
  CmdDoc::briefIn(ch);
}

void CmdClass::briefIn(CmdHelper* cmd)
{
  if(!cmd) return;

  cmd->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdClass::init(CmdHelper* ch)
{
  if(!ch)
  {
    fatal(FUNC, "Called with NULL pointer.");
    return false;
  }

  mCmd = ch;

  return true;
}
