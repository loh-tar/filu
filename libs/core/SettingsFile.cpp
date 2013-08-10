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
  switch(static_cast<int>(val.type())) // Don't Warnung: case-Wert »135« nicht in Aufzählungstyp »QVariant::Type«
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
SettingsFile::getPath(const QString& key) const
{
  static int i = 0;
  if(++i > 10)
  {
    QString msg = "*** CONFIG FILE ERROR *** Endless recursion $" + key + " ";
    // FIXME warning(FUNC, tr("Endless recursion while access of %1").arg(key))
    i = 0;
    return msg;
  }

  QString path = getValue(key, QVariant()).toString();

  if(path.startsWith("~/"))
  {
    path.remove(0, 1);
    path.prepend(QDir::homePath());
  }

  if(path.isEmpty())
  {
    i = 0;
    return path;
  }

  QRegExp rx("\\$\\w+");

  if(rx.indexIn(path, 0) > -1)
  {
    QString var = rx.cap(0);
    QString ke2 = var.right(var.size() - 1);
    QString sub = getPath(ke2);
    sub.chop(1);
    path.replace(var, sub);

  }

  if(!path.endsWith("/")) path.append("/");

  i = 0;

  return path;
}

QString
SettingsFile::getUrl(const QString& key) const
{
  QString url = getPath(key);
  url.chop(1); //Remove added slash /

  return url;
}

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
