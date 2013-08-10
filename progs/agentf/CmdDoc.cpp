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
#include <QDebug>
#include <QDir>
// #include <QFile>
#include <QProcess>
// #include <QDirIterator>

#include "CmdDoc.h"

#include "CmdHelper.h"
#include "FiluU.h"
#include "FTool.h"
#include "RcFile.h"

static const QString cCmd1 = "doc";
static const QString cCmd1Brief = QObject::tr("List and show documentation");

CmdDoc::CmdDoc(AgentF* agent)
     : CmdClass(agent, FUNC)
{}

CmdDoc::~CmdDoc()
{}

bool CmdDoc::isCmd(const QString& cmd)
{
  return cCmd1 == cmd;
}

QString CmdDoc::regCmd(CmdHelper* ch)
{
  if(!ch) return "";

  ch->regCmds(cCmd1);

  return cCmd1;
}

void CmdDoc::briefIn(CmdHelper* ch)
{
  if(!ch) return;

  ch->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdDoc::exec(CmdHelper* ch)
{
  if(!init(ch)) return false;

  mCmd->regStdOpts("grep");

  if(mCmd->wantHelp())
  {
    mCmd->inOptBrief("grep", tr("Search in documentation"));
  }

  if(mCmd->isMissingParms())
  {
    if(mCmd->printThisWay("[<FileNumber>|<Pattern>]")) return !hasError();

    mCmd->printComment(tr(
      "Without any parameter will a table of all available documentation shown "
      "and you prompt to enter a file number to display the document. After quit "
      "reading the doc you will ask again to enter a file number. Hit RETURN "
      "to leave the loop."));
    mCmd->printComment(tr(
      "When you give the <FileNumber> as parameter will the document shown immediately "
      "and you will not ask again for a new file number. You can limit the table "
      "by give a <Pattern> matching the document name. If the result is only one "
      "file will it shown immediately."));
    mCmd->printNote(tr(
      "Text files are shown by UNIX 'less' command whereas other files will called "
      "with 'xdg-open' to use your preferred application."));

    mCmd->aided();
    return !hasError();
  }

  if(mCmd->has("grep"))     grep();
  else                      list();

  return !hasError();
}

void CmdDoc::list()
{
  if(!mCmd->parmCount())
  {
    if(!loadFileNames()) return;
    listFiles();
    while(lessFile()) {} // Nothing todo in block
  }
  else
  {
    QString parm = mCmd->parmStr(1);
    int fileNumber = parm.toInt();

    if(!fileNumber)
    {
      if(!loadFileNames(parm)) return;
      if(mAllFiles.size() == 1)
      {
        lessFile(mAllFiles.keys().at(0));
      }
      else
      {
        listFiles();
        while(lessFile()) {} // Nothing todo in block
      }
    }
    else
    {
      if(!loadFileNames()) return;
      lessFile(fileNumber);
    }
  }
}

void CmdDoc::grep()
{
  if(mCmd->isMissingParms(1))
  {
    mCmd->groupOpts("Options", ""); // Don't show <Options>
    if(mCmd->printThisWay("<Pattern> ~~grep")) return;

    mCmd->aided();
    return;
  }

  verbose(FUNC, "Not yet implemented.");
}

void CmdDoc::listFiles()
{
  int minColWidth = 1;
  minColWidth  = qMax(minColWidth, FTool::maxSizeOfStrings(mTxtFiles));
  minColWidth  = qMax(minColWidth, FTool::maxSizeOfStrings(mImpFiles));
  minColWidth  = qMax(minColWidth, FTool::maxSizeOfStrings(mConfFiles));
  minColWidth  = qMax(minColWidth, FTool::maxSizeOfStrings(mFooFiles));

  FTool::TableOptions tabOpt; // Table Options
//   tabOpt.insert(FTool::eWidth, 80);
//   tabOpt.insert(FTool::eColumns, 2);
  tabOpt.insert(FTool::eMinColWidth, minColWidth + 1);
//   tabOpt.insert(FTool::eIndent, 2);
  tabOpt.insert(FTool::eAddEmptyLines, 1);

  if(mTxtFiles.size())
  {
    print(tr("Text Documents"));
    foreach(const QString& row, FTool::formatToTable(mTxtFiles, tabOpt)) print(row);
  }

  if(mFooFiles.size())
  {
    print(tr("Other Documents"));
    foreach(const QString& row, FTool::formatToTable(mFooFiles, tabOpt)) print(row);
  }

  if(mConfFiles.size())
  {
    print(tr("Config Files"));
    foreach(const QString& row, FTool::formatToTable(mConfFiles, tabOpt)) print(row);
  }

  if(mImpFiles.size())
  {
    print(tr("Example Files To Import"));
    foreach(const QString& row, FTool::formatToTable(mImpFiles, tabOpt)) print(row);
  }
}

bool CmdDoc::lessFile(int fileNumber/* = 0*/)
{
  if(!fileNumber)
  {
    fileNumber = FTool::askUserInt("Enter file number to show: ");

    if(!fileNumber) return false;
  }

  QString program;

  if(mFooFileNumbers.contains(fileNumber))
  {
    program = QString("xdg-open %1%2").arg(mRcFile->getPath("DocPath")
                                          , mAllFiles.value(fileNumber));
  }
  else
  {
    program = QString("less %1%2").arg(mRcFile->getPath("DocPath")
                                      , mAllFiles.value(fileNumber));
  }

  if(verboseLevel(eInfo))
  {
    print(tr("Execute: %1").arg(program));
    FTool::sleep(500);
  }

  QProcess::execute(program);

  return true;
}

bool CmdDoc::loadFileNames(const QString& pattern/* = ""*/)
{
  QDir docDir(mRcFile->getPath("DocPath"));

  if(!docDir.exists())
  {
    error(FUNC, tr("Document path does not exist."));
    errInfo(FUNC, tr("Check your config files."));
    errInfo(FUNC, tr("DocPath is now: %1").arg(mRcFile->getPath("DocPath")));
    return false;
  }

  int idx = 0;
  foreach(const QString& file, docDir.entryList(QDir::Files/*, QDir::Type*/))
  {
    const QString mask = "%1 - %2";
    const int     w    = 2;

    if(!file.contains(pattern)) continue;

    mAllFiles.insert(++idx, file);

    if(file.endsWith(".txt"))        mTxtFiles  << mask.arg(idx, w).arg(file.left(file.size() - 4));
    else if(file.endsWith(".imp"))   mImpFiles  << mask.arg(idx, w).arg(file.left(file.size() - 4));
    else if(file.endsWith(".conf"))  mConfFiles << mask.arg(idx, w).arg(file.left(file.size() - 5));
    else
    {
      mFooFiles       << mask.arg(idx, w).arg(file/*.left(file.size() - 4)*/);
      mFooFileNumbers << idx;
    }
  }

  if(!mAllFiles.size() and pattern.isEmpty())
  {
    fatal(FUNC, "Where is my documentation?");
    return false;
  }
  else if(!mAllFiles.size())
  {
    verbose(FUNC, tr("No files match pattern '%1'.").arg(pattern));
    return false;
  }

  return true;
}
