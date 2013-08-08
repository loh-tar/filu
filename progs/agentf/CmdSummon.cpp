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

#include <QFile>

#include "CmdSummon.h"

#include "CmdHelper.h"
#include "FTool.h"
#include "FiluU.h"
#include "RcFile.h"

static const QString cCmd1 = "sum";
static const QString cCmd1Brief = QObject::tr("Summon the devil");

CmdSummon::CmdSummon(AgentF* agent)
         : CmdClass(agent, FUNC)
{}

CmdSummon::~CmdSummon()
{}

bool CmdSummon::isCmd(const QString& cmd)
{
  return cCmd1 == cmd;
}

QString CmdSummon::regCmd(CmdHelper* ch)
{
  if(!ch) return "";

  ch->regCmds(cCmd1);

  return cCmd1;
}

void CmdSummon::briefIn(CmdHelper* ch)
{
  if(!ch) return;

  ch->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdSummon::exec(CmdHelper* ch)
{
  if(!init(ch)) return false;

  mCmd->regStdOpts("cpc");

  if(mCmd->isMissingParms(1))
  {
    mCmd->inOptBrief("cpc", "Copy current config instead of create a fresh one. "
                            "A perhaps existing <Devil> config is overwritten without warning!");

    if(mCmd->printThisWay("<Devil>")) return true;

    mCmd->printComment(tr(
      "Switch to (or create a new) development data base schema <Devil> for "
      "the current user."));
    mCmd->prin4Comment(tr(
      "There will also a fresh user config file created (or restored if present) "
      "where you can set any special config without to touch your productive "
      "configuration."));
      mCmd->prin4Comment(tr(
      "At default is a new FiluHome created at '%1<Devil>'. To change this add "
      "'Hell=<YourChoice>' to your productive config file.  See doc/config-file.txt. "
      "The <Devil> name should only contain 'valid' word characters, otherwise "
      "will they replaced by underscores.").arg(mRcFile->getPath("Hell")));
    mCmd->prin4Comment(tr(
      "The current config is saved as '%1.Productive' or '%1.Devil.<CurrentDevil>' "
      "if current was already divilish.").arg(mRcFile->fileName()));
    mCmd->prin4Note(tr(
      "If you run AgentF from your 'FiluSource/[build/]' directory will the config "
      "keys 'SqlPath' and 'ProviderPath' set so that the source files are used. "
      "In fact is that test poor it's only tested if '[../build/]database/sqls/' exist."));
    mCmd->printNote(tr(
      "To list existing development schemata use the 'exo' command with no more arguments."));
    mCmd->printForInst("newidea");
    mCmd->aided();
    return true;
  }

  QString devil   = FTool::makeValidWord(mCmd->parmStr(1));
  QString current = FTool::makeValidWord(mRcFile->getST("Devil"));
  if(current == devil)
  {
    verbose(FUNC, tr("I'm already '%1' devilish").arg(devil));
    return true;
  }

  QString configFile  = mRcFile->fileName();
  QString devilFile   = configFile + ".Devil." + devil;
  QString currentFile = current.isEmpty() ? configFile + ".Productive"
                                          : configFile + ".Devil." + current;

  QFile::remove(currentFile);
  QFile::rename(configFile, currentFile);
  verbose(FUNC, tr("Current config file saved."));
  verbose(FUNC, tr("I summon the devil '%1'").arg(devil));

  if(mCmd->has("cpc"))
  {
    QFile::remove(devilFile);
    QFile::copy(currentFile, devilFile);
    verbose(FUNC, tr("Current config file copyed as new devil config."));
  }

  if(QFile::exists(devilFile))
  {
    QFile::copy(devilFile, configFile);
    verbose(FUNC, tr("Existing devil config file restored."));

    mRcFile->sync();
    mRcFile->set("Devil", devil);
    mRcFile->checkFiluHome();
    verbose(FUNC, tr("SqlPath is now: %1").arg(mRcFile->getPath("SqlPath")));
    verbose(FUNC, tr("ProviderPath is now: %1").arg(mRcFile->getPath("ProviderPath")));
  }
  else
  {
    mRcFile->checkConfigFile();
    mRcFile->set("Devil", devil);
    mRcFile->set("FiluHome", mRcFile->getPath("Hell") + devil);
    mRcFile->checkFiluHome();

    QString devilPath(getenv("PWD"));
    devilPath.remove(QRegExp("/build$"));
    devilPath.append("/database/sqls/");
    if(QFile::exists(devilPath))
    {
      verbose(FUNC, tr("Change SqlPath to: %1").arg(devilPath));
      mRcFile->set("SqlPath", devilPath);

      devilPath.remove(QRegExp("/database/sqls/$"));
      devilPath.append("/scripts/provider/");
      verbose(FUNC, tr("Change ProviderPath to: %1").arg(devilPath));
      mRcFile->set("ProviderPath", devilPath);
    }
    else
    {
      verbose(FUNC, tr("Let SqlPath untouched: %1").arg(mRcFile->getUrl("SqlPath")));
      verbose(FUNC, tr("Let ProviderPath untouched: %1").arg(mRcFile->getUrl("ProviderPath")));
    }
  }

  mFilu->closeDB();
  mFilu->openDB();

  return !hasError();
}
