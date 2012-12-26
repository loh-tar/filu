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

#include "CmdSet.h"

#include "CmdHelper.h"
#include "FiluU.h"

static const QString cCmd1 = "set";
static const QString cCmd1Brief = QObject::tr("Set config file values");

CmdSet::CmdSet(FClass* parent)
      : CmdClass(parent, FUNC)
{}

CmdSet::~CmdSet()
{}

bool CmdSet::isCmd(const QString& cmd)
{
  return cCmd1 == cmd;
}

void CmdSet::regCmd(CmdHelper* ch)
{
  if(!ch) return;

  ch->regCmds(cCmd1);
}

void CmdSet::briefIn(CmdHelper* ch)
{
  if(!ch) return;

  ch->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdSet::exec(CmdHelper* ch)
{
  if(!init(ch)) return false;

  if(mCmd->isMissingParms())
  {
    mCmd->regOpts("config");
    mCmd->inOptBrief("config", "<Key>=<Value> [<Key>=<Value>].."
                   , "Key is any config file key and value ...yes. There are no checks done if "
                     "Key is known or value is valid");

    mCmd->groupOpts("Options", ""); // Don't show <Options>
    if(mCmd->printThisWay("~~config")) return true;

    mCmd->printComment(tr("As you may know take the --config option each Filu program to use temporary "
                          "different settings. With the set command will these settings written "
                          "into the config file."));
    mCmd->prin4Comment(tr("But because all this is not truly done by AgentF you may notice a differend "
                          "behavior of the set command from other commands of AgentF. The point is: "
                          "--help does not prevent from writing into the config file."));

    mCmd->printForInst("--config SqlDebug=Ample Verbose=Ample");
    mCmd->printForInst("--config Devil=-");
    mCmd->printComment(tr("Use the hyphen to disable *temporary* an existing Devil=foo setting"));
    mCmd->aided();
    return true;
  }

//   if(mIamEvil)
//   {
//     warning(FUNC, tr("The set command has no effect in batch or daemon mode."));
//   }

  // Nothing todo, all done by RcFile

  return !hasError();
}
