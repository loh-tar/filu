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

#include "SettingsFile.h"

SettingsFile::SettingsFile()
            : QSettings("Filu")
{
  setIniCodec("UTF-8");
}

SettingsFile::SettingsFile(const QString& file)
            : QSettings(file, QSettings::IniFormat)
{
  setIniCodec("UTF-8");
}

SettingsFile::~SettingsFile()
{}

void
SettingsFile::set(const QString& key, const QVariant& val)
{
  //qDebug() << "SettingsFile::set" << key << val << val.type() << (int)val.type();
  switch(val.type())
  {
    case QVariant::Date:
      setValue(key, val.toDate().toString(Qt::ISODate));
      break;
    case QVariant::Double:
      setValue(key, QString::number(val.toDouble()));
      break;
    case QMetaType::Float:
      setValue(key, QString::number(val.toDouble()));
      break;
    default:
      setValue(key, val);
  }
}

QVariant
SettingsFile::getValue(const QString& key, const QVariant& def) const
{
  // These stupid looking function is needed to support RcFile
  return value(key, def);
};

QString
SettingsFile::getST(const QString& key, const QString& def /*= QString()*/) const
{
  return getValue(key, def).toString();
}

QPoint
SettingsFile::getPT(const QString& key, const QPoint& def /*= QPoint()*/) const
{
  return getValue(key, def).toPoint();
}

QSize
SettingsFile::getSZ(const QString& key, const QSize& def /*= QSize()*/) const
{
  return getValue(key, def).toSize();
}

QByteArray
SettingsFile::getBA(const QString& key) const
{
  return getValue(key, QVariant()).toByteArray();
}

QDate
SettingsFile::getDT(const QString& key, const QDate& def /*= QDate()*/) const
{
  return getValue(key, def).toDate();
}

int
SettingsFile::getIT(const QString& key, int def /*= 0*/) const
{
  return getValue(key, def).toInt();
}

bool
SettingsFile::getBL(const QString& key, bool def /*= true*/) const
{
  return getValue(key, def).toBool();
}

double
SettingsFile::getDB(const QString& key, double def /*= 0.0*/) const
{
  return getValue(key, def).toDouble();
}
