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

#include <QDir>

#include "CmdDB.h"

#include "CmdHelper.h"
#include "FiluU.h"
#include "RcFile.h"

CmdDB::CmdDB(FClass* parent)
      : CmdClass(parent, FUNC)
{}

CmdDB::~CmdDB()
{}

void CmdDB::briefIn(CmdHelper* ch)
{
  if(!ch) return;

  static const QString cCmd1 = "db";
  static const QString cCmd1Brief = tr("Maintain the database");

  ch->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdDB::exec(CmdHelper* ch)
{
  if(!init(ch)) return false;

  mTypes << "func" << "misc" << "views";

  mCmd->regSubCmds("remake ls patch");
  mCmd->regStdOpts("user");

  if(mCmd->subCmdLooksBad()) return false;

  if(mCmd->wantHelp())
  {
    mCmd->inSubBrief("patch", tr("Apply a patch to the database"));
    mCmd->inSubBrief("remake", tr("Create or update a database function or view"));
    mCmd->inSubBrief("ls", tr("List database creation SQLs"));
//     mCmd->inSubBrief("vacuum", tr("Perform some janitor tasks on the database by running vacuumdb"));

    mCmd->inOptBrief("user", ""
                   , "Interpred <SqlType> as user SQL");
  }

  if(mCmd->needHelp(2))
  {
    if(mCmd->printThisWay("<Command> <ParmList>")) return !hasError();

    mCmd->aided();
    return !hasError();
  }

  // Look for each known command and call the related function
  if(mCmd->hasSubCmd("remake"))         remake();
  else if(mCmd->hasSubCmd("ls"))        list();
  else if(mCmd->hasSubCmd("patch"))     patch();
//   else if(mCmd->hasSubCmd(""))          ();
  else
  {
    fatal(FUNC, QString("Unsupported command: %1").arg(mCmd->cmd()));
  }

  check4FiluError(FUNC);

  return !hasError();
}

void CmdDB::remake()
{
  if(mCmd->isMissingParms(2))
  {
    if(mCmd->printThisWay("<SqlType> <SqlName>")) return;

    mCmd->printNote(tr("<SqlType> must be one of: %1").arg(mTypes.join(" ")));
    mCmd->printForInst("func account_insert --user");
    mCmd->aided();
    return;
  }

  QString type   = mCmd->strParm(1);
  if(!mTypes.contains(type))
  {
    error(FUNC, tr("SQL-Type must be one of: %1").arg(mTypes.join(" ")));
    return;
  }

  if(mCmd->has("user"))
  {
    mFilu->createUserFunc(mCmd->strParm(2));
  }
  else
  {
    if("func" == type) mFilu->createFunc(mCmd->strParm(2));
    else if("view" == type) mFilu->createView(mCmd->strParm(2));
    else mFilu->createMisc(mCmd->strParm(2));
  }
}

void CmdDB::list()
{
  mTypes  << "patch";

  if(mCmd->isMissingParms(1))
  {
    if(mCmd->printThisWay("<SqlType>")) return;

    mCmd->printNote(tr("<SqlType> must be one of: %1").arg(mTypes.join(" ")));
    mCmd->printForInst("func --user");
    mCmd->aided();
    return;
  }

  QString type   = mCmd->strParm(1);
  if(!mTypes.contains(type))
  {
    error(FUNC, tr("SQL-Type must be one of: %1").arg(mTypes.join(" ")));
    return;
  }

  if("func" == type) type = "functions";
  QString schema = mCmd->has("user") ? "user/" : "filu/";

  QDir dir(mRcFile->getST("SqlPath") + schema + type, "*.sql");
  QStringList files = dir.entryList(QDir::Files, QDir::Name);
  foreach(QString sql, files) print(sql.left(sql.size() - 4));
}

void CmdDB::patch()
{
    if(mCmd->isMissingParms(1))
    {
      mCmd->inOptBrief("user", ""
                     , "Interpred <PatchSql> as user SQL");

      if(mCmd->printThisWay("<PatchSql>")) return;

      mCmd->aided();
      return;
    }

    verbose(FUNC, "Not yet implemented.");
//     if(mCmd->has("user")) mFilu->applyPatch(mCmd->strParm(1), eUser);
//     else mFilu->applyPatch(mCmd->strParm(1), eFilu);
}
