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

#include <QDirIterator>
#include <QFile>
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

  mCmd->regStdOpts("del-omen save-omen");

  if(mCmd->isMissingParms())
  {
    mCmd->inOptBrief("del-omen", "[<Devil>]", "Delete the config file and home directory of <Devil>");
    mCmd->inOptBrief("save-omen", "Don't delete the config file and home directory of <Devil>");

    if(mCmd->printThisWay("[<Devil>]")) return true;

    mCmd->printComment(tr(
      "Switch in any case back to the productive version by restoring the "
      "productive user config file and delete (if given) the development "
      "schema <Devil>. At default is the devil config file and home directory saved."));
    mCmd->printNote(tr(
      "You could set in your productive config file 'DeleteDevilConfig=true' to auto "
      "remove the devil config file and home directory. See doc/config-file.txt."));
    mCmd->printForInst("mylastidea", tr(
      "Remove the DB schema but keep the config file and home directory."));
    mCmd->printForInst("--del-omen mygoodidea", tr(
      "Only delete the config file and home directory, keep the DB untouched."));
    mCmd->printForInst("mybadidea --del-omen", tr(
      "Delete DB schema, config file and home directory."));
    mCmd->aided();
    return true;
  }

  QString configFile  = mRcFile->fileName();
  QString devil       = FTool::makeValidWord(mRcFile->getST("Devil"));
  QString killDevil   = FTool::makeValidWord(mCmd->argStr(1, ""));
  QString devilFile   = configFile + ".Devil." + devil;
  QString proFile     = configFile + ".Productive";
  bool    configSaved = false;

  if(!devil.isEmpty())
  {
    verbose(FUNC, tr("I renunciate the devil '%1'").arg(devil));

    QFile::remove(devilFile);
    QFile::rename(configFile, devilFile);

    configSaved = true;

    if(QFile::exists(proFile))
    {
      QFile::rename(proFile, configFile);

      mRcFile->sync();
      mRcFile->remove("Devil"); // Be sure we are no longer devilish
      mRcFile->checkFiluHome();
      verbose(FUNC, tr("Productive config file restored."));
    }
    else
    {
      mRcFile->checkConfigFile();
      mRcFile->checkFiluHome();
    }

    verbose(FUNC, tr("SqlPath is now: %1").arg(mRcFile->getPath("SqlPath")));
    verbose(FUNC, tr("ProviderPath is now: %1").arg(mRcFile->getPath("ProviderPath")));
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

  if(     (mCmd->has("del-omen") or mRcFile->getBL("DeleteDevilConfig"))
      and !mCmd->has("save-omen") )
  {
    QString devil = killDevil.isEmpty() ? mCmd->optStr("del-omen") : killDevil;
    devilFile = configFile + ".Devil." + devil;

    if(!devil.isEmpty())
    {
      if(QFile::exists(devilFile))
      {
        FTool::removeDir(SettingsFile(devilFile).getPath("FiluHome"));
        verbose(FUNC, tr("FiluHome of devil '%1' removed.").arg(devil));

        QFile::remove(devilFile);
        verbose(FUNC, tr("Config file for devil '%1' removed.").arg(devil));
        configSaved = false;
      }
      else
      {
        error(FUNC, tr("No config file for devil '%1' found to remove.").arg(devil));
      }
    }
  }

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
//     foreach(const QString& d, devils)
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
  else if(configSaved)
  {
    verbose(FUNC, tr("Config file for devil '%1' saved.").arg(devil));
  }

  if(verboseLevel(eInfo))
  {
    if(!devils.size())
    {
      verbose(FUNC, tr("No devils out of hell."));
    }
    else if(devils.size() == 1)
    {
      verbose(FUNC, tr("The devil is still amongst us:"));
      verbose(FUNC, QString("  %1").arg(devils.toList().at(0)));
    }
    else
    {
      verbose(FUNC, tr("These devils are still amongst us:"));
      foreach(const QString& d, devils) verbose(FUNC, QString("  %1").arg(d));
    }

    // FIXME: Looks ugly, find and print omen in a  more nicely way
    QString path = mRcFile->fileName();
    path.remove(QRegExp("[\\w\\.]+$"));
    QDirIterator dirIterator(path);
    bool infoTxt = false;
    while(dirIterator.hasNext())
    {
      dirIterator.next();
      QString omen = dirIterator.fileName();
      if(!omen.startsWith("Filu.conf.Devil.")) continue;

      omen.remove("Filu.conf.Devil.");
      if(devils.contains(omen)) continue;
      if(!infoTxt)
      {
        verbose(FUNC, tr("There are devil omen:"));
        infoTxt = true;
      }
      verbose(FUNC, QString("  %1").arg(omen));
    }
  }

  return !hasError();
}
