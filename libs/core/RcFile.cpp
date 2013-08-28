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

#include <QDate>
#include <QDir>

#include "RcFile.h"

#include "FTool.h"
#include "Newswire.h"

RcFile::RcFile(Newswire* parent)
      : SettingsFile()
      , mNewswire(parent)
{
}

RcFile::~RcFile()
{
}

bool RcFile::setConfigKeys(const QHash<QString, QVariant>& config)
{
  const QStringList keys = config.keys();
  if(!checkConfigKeys(keys)) return false;

  foreach(const QString& key, keys)
  {
    QString val = config.value(key).toString();
    if("---" == val)
    {
      mNewswire->verbose(FUNC, tr("Remove from config file: %1").arg(key));
      remove(key);
    }
  }

  int width = - FTool::maxSizeOfStrings(keys);
  foreach(const QString& key, keys)
  {
    QString val = config.value(key).toString();

    if("---" == val) continue;

    mNewswire->verbose(FUNC, tr("Write to config file: %1 = %2")
                               .arg(key, width).arg(val));
    set(key, val);
  }

  checkFiluHome();
  return true;
}

QVariant RcFile::getValue(const QString& key, const QVariant& /*def*/) const
{
  if(mForced.contains(key)) return mForced.value(key);
  else return value(key);
}

void RcFile::saveGroup()
{
  mSavedGroup = group();
  while(!group().isEmpty()) endGroup();
}

void RcFile::restoreGroup()
{
  beginGroup(mSavedGroup);
}

bool RcFile::forceConfigSetting(const QHash<QString, QVariant>& forced)
{
  if(forced.size() == 0)
  {
    checkConfigFile();
    checkFiluHome();
    return true;
  }

  const QStringList keys = forced.keys();
  if(!checkConfigKeys(keys)) return false;

  mForced = forced;

  checkConfigFile();
  checkFiluHome();

  if(!mNewswire->verboseLevel(Newswire::eAmple)) return true;

  int width = - FTool::maxSizeOfStrings(keys);
  foreach(const QString& key, keys)
  {
    mNewswire->verbose(FUNC, tr("Use temporary config: %1 = %2")
                                .arg(key, width).arg(getST(key)));
  }

  return true;
}

void RcFile::checkConfigFile()
{
  mNewswire->setVerboseLevel(FUNC, getST("Verbose"));

  QString filuConf = QSettings::fileName();
  if(!QFile::exists(filuConf))
  {
    mNewswire->verbose(FUNC, tr("Create user config file: %1").arg(filuConf));
    QFile::copy(getPath("InstallPath") + "user-config-file.conf", filuConf);
    sync();
  }
}

void RcFile::checkFiluHome()
{
  QString filuHome = getUrl("FiluHome");
  if(!QFile::exists(filuHome))
  {
    mNewswire->verbose(FUNC, tr("Create new FiluHome: %1").arg(filuHome));
    FTool::copyDir(getPath("InstallPath") + "userfiles/", filuHome);
  }

  mNewswire->setLogFile(getUrl("LogFile"));
}

bool RcFile::checkConfigKeys(const QStringList& keys) const
{
  const QStringList known = QSettings::allKeys();

  bool ok = true;

  foreach(const QString& key, keys)
  {
    if(!known.contains(key))
    {
      ok = false;
      mNewswire->error(FUNC, tr("Unknown config key: %1").arg(key));
    }

    if(key == "Devil")
    {
      ok = false;
      mNewswire->error(FUNC, tr("For your own security is playing with the devil forbidden"));
    }
  }

  return ok;
}
