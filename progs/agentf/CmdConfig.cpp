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

#include <QVariant>

#include "CmdConfig.h"

#include "CmdHelper.h"
#include "FTool.h"
#include "FiluU.h"
#include "RcFile.h"

static const QString cCmd1 = "config";
static const QString cCmd1Brief = QObject::tr("Set or show config file values");

CmdConfig::CmdConfig(AgentF* agent)
      : CmdClass(agent, FUNC)
{}

CmdConfig::~CmdConfig()
{}

bool CmdConfig::isCmd(const QString& cmd)
{
  return cCmd1 == cmd;
}

QString CmdConfig::regCmd(CmdHelper* ch)
{
  if(!ch) return "";

  ch->regCmds(cCmd1);

  return cCmd1;
}

void CmdConfig::briefIn(CmdHelper* ch)
{
  if(!ch) return;

  ch->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdConfig::exec(CmdHelper* ch)
{
  if(!init(ch)) return false;

  mCmd->regSubCmds("set show");

  if(mCmd->subCmdLooksBad()) return false;

  if(mCmd->wantHelp())
  {
    mCmd->inSubBrief("set", tr("Set config file values"));
    mCmd->inSubBrief("show", tr("Print config file settings"));
  }

  if(mCmd->needHelp(2))
  {
    if(mCmd->printThisWay("<Command>")) return true;

    mCmd->printComment(tr(
      "As you may know take the --config option each Filu program to use temporary "
      "different settings. With the set command will these settings written "
      "into the config file."));
    mCmd->printNote(tr(
      "To remove (unset) a key use '<Key>=---'"));
    mCmd->printForInst("set SqlDebug=Ample Verbose=Ample");
    mCmd->aided();
    return true;
  }

//   if(mIamEvil)
//   {
//     warning(FUNC, tr(""));
//   }

  if(mCmd->hasSubCmd("set"))        setConfig();
  else if(mCmd->hasSubCmd("show"))  showConfig();
  else
  {
    fatal(FUNC, QString("Unsupported command: %1").arg(mCmd->cmd()));
  }

  return !hasError();
}

void CmdConfig::setConfig()
{
  if(mCmd->isMissingParms(1))
  {
    if(mCmd->printThisWay("<Key>=<Value> [<Key>=<Value>]..")) return;

    mCmd->printComment(tr(
      "There are checks done if key is known but not if value is valid. "
      "To unset <Key> leave <Value> blank or set to triple hyphen '---'."));
    mCmd->printNote("");
    mCmd->printForInst("SqlDebug=Ample Verbose=Ample");
    mCmd->aided();
    return;
  }

  QHash<QString, QVariant> config;
  foreach(const QString&parm, mCmd->parmList())
  {
    QStringList keyval = parm.split("=");
    if(keyval.size() < 2)
    {
      error(FUNC, tr("Parameter %1 has not the format <Key>=<Value>: %2").arg(1).arg(parm));
      return;
    }

    QString  key = keyval.at(0);
    QVariant val = keyval.at(1);

    if(val.toString().isEmpty()) val = "---";

    config.insert(key, val);
  }

  mRcFile->setConfigKeys(config);
}

void CmdConfig::showConfig()
{
  mCmd->regStdOpts("noKeys path url");

  if(mCmd->isMissingParms())
  {
    mCmd->inOptBrief("noKeys", tr("Print only the value of the key"));
    mCmd->inOptBrief("path", tr("Print as path"));
    mCmd->inOptBrief("url", tr("Print as url"));

    if(mCmd->printThisWay("[<Pattern>]")) return;

    mCmd->printComment(tr(
      "Give <Pattern> to limit the output for matching config keys. The <Pattern> "
      "may be a regex but should encapsulated in quotes in that case."));
    mCmd->printNote("");
    mCmd->printForInst("ImportPath --noKeys --path --verbose 0");
    mCmd->printForInst("\"Min|Max\"");
    mCmd->aided();
    return;
  }

  QStringList keys;

  QString pattern = mCmd->argStr(1);
  foreach(const QString& key, mRcFile->allKeys())
  {
    if(!key.contains(QRegExp(pattern))) continue;

    keys.append(key);
  }

  if(mCmd->has("noKeys"))
  {
    foreach(const QString& key, keys)
    {
      if(mCmd->has("path"))       print(mRcFile->getPath(key));
      else if(mCmd->has("url"))   print(mRcFile->getUrl(key));
      else                        print(mRcFile->getST(key));
    }
  }
  else
  {
    int width = - FTool::maxSizeOfStrings(keys);
    foreach(const QString& key, keys)
    {
      if(mCmd->has("path"))       print(QString("%1 = %2").arg(key, width).arg(mRcFile->getPath(key)));
      else if(mCmd->has("url"))   print(QString("%1 = %2").arg(key, width).arg(mRcFile->getUrl(key)));
      else                        print(QString("%1 = %2").arg(key, width).arg(mRcFile->getST(key)));
    }
  }
}
