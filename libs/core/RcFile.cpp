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

QVariant RcFile::getValue(const QString& key, const QVariant& /*def*/) const
{
  if(mForced.contains(key)) return mForced.value(key);
  else return value(key);
};

void RcFile::saveGroup()
{
  mSavedGroup = group();
  while(!group().isEmpty()) endGroup();
}

void RcFile::restoreGroup()
{
  beginGroup(mSavedGroup);
}

void RcFile::takeConfigParms(const QHash<QString, QVariant>& forced)
{
  if(forced.size() == 0)
  {
    checkFiluHome();
    return;
  }

  mForced = forced;

  bool makePermanent = QCoreApplication::arguments().at(1) == "set" ? true : false;

  checkFiluHome();

  if(mForced.size())
  {
    QStringList keys = mForced.keys();
    int width = - FTool::maxSizeOfStrings(keys);
    foreach(QString key, keys)
    {
      if(makePermanent)
      {
        mNewswire->verbose(FUNC, tr("Write to config file: %1 = %2")
                                   .arg(key, width)
                                   .arg(mForced.value(key).toString())
                                       , Newswire::eEver);

        set(key, mForced.value(key));
      }
      else
      {
        mNewswire->verbose(FUNC, tr("Use temporary config parm: %1 = %2")
                                   .arg(key, width)
                                   .arg(mForced.value(key).toString())
                                       , Newswire::eAmple);
      }
    }
  }

  return;
}

void RcFile::checkFiluHome()
{
  mNewswire->setVerboseLevel(FUNC, getST("Verbose"));

  QString filuConf = QSettings::fileName();
  if(!QFile::exists(filuConf))
  {
    mNewswire->verbose(FUNC, tr("Create user config file: %1").arg(filuConf));
    QFile::copy(getPath("InstallPath") + "userfiles/Filu.conf", filuConf);
    sync();
  }

  QString filuHome = getUrl("FiluHome");
  if(!QFile::exists(filuHome))
  {
    mNewswire->verbose(FUNC, tr("Create new FiluHome: %1").arg(filuHome));
    FTool::copyDir(getPath("InstallPath") + "userfiles/", filuHome);
  }

  mNewswire->setLogFile(getUrl("LogFile"));
}
