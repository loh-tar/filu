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

#include <QSqlQuery>

#include "CmdExorcise.h"

#include "CmdHelper.h"
#include "FTool.h"
#include "FiluU.h"
#include "RcFile.h"

static const QString cCmd1 = "exo";
static const QString cCmd1Brief = QObject::tr("Exorcise the devil");

CmdExorcise::CmdExorcise(AgentF* agent)
           : CmdClass(agent, FUNC)
{}

CmdExorcise::~CmdExorcise()
{}

bool CmdExorcise::isCmd(const QString& cmd)
{
  return cCmd1 == cmd;
}

QString CmdExorcise::regCmd(CmdHelper* ch)
{
  if(!ch) return "";

  ch->regCmds(cCmd1);

  return cCmd1;
}

void CmdExorcise::briefIn(CmdHelper* ch)
{
  if(!ch) return;

  ch->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdExorcise::exec(CmdHelper* ch)
{
  if(!init(ch)) return false;

  if(mCmd->isMissingParms())
  {
    if(mCmd->printThisWay("[<Devil>]")) return true;

    mCmd->printComment(tr("Switch in any case back to the productive version and delete "
                          "(if given) the development schemata."));
    mCmd->printNote(tr("The config keys 'SqlPath' and 'ProviderPath' will removed from "
                       "the user settings file."));

    mCmd->printForInst("mylastidea");
    mCmd->aided();
    return true;
  }

  QString devil = FTool::makeValidWord(mRcFile->getST("Devil"));

  if(!devil.isEmpty())
  {
    verbose(FUNC, tr("I renunciate the devil '%1'").arg(devil));
    mRcFile->remove("Devil");
    mRcFile->remove("SqlPath");
    mRcFile->remove("ProviderPath");
    verbose(FUNC, tr("SqlPath is now: %1").arg(mRcFile->getST("SqlPath")));
    verbose(FUNC, tr("ProviderPath is now: %1").arg(mRcFile->getST("ProviderPath")));
    mFilu->closeDB();
    mFilu->openDB();
  }

  QString user = qgetenv("USER");

  mFilu->setSqlParm(":username", user);
  QSqlQuery* query = mFilu->execSql("GetDevils");

  QSet<QString> devils;
  const QString devilPrefix = QString("user_%1_").arg(user);
  while(query->next())
  {
    QString devil = query->value(0).toString();
    devil.remove(devilPrefix);
    devils.insert(devil);
  }

  const QString killDevil = FTool::makeValidWord(mCmd->argStr(1, ""));
  const QString filuDevil = "%1_%2_%3";
  const QString userDevil = "user_%1_%2";

//   Out commentetd because I run in my own trap.
//   It is not possible to execute a SQL more than one time with static
//   parameters. What we need is a "execute once" function wich release the SQL
//   after executen, or a "release <sql>" function, or a "execStatic" function
//   wich works with normal setSqlParm() but does not use query-bind() but
//   replace the paramater by his own
//   if("+++" == killDevil) // Exorcise all devils
//   {
//     foreach(QString d, devils)
//     {
//       verbose(FUNC, tr("Devil '%1'.").arg(d));
//       mFilu->setStaticSqlParm(":filuDevil", filuDevil.arg(mRcFile->getST("FiluSchema"), user, d));
//       mFilu->setStaticSqlParm(":userDevil", userDevil.arg(user, d));
//       mFilu->execSql("BackToHell");
//
// //       devils.remove(d);
//
//       if(!mFilu->hasError())
//       {
//         verbose(FUNC, tr("Devil '%1' is banned back to hell.").arg(d));
//       }
//       else
//       {
//         return;
//       }
//     }
//
//     devils.clear();
//   }
//   else

  if(!killDevil.isEmpty())
  {
    if(devils.contains(killDevil))
    {
      mFilu->setStaticSqlParm(":filuDevil", filuDevil.arg("filu", user, killDevil));
      mFilu->setStaticSqlParm(":userDevil", userDevil.arg(user, killDevil));
      mFilu->execSql("BackToHell");

      devils.remove(killDevil);

      if(!mFilu->hasError())
      {
        verbose(FUNC, tr("Devil '%1' is banned back to hell.").arg(killDevil));
      }
    }
    else
    {
      error(FUNC, tr("No devil '%1' is amongst us.").arg(killDevil));
    }
  }

  if(verboseLevel(eInfo))
  {
    if(!devils.size())
    {
      verbose(FUNC, tr("No devils out of hell."));
    }
    else if(devils.size() == 1)
    {
      verbose(FUNC, tr("The Devil is still amongst us:"));
      verbose(FUNC, QString("  %1").arg(devils.toList().at(0)));
    }
    else
    {
      verbose(FUNC, tr("These Devils are still amongst us:"));
      foreach(QString d, devils) verbose(FUNC, QString("  %1").arg(d));
    }
  }

  return !hasError();
}
