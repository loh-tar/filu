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
#include <QTextStream>

#include "CmdImp.h"

#include "CmdHelper.h"
#include "FiluU.h"
#include "Importer.h"

static const QString cCmd1 = "imp";
static const QString cCmd1Brief = QObject::tr("Imports an (surprise!) import file. "
                                              "See doc/import-file-format.txt");

CmdImp::CmdImp(FClass* parent)
      : CmdClass(parent, FUNC)
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

  if(mCmd->isMissingParms())
  {
    if(mCmd->printThisWay("[<FileName>]")) return true;

    mCmd->printComment(tr("Without <FileName> will read from stdin (Ctrl-D to quit)."));
    mCmd->aided();
    return true;
  }

  QTextStream* in;
  QFile* file = 0;

  if(!mCmd->argStr(1).isEmpty())
  {
    file = new QFile(mCmd->argStr(1));
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
      error(FUNC, tr("Can't open file: %1").arg(mCmd->argStr(1)));
      return false;
    }

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
