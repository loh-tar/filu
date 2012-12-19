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

#include <QDate>
#include <QDir>

#include "FTool.h"
#include "Newswire.h"
#include "RcFile.h"

RcFile::RcFile(Newswire* parent)
      : QSettings("Filu")
      , mNewswire(parent)
{
  mDefault.reserve(6);

  mDefault.insert("FiluHome",          "/Filu/");
  mDefault.insert("IndicatorPath",     "Indicators/");
  mDefault.insert("IndiSetsPath",      "IndicatorSets/");
  mDefault.insert("TradingRulePath",   "TradingRules/");
  mDefault.insert("IndiFilterSetPath", "IndicatorFilterSettings/");

  mDefault.insert("LogFile",           "filu.log");

  checkFiluHome();
}

RcFile::~RcFile()
{
}

void RcFile::set(const QString& key, const QVariant& val)
{
  switch(val.type())
  {
    case QVariant::Date:
      setValue(key, val.toDate().toString(Qt::ISODate));
      break;
    default:
      setValue(key, val);
  }

  mForced.remove(key);
}

QString RcFile::getST(const QString& key)
{
  return getValue(key).toString();
}

QPoint RcFile::getPT(const QString& key)
{
  return getValue(key).toPoint();
}

QSize RcFile::getSZ(const QString& key)
{
  return getValue(key).toSize();
}

QByteArray RcFile::getBA(const QString& key)
{
  return getValue(key).toByteArray();
}

QDate RcFile::getDT(const QString& key)
{
  return getValue(key).toDate();
}

int RcFile::getIT(const QString& key)
{
  return getValue(key).toInt();
}

bool RcFile::getBL(const QString& key)
{
  return getValue(key).toBool();
}

double RcFile::getDB(const QString& key)
{
  return getValue(key).toDouble();
}

QString RcFile::getGlobalST(const QString& key)
{
  saveGroup();

  QString val = getValue(key).toString();

  restoreGroup();

  return val;
}

void RcFile::saveGroup()
{
  mDefault.insert("_SavedGroup", group()); // Use of mDefault is only a makeshift
  while(!group().isEmpty()) endGroup();
}

void RcFile::restoreGroup()
{
  beginGroup(mDefault.value("_SavedGroup").toString());
}

QStringList RcFile::takeConfigParms(QStringList& cmdLine)
{
  mNewswire->setVerboseLevel(FUNC, getST("Verbose"));

  QStringList filuParms;

  int pos = cmdLine.indexOf("--config");

  if(-1 == pos)
  {
    mNewswire->setLogFile(getST("LogFile"));
    return filuParms;
  }

  bool makePermanent = cmdLine.at(1) == "set" ? true : false;

  cmdLine.takeAt(pos); // Remove --config

  QString parm;
  while(pos < cmdLine.size())
  {
    if(cmdLine.at(pos).startsWith("--")) break;

    parm.append(cmdLine.takeAt(pos));

    QStringList keyVal = parm.split("=", QString::SkipEmptyParts);
    if(keyVal.size() < 2) continue;

    mForced.insert(keyVal.at(0), keyVal.at(1));
    filuParms.append(parm);
    parm.clear();
  }

  mNewswire->setVerboseLevel(FUNC, getST("Verbose"));
  mNewswire->setLogFile(getST("LogFile"));

  if(mForced.size())
  {
    QStringList keys = mForced.keys();
    int width = - FTool::maxSizeOfStrings(keys);
    foreach(QString key, keys)
    {
      if(makePermanent)
      {
        mNewswire->verbose(FUNC, tr("Write to config file: %1 = %2")
                                 .arg(key, width).arg(mForced.value(key).toString()), Newswire::eEver);

        set(key, mForced.value(key));
      }
      else
      {
        mNewswire->verbose(FUNC, tr("Use temporary config parm: %1 = %2")
                                  .arg(key, width).arg(mForced.value(key).toString()), Newswire::eAmple);
      }
    }
  }

  return filuParms;
}

/***********************************************************************
*
*                             Protected  Stuff
*
************************************************************************/
void RcFile::checkFiluHome()
{
  QString filuHome = getST("FiluHome");

  if(QFile::exists(filuHome)) return;

  if(mDefault.value("FiluHome").toString() == filuHome)
  {
    filuHome = QDir::homePath() + filuHome;
  }

  mNewswire->verbose(FUNC, tr("Create new FiluHome: %1").arg(filuHome));

  FTool::copyDir(getST("InstallPath") + "userfiles/", filuHome);

  QString filuConf = QDir::homePath() + "/.config/Filu.conf";
  if(!QFile::exists(filuConf))
  {
    mNewswire->verbose(FUNC, tr("Create user settings file: %1").arg(filuConf));
    QFile::copy(filuHome + "Filu.conf", filuConf);
    sync();
  }

  QFile::remove(filuHome + "Filu.conf");

  // Write some relevant keys to the new config file
  set("FiluHome", filuHome);
  setFullPath("FiluHome", "IndicatorPath");
  setFullPath("FiluHome", "IndiSetsPath");
  setFullPath("FiluHome", "TradingRulePath");
  setFullPath("FiluHome", "IndiFilterSetPath");
  setFullPath("FiluHome", "LogFile");
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
