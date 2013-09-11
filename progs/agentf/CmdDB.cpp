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

#include <QDir>
#include <QSqlQuery>
#include <QSqlRecord>

#include "CmdDB.h"

#include "CmdHelper.h"
#include "FiluU.h"
#include "FTool.h"
#include "RcFile.h"

static const QString cCmd1 = "db";
static const QString cCmd1Brief = QObject::tr("Maintain the database");

CmdDB::CmdDB(AgentF* agent)
     : CmdClass(agent, FUNC)
{}

CmdDB::~CmdDB()
{}

bool CmdDB::isCmd(const QString& cmd)
{
  return cCmd1 == cmd;
}

QString CmdDB::regCmd(CmdHelper* ch)
{
  if(!ch) return "";

  ch->regCmds(cCmd1);

  return cCmd1;
}

void CmdDB::briefIn(CmdHelper* ch)
{
  if(!ch) return;

  ch->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdDB::exec(CmdHelper* ch)
{
  if(!init(ch)) return false;

  mTypes << "tables" << "func" << "views" << "data";

  mCmd->regSubCmds("remake ls patch tinker show");
  mCmd->regStdOpts("user");

  if(mCmd->subCmdLooksBad()) return false;

  if(mCmd->wantHelp())
  {
    mCmd->inSubBrief("patch", tr("Apply a patch to the database"));
    mCmd->inSubBrief("remake", tr("Create or update one or all database functions and views"));
    mCmd->inSubBrief("ls", tr("List database tables and creation SQLs"));
    mCmd->inSubBrief("tinker", tr("Change one value of any table"));
    mCmd->inSubBrief("show", tr("Show table entries"));
//     mCmd->inSubBrief("vacuum", tr("Perform some janitor tasks on the database by running vacuumdb"));

    mCmd->inOptBrief("user", "Work on user schema instead of Filu schema");
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
  else if(mCmd->hasSubCmd("tinker"))    tinker();
  else if(mCmd->hasSubCmd("show"))      show();
  else
  {
    fatal(FUNC, QString("Unsupported command: %1").arg(mCmd->cmd()));
  }

  check4FiluError(FUNC);

  return !hasError();
}

void CmdDB::remake()
{
  mCmd->regStdOpts("all +user");

  int parmsNeeded = mCmd->has("all") ? 0 : 2;

  if(mCmd->isMissingParms(parmsNeeded))
  {
    mCmd->inOptBrief("all", "Recreate all functions and views of the schema but no tables or default data");
    mCmd->inOptBrief("+user", "Work on user schema too, take only effect with --all");

    if(mCmd->printThisWay("[<SqlType> <SqlName>]|[--all]")) return;

    mCmd->printNote(tr("<SqlType> must be one of: %1").arg(mTypes.join(" ")));
    mCmd->printForInst("func account_insert --user");
    mCmd->printForInst("--all --user");
    mCmd->aided();
    return;
  }

  // FIXME Incomment after Filu has setDebugLevel()
  //if(mCmd->has("verbose")) mFilu->setVerboseLevel(verboseLevel());

  Filu::Schema schema = mCmd->has("user") ? Filu::eUser : Filu::eFilu;

  if(mCmd->has("all"))
  {
    mFilu->createFunctions(schema);
    mFilu->createViews(schema);

    if(mCmd->has("+user") and !mCmd->has("user"))
    {
      mFilu->createFunctions(Filu::eUser);
      mFilu->createViews(Filu::eUser);
    }

    return;
  }

  QString type   = mCmd->strParm(1);
  if(!mTypes.contains(type))
  {
    error(FUNC, tr("SqlType must be one of: %1").arg(mTypes.join(" ")));
    return;
  }

  if("tables" == type)      mFilu->createTabl(mCmd->strParm(2), schema);
  else if("func" == type)   mFilu->createFunc(mCmd->strParm(2), schema);
  else if("views" == type)  mFilu->createView(mCmd->strParm(2), schema);
  else                      mFilu->createData(mCmd->strParm(2), schema);
}

void CmdDB::list()
{
  mTypes  << "patch";

  mCmd->regStdOpts("oneCol");
  mCmd->regStdOpts("tables");

  int parmsNeeded = mCmd->has("tables") ? 0 : 1;

  if(mCmd->isMissingParms(parmsNeeded))
  {
    mCmd->inOptBrief("oneCol", "Print a single column instead of a table");
    mCmd->inOptBrief("tables", "Print existing tables instead of creation sqls");

    if(mCmd->printThisWay("<Type>")) return;

    mCmd->printNote(tr("<Type> must be one of: %1").arg(mTypes.join(" ")));
    mCmd->printForInst("func --user");
    mCmd->printForInst("--tables");
    mCmd->aided();
    return;
  }

  QStringList resultLst;

  if(mCmd->has("tables"))
  {
    Filu::Schema schema = mCmd->has("user") ? Filu::eUser : Filu::eFilu;
    resultLst = mFilu->getTables(schema);
  }
  else
  {
    QString type = mCmd->strParm(1);
    if(!mTypes.contains(type))
    {
      error(FUNC, tr("Type must be one of: %1").arg(mTypes.join(" ")));
      return;
    }

    if("func" == type) type = "functions";
    QString schema = mCmd->has("user") ? "user/" : "filu/";
    QDir dir(mRcFile->getPath("SqlPath") + schema + type, "*.sql");
    resultLst = dir.entryList(QDir::Files, QDir::Name);
    resultLst.replaceInStrings(".sql", "");
  }

  if(mCmd->has("oneCol"))
  {
    foreach(const QString& result, resultLst) print(result);
  }
  else
  {
    // FIXME Obtain the true terminal width instead of fixed 80
    int width  = 80;
    int indent = 2;
    QString tmplate = QString(" ").repeated(indent);
    tmplate.append("%1");
    foreach(const QString& row, FTool::formatToTable(resultLst, width - indent))
    {
      print(tmplate.arg(row));
    }
  }
}

void CmdDB::patch()
{
  if(mCmd->isMissingParms(1))
  {
    if(mCmd->printThisWay("<PatchSql>")) return;
    mCmd->printComment("To list available patches run 'db ls patch'.");
    mCmd->aided();
    return;
  }

  if(mCmd->has("user")) mFilu->applyPatch(mCmd->strParm(1), Filu::eUser);
  else                  mFilu->applyPatch(mCmd->strParm(1), Filu::eFilu);
}

void CmdDB::tinker()
{
  if(mCmd->isMissingParms(4))
  {
//       mCmd->inOptBrief("", "", "");

    if(mCmd->printThisWay("<RecordId> <TableName> <FieldName> <NewValue>")) return;

    mCmd->printComment(tr("There will made no checks if the change may cause trouble,"
                          "so use it with care. To obtain the needed parameter use 'ls' and 'show'"));
    mCmd->printForInst("3 market caption NYSE",
                       tr("Well, you rename most likely 'NewYork' but now does it not fit with the market name "
                          "shipped by the scripts"));
    mCmd->printForInst("1234 eodbar qclose 45.67",
                       tr("No check is made if close > high or close < low and no update is made for "
                          "the 'quality' field. Therefore may at any time later the close price again "
                          "replaced with the same wrong value by a script. Sure, you can tinker the "
                          "quality field too"));
    mCmd->aided();
    return;
  }

  int     id          = mCmd->parmInt(1);
  QString table       = mCmd->parmStr(2);
  QString field       = mCmd->parmStr(3);
  QString newValue    = mCmd->parmStr(4);
  Filu::Schema schema = mCmd->has("user") ? Filu::eUser : Filu::eFilu;

  if(mCmd->hasError()) return;

  if(!mFilu->getTableColumns(table, schema).contains(field))
  {
    if(check4FiluError(FUNC)) return; // Fail if table was not found
    error(FUNC, tr("Table '%1' has no field '%2'").arg(table, field));
    return;
  }

  mFilu->updateField(field, newValue, table, id, schema);
}

void CmdDB::show()
{
  mCmd->regStdOpts("noPager machine ticket");

  if(mCmd->isMissingParms(1))
  {
    mCmd->inOptBrief("noPager", "Don't interrupt result printing");
    mCmd->inOptBrief("machine", "Print result as csv table");
    mCmd->inOptBrief("ticket", "Print result as...ticket");

    if(mCmd->printThisWay("<Table> [<Field><Operator><Value>..]")) return;

    mCmd->printComment("The Operator can be = > < whereas you have to enclose the whole parameter in "
                       "(double)quotes if operator is < or >. "
                       "The search is pretty smart. If Field is 'caption' there will be searched fuzzy "
                       "anyway what kind of operator is used.");

    mCmd->printNote(tr("There must not spaces between Field/Value and Operator"));

    mCmd->printForInst("fi caption=ll \"ftype_id>2\" --ticket");
    mCmd->printForInst("eodbar quality=3");
    mCmd->printForInst("market");
    mCmd->printComment(" ");
    mCmd->printComment("The first example list FIs with 'll' in name but no currencies, "
                       "the second list bad bars and the latter all markets");
    mCmd->aided();
    return;
  }

  QString table       = mCmd->parmStr(1);
  Filu::Schema schema = mCmd->has("user") ? Filu::eUser : Filu::eFilu;
  QStringList  fvList = mCmd->parmList(); // Field Value List
  fvList.removeAt(0); // Remove 'Table'

  if(mCmd->hasError()) return;

  QSqlQuery* query = mFilu->searchRows(table, fvList, schema);
  if(!query) return;

  verbose(FUNC, tr("Found %1 matching records.").arg(query->size()));

  if(mCmd->has("ticket"))
  {
    showPrintTicket(query);
    return;
  }

  showPrintTable(query);
}

void CmdDB::showPrintTable(QSqlQuery* query)
{
  QSqlRecord rec = query->record();

  // Print Header
  QString header;
  QList<int> colWidth;
  QString separator(" ");
  for(int i = 0; i < rec.count(); ++i)
  {
    QString fn = rec.fieldName(i);

    // Try to determine a useful column width
    int                               width =    8;
    if(fn.endsWith("_id"))            width =    5;
    else if(fn.endsWith("date"))      width =   10;
    else if(fn.endsWith("text"))      width =  -30;
    else if(fn.endsWith("note"))      width =  -30;
    else if("qvol" == fn)             width =   10;
    else if("quality" == fn)          width =    1;
    else if("caption" == fn)          width =  -20;

    if(width < 0)
    {
      if(-width < fn.size()) width = -fn.size();
    }
    else
    {
      if( width < fn.size()) width =  fn.size();
    }

    if(mCmd->has("machine"))
    {
      width = 0;
      separator = ";";
    }

    colWidth.append(width);
    header.append(QString("%1").arg(fn, width));
    header.append(separator);
  }

  header.chop(1); // Remove last separator
  print(header);

  // Print Data
  int   printed = 1;             // Count printed lines
  int   toPrint = query->size(); // Count down records
  bool  pager   = mCmd->has("noPager") ? false : true;
  while(query->next())
  {
    QString data;
    for(int i = 0; i < rec.count(); ++i)
    {
      QString value = query->value(i).toString();
      if(    colWidth.at(i) // Don't change if --machine was given
         and value.size() > abs(colWidth.at(i)))
      {
        value = value.left(abs(colWidth.at(i)) - 1);
        value.append("~");
      }

      data.append(QString("%1").arg(value, colWidth.at(i)));
      data.append(separator);
    }

    data.chop(1); // Remove last separator
    print(data);
    --toPrint;
    ++printed;

    if(pager and printed > 25) // FIXME Determine the real terminal hight
    {
      if(!FTool::askUserNoYes(tr("%1 records left. Show more?").arg(toPrint))) break;

      print(header);
      printed = 1;
    }
  }
}

void CmdDB::showPrintTicket(QSqlQuery* query)
{
  QSqlRecord rec = query->record();

  // Determine the longest field name
  int fieldNameSize = 0;
  for(int i = 0; i < rec.count(); ++i)
  {
    fieldNameSize = qMax(fieldNameSize, rec.fieldName(i).size());
  }

  // Print Data
  QString tmplate("%1 : %2");
  int     printed = 0;             // Count printed lines
  int     toPrint = query->size(); // Count down records
  bool    pager   = mCmd->has("noPager") ? false : true;
  while(query->next())
  {
    for(int i = 0; i < rec.count(); ++i)
    {
      print(tmplate.arg(rec.fieldName(i), -fieldNameSize)
                   .arg(query->value(i).toString()));
      ++printed;
    }

    print("");
    ++printed;
    --toPrint;

    if(pager and printed > 25) // FIXME Determine the real terminal hight
    {
      if(!FTool::askUserNoYes(tr("%1 records left. Show more?").arg(toPrint))) break;

      printed = 0;
    }
  }
}
