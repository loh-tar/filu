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

#include "CmdClass.h"

#include "CmdHelper.h"

// List here all sub classes
#include "CmdAdd.h"
#include "CmdDB.h"
#include "CmdImp.h"
#include "CmdSummon.h"

static const QString cCmd1 = "CmdClass";
static const QString cCmd1Brief = QObject::tr("You should never read this. "
                                              "Hey hacker: Derived class need 'briefIn' function.");

CmdClass::CmdClass(FClass* parent, const QString& func)
      : FClass(parent, func)
      , mCmd(0)
{}

CmdClass::~CmdClass()
{}

CmdClass* CmdClass::createNew(const QString& type, FClass* parent)
{
  // Append these command list when you add a new sub classe
  if("Add" == type)         return new CmdAdd(parent);
  else if("DB" == type)     return new CmdDB(parent);
  else if("Imp" == type)    return new CmdImp(parent);
  else if("Summon" == type) return new CmdSummon(parent);
//   else if("" == type)       return new Cmd(parent);

  // FIXME Not possible because this here is a static function
  //   fatal(FUNC, QString("Unknown CmdClass type: '%1'").arg(type));

  return 0;
}

void CmdClass::allRegCmds(CmdHelper* ch)
{
  // Append these command list when you add a new sub classe
  CmdAdd::regCmd(ch);
  CmdDB::regCmd(ch);
  CmdImp::regCmd(ch);
  CmdSummon::regCmd(ch);
}

void CmdClass::allBriefIn(CmdHelper* ch)
{
  // Append these command list when you add a new sub classe
  CmdAdd::briefIn(ch);
  CmdDB::briefIn(ch);
  CmdImp::briefIn(ch);
  CmdSummon::briefIn(ch);
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
