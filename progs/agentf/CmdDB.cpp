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
#include "FTool.h"
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
    mCmd->inSubBrief("remake", tr("Create or update one or all database functions and views"));
    mCmd->inSubBrief("ls", tr("List database tables and creation SQLs"));
//     mCmd->inSubBrief("vacuum", tr("Perform some janitor tasks on the database by running vacuumdb"));

    mCmd->inOptBrief("user", ""
                   , "Work on user schema instead of Filu schema");
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
  mCmd->regStdOpts("all both");

  int parmsNeeded = mCmd->has("all") ? 0 : 2;

  if(mCmd->isMissingParms(parmsNeeded))
  {
    mCmd->inOptBrief("all", ""
                   , "Recreate all functions and views of the schema but no(t yet) misc stuff");
    mCmd->inOptBrief("both", ""
                   , "Recreate all functions and views of user and Filu schema. "
                     "Take only effect with --all");

    if(mCmd->printThisWay("[<SqlType> <SqlName>]|[--all]")) return;

    mCmd->printNote(tr("<SqlType> must be one of: %1").arg(mTypes.join(" ")));
    mCmd->printForInst("func account_insert --user");
    mCmd->printForInst("--all --user");
    mCmd->aided();
    return;
  }

  // FIXME Incomment after Filu has setDebugLevel()
  //if(mCmd->has("verbose")) mFilu->setVerboseLevel(verboseLevel());

  if(mCmd->has("all"))
  {
    if(mCmd->has("both") and mCmd->has("user"))
    {
      error(FUNC, tr("Option '--both' is not allowed with '--user'"));
      return;
    }

    if(mCmd->has("both") or !mCmd->has("user"))
    {
      mFilu->createFunctions();
      mFilu->createViews();
      //mFilu->();
    }

    if(mCmd->has("both") or mCmd->has("user"))
    {
      mFilu->createUserFunctions();
      //mFilu->();
    }

    return;
  }

  QString type   = mCmd->strParm(1);
  if(!mTypes.contains(type))
  {
    error(FUNC, tr("SqlType must be one of: %1").arg(mTypes.join(" ")));
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
  mTypes  << "patch" << "tables";

  mCmd->regStdOpts("oneCol");

  if(mCmd->isMissingParms(1))
  {
    mCmd->inOptBrief("oneCol", ""
                   , "Print a single column instead of a table");

    if(mCmd->printThisWay("<Type>")) return;

    mCmd->printNote(tr("<Type> must be one of: %1").arg(mTypes.join(" ")));
    mCmd->printForInst("func --user");
    mCmd->aided();
    return;
  }

  QString type   = mCmd->strParm(1);
  if(!mTypes.contains(type))
  {
    error(FUNC, tr("Type must be one of: %1").arg(mTypes.join(" ")));
    return;
  }


  QStringList resultLst;

  if("tables" == type)
  {
    Filu::Schema schema = mCmd->has("user") ? Filu::eUser : Filu::eFilu;
    resultLst = mFilu->getTables(schema);
  }
  else
  {
    if("func" == type) type = "functions";
    QString schema = mCmd->has("user") ? "user/" : "filu/";
    QDir dir(mRcFile->getST("SqlPath") + schema + type, "*.sql");
    resultLst = dir.entryList(QDir::Files, QDir::Name);
    resultLst.replaceInStrings(".sql", "");
  }

  if(mCmd->has("oneCol"))
  {
    foreach(QString result, resultLst) print(result);
  }
  else
  {
    // FIXME Obtain the true terminal width instead of fixed 80
    int width  = 80;
    int indent = 2;
    QString tmplate = QString(" ").repeated(indent);
    tmplate.append("%1");
    foreach(QString row, FTool::formatToTable(resultLst, width - indent))
    {
      print(tmplate.arg(row));
    }
  }
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
