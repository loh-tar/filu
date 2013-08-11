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
#include <QFile>
#include <QTextStream>

#include "CmdImp.h"

#include "CmdHelper.h"
#include "FiluU.h"
#include "Importer.h"
#include "RcFile.h"

static const QString cCantFind = QObject::tr("Can't find file: %1");
static const QString cLookFor  = QObject::tr("Look for: %1");

static const QString cCmd1 = "imp";
static const QString cCmd1Brief = QObject::tr("Imports data in .imp format");

CmdImp::CmdImp(AgentF* agent)
      : CmdClass(agent, FUNC)
      , mImporter(0)
{}

CmdImp::~CmdImp()
{
  if(mImporter) delete mImporter;
}

bool CmdImp::isCmd(const QString& cmd)
{
  return cCmd1 == cmd;
}

QString CmdImp::regCmd(CmdHelper* ch)
{
  if(!ch) return "";

  ch->regCmds(cCmd1);

  return cCmd1;
}

void CmdImp::briefIn(CmdHelper* ch)
{
  if(!ch) return;

  ch->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdImp::exec(CmdHelper* ch)
{
  if(!init(ch)) return false;

  mCmd->regOpts("stdin");

  int needParm = mCmd->has("stdin") ? 0 : 1;

  if(mCmd->isMissingParms(needParm))
  {
    mCmd->inOptBrief("stdin", "To read from stdin instead from file");

    if(mCmd->printThisWay("<File>|~~stdin")) return true;

    mCmd->printComment(tr(
      "With --stdin will read from stdin until Ctrl-D is read. "
      "The <File> may be absulute path or not. In the latter case is "
      "searched in the current directory and in ImportPath."));
    mCmd->printComment(tr(
      "See also doc/import-file-format.txt and doc/config-file.txt"));

    mCmd->aided();
    return true;
  }

  QTextStream* in;
  QFile* file = 0;

  if(!mCmd->argStr(1).isEmpty())
  {
    QString url = mCmd->argStr(1);
    verbose(FUNC, cLookFor.arg(url), eAmple);
    if(!QFile::exists(url))
    {
      url = mRcFile->getPath("ImportPath") + mCmd->argStr(1);
      verbose(FUNC, cLookFor.arg(url), eAmple);
      if(!QFile::exists(url))
      {
        error(FUNC, cCantFind.arg(mCmd->argStr(1)));
        errInfo(FUNC, tr("ImportPath is: %1").arg(mRcFile->getPath("ImportPath")));
        return false;
      }
    }

    file = new QFile(url);
    if(!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
      error(FUNC, tr("Can't open file: %1").arg(url));
      return false;
    }

    verbose(FUNC, tr("Import file: %1").arg(QFileInfo(url).canonicalFilePath()));
    in = new QTextStream(file);
  }
  else
  {
    in = new QTextStream(stdin);
  }

  if(!mImporter) mImporter = new Importer(this);

  while (!in->atEnd())
  {
    QString line = in->readLine();
    if(!mImporter->import(line)) break;
  }

  mImporter->import("[EOF]");

  if(file)
  {
    file->close();
    delete file;
  }

  delete in;

  check4FiluError(FUNC);

  return !hasError();
}
