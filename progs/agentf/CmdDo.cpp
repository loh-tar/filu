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

#include "CmdDo.h"

#include "AgentF.h"
#include "CmdHelper.h"
#include "FiluU.h"

static const QString cCmd1 = "do";
static const QString cCmd1Brief = QObject::tr("Do jobs. The jobs can read from stdin, file or database");

CmdDo::CmdDo(AgentF* agent)
     : CmdClass(agent, FUNC)
     , mIamEvil(false)
{}

CmdDo::~CmdDo()
{}

bool CmdDo::isCmd(const QString& cmd)
{
  return cCmd1 == cmd;
}

QString CmdDo::regCmd(CmdHelper* ch)
{
  if(!ch) return "";

  ch->regCmds(cCmd1);

  return cCmd1;
}

void CmdDo::briefIn(CmdHelper* ch)
{
  if(!ch) return;

  ch->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdDo::exec(CmdHelper* ch)
{
  if(!init(ch)) return false;

  mCmd->regOptsOneOfIsMandatory("Source", "stdin file jobs");

  if(mCmd->isMissingParms(0))
  {
    mCmd->inOptBrief("stdin", "[<Name>]", "Read commands from stdin until 'quit' or Ctrl-D. "
                                          "The optional name is only used at message logging");
    mCmd->inOptBrief("jobs", "Execute pending jobs");
    mCmd->inOptBrief("file", "<FileName>", "Read commands from given file. "
                                           "Lines begin with an asterisk are ignored");

    if(mCmd->printThisWay("<Source>")) return true;

    mCmd->printComment(tr("The command can be any command supported by AgentF"));
    mCmd->aided();
    return true;
  }

  if(mCmd->hasOpt("stdin"))   return fromStdIn();
  if(mCmd->hasOpt("file"))    return fromFile();
  if(mCmd->hasOpt("jobs"))    return fromDB();

  return false; // Should never happens
}

bool CmdDo::fromFile()
{
  QString fileName = mCmd->argStr(1);

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    error(FUNC, tr("Can't open file: %1").arg(fileName));
    return false;
  }

  // Don't run 'do --stdin' which could make trouble
  bool saveIamEvil = mIamEvil;
  mIamEvil = true;

  // Read the commands from file
  QTextStream in(&file);
  int lnNo = 0;
  while (!in.atEnd())
  {
    QString line = in.readLine();
    ++lnNo;
    QStringList cmd;

    if(!lineToCommand(line, cmd)) continue;

    cmd.prepend("RCF"); // Add the "caller", normaly is here "agentf" placed
    mAgent->exec(cmd);
    if(hasError())
    {
      error(FUNC, tr("Error after executing line %1 of file %2").arg(lnNo).arg(fileName));
      break;
    }
  }

  file.close();
  mIamEvil = saveIamEvil;

  return !hasError();
}

bool CmdDo::fromStdIn()
{
  if(mIamEvil) return true; // Don't do stupid things

  mIamEvil = true;

  if(verboseLevel(eMax))
  {
    mAgent->setMsgTargetFormat(eVerbose, QString("%D %T %C %1: %F %x").arg(mCmd->argStr(1)));
    mAgent->setMsgTargetFormat(eConsLog, QString("%D %T %C %1: *** %t *** %F %x").arg(mCmd->argStr(1)));
    mAgent->setMsgTargetFormat(eRecord,  QString("%D %T %C %1: %F %x").arg(mCmd->argStr(1)));
  }
  else
  {
    mAgent->setMsgTargetFormat(eVerbose, QString("%D %T %C %1: %x").arg(mCmd->argStr(1)));
    mAgent->setMsgTargetFormat(eConsLog, QString("%D %T %C %1: *** %t *** %x").arg(mCmd->argStr(1)));
    mAgent->setMsgTargetFormat(eRecord,  QString("%D %T %C %1: %x").arg(mCmd->argStr(1)));
  }

  mAgent->setNoFileLogging();
  mFilu->setNoErrorLogging();

  QTextStream console(stdout);
  QTextStream in(stdin);
  while (!in.atEnd())
  {
    console << "[READY] (Ctrl+D or \"quit\" for quit)" << endl;
    QString line = in.readLine();
    QStringList cmd;
    if(!lineToCommand(line, cmd)) continue;
    if(cmd.at(0) == "quit") break;

    cmd.prepend("DAEMON"); // Add the "caller", normaly is here "agentf" placed
    mAgent->exec(cmd);
  }
}

bool CmdDo::fromDB()
{
  verbose(FUNC, "Not yet implemented.");
  return true;
}

bool CmdDo::lineToCommand(const QString& line, QStringList& cmd)
{
  if(line.startsWith("*")) return false; // Ignore remarks
  if(line.isEmpty()) return false;

  cmd = line.split(" ", QString::SkipEmptyParts);

  // Concat splitted parts if there was quotation marks
  int size = cmd.size();
  for(int i = 0; i < size; ++i)
  {
    if(cmd.at(i).startsWith("\""))
    {
      QString help = cmd.at(i);
      for(int j = i; j < size; )
      {
        help.append(" " + cmd.at(j));
        cmd.removeAt(j);
        --size;
        if(help.endsWith("\"")) break;
      }
      help.remove("\"");
      cmd[i] = help;
    }
  }

  return true;
}
