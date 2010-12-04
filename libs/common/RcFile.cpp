//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010  loh.tar@googlemail.com
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

#include "RcFile.h"

RcFile::RcFile() : QSettings("Filu")
                 , mConsole(stdout)
{
  // Global stuff
  mDefault.insert("InstallPath",       "/usr/local/lib/Filu/");
  mDefault.insert("ProviderPath",      "provider/");
  mDefault.insert("FiluHome",          "Filu/");
  mDefault.insert("IndicatorPath",     "Indicators/");
  mDefault.insert("IndiSetsPath",      "IndicatorSets/");
  mDefault.insert("TradingRulePath",   "TradingRules/");
  mDefault.insert("IndiFilterSetPath", "IndicatorFilterSettings/");

  // Filu stuff
  mDefault.insert("HostName",          "localhost");
  mDefault.insert("HostPort",          5432);
  mDefault.insert("UserName",          "filu");
  mDefault.insert("Password",          "filu"); // FIXME: hmm, pw in clear text...
  mDefault.insert("DatabaseName",      "filu");
  mDefault.insert("FiluSchema",        "filu");
  mDefault.insert("SqlPath",           "/usr/local/lib/Filu/sqls/");
  mDefault.insert("CommitBlockSize",   500);
  mDefault.insert("DaysToFetchIfNoData",    365);
  mDefault.insert("SqlDebugLevel",      1);

  // Agent stuff
  mDefault.insert("MaxClones",         5);
  mDefault.insert("LogFile",           "agentf.log");
  mDefault.insert("DebugLevel",        0);

  // Performer stuff
  mDefault.insert("PerformerSize",     QSize(832,512));
  mDefault.insert("PerformerPosition", "");
  mDefault.insert("PerformerState",    "");
  mDefault.insert("PerformerIndiSet",  "Default");

  // Manager stuff
  mDefault.insert("ManagerSize",     QSize(832,512));
  mDefault.insert("ManagerPosition", "");
  mDefault.insert("ManagerState",    "");

  // Inspetor stuff
  mDefault.insert("InspectorSize",     QSize(832,512));
  mDefault.insert("InspectorPosition", "");
  mDefault.insert("InspectorState",    "");
  //mDefault.insert("",    "");

  checkFiluHome();
}

RcFile::~RcFile()
{
}

void RcFile::get(const QString& key, QString& val)
{
  QVariant v;
  v = value(key, mDefault.value(key));

  val = v.toString();
}

QString RcFile::getST(const QString& key)
{
  return value(key, mDefault.value(key)).toString();
}

void RcFile::set(const QString& key, const QVariant& val)
{
  setValue(key, val);
}

void RcFile::get(const QString& key, QPoint& val)
{
  QVariant v;
  v = value(key, mDefault.value(key));

  val = v.toPoint();
}

QPoint RcFile::getPT(const QString& key)
{
  return value(key, mDefault.value(key)).toPoint();
}

void RcFile::get(const QString& key, QSize& val)
{
  QVariant v;
  v = value(key, mDefault.value(key));

  val = v.toSize();
}

QSize RcFile::getSZ(const QString& key)
{
  return value(key, mDefault.value(key)).toSize();
}

void RcFile::get(const QString& key, QByteArray& val)
{
  QVariant v;
  v = value(key, mDefault.value(key));

  val = v.toByteArray();
}

QByteArray RcFile::getBA(const QString& key)
{
  return value(key, mDefault.value(key)).toByteArray();
}

int RcFile::getIT(const QString& key)
{
  return value(key, mDefault.value(key)).toInt();
}

bool RcFile::getBL(const QString& key)
{
  return value(key, mDefault.value(key)).toBool();
}

double RcFile::getDB(const QString& key)
{
  return value(key, mDefault.value(key)).toDouble();
}

/***********************************************************************
*
*                             Protected  Stuff
*
************************************************************************/
void RcFile::checkFiluHome()
{
  QString filuHome = getST("FiluHome");

  // In case of first run is filuHome = ".Filu/"
  // In case of any run is filuHome = "/home/steve/.Filu/"
  if(!filuHome.startsWith('/') or !QDir().exists(filuHome))
  {
    mConsole << "RcFile::checkFiluHome() ..." << endl;

    QString dir;
    if(!filuHome.startsWith('/')) // ".Filu/" ?
    {
      dir = QDir::homePath();
      dir.append("/" + filuHome); // Now: "/home/steve/.Filu/"
    }
    else
    {
      // An other filuHome was given in config file
      dir = filuHome;
    }

    if(!createDir(dir)) return;

    set("FiluHome", dir); // Write to config file

    FTool::copyDir(getST("InstallPath") + "userfiles/", dir);

    // Write all relevant keys to the (still empty) config file
    setValue("InstallPath", getST("InstallPath"));

    setFullPath("InstallPath", "ProviderPath");
    setFullPath("FiluHome", "IndicatorPath");
    setFullPath("FiluHome", "IndiSetsPath");
    setFullPath("FiluHome", "TradingRulePath");
    setFullPath("FiluHome", "IndiFilterSetPath");
    setFullPath("FiluHome", "LogFile");

    mConsole << "RcFile::checkFiluHome() " << tr("...done.") << endl;

    sync();
  }

}

bool RcFile::createDir(const QString& d)
{
  mConsole << "RcFile::createDir() " + tr("Create... ") + d;;
  QDir dir;
  if(!dir.mkpath(d))
  {
    mConsole << tr(" ...FAIL!") << endl;
    return false;
  }

  mConsole << endl;

  return true;
}

void RcFile::setFullPath(const QString& path, const QString& key)
{
  QString fullPath = getST(key);

  if(!fullPath.startsWith('/'))
  {
    fullPath.prepend(getST(path));
  }

  set(key, fullPath);
}
