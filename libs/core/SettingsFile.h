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

#ifndef SETTINGSFILE_HPP
#define SETTINGSFILE_HPP

#include <QDate>
#include <QPoint>
#include <QSettings>
#include <QSize>

/***********************************************************************
 *
 *   A QSettings which use utf-8 and ini-format.
 *   Saves double and float as string not as QVariant,
 *   has (ugly named) handy access functions and is the base of RcFile.
 *
 ************************************************************************/

class SettingsFile : public QSettings
{
  public:
                  SettingsFile();
                  SettingsFile(const QString& file);
    virtual      ~SettingsFile();

    void          set(const QString& key, const QVariant& val);

    QString       getST(const QString& key, const QString& def = QString()) const;
    QPoint        getPT(const QString& key, const QPoint& def = QPoint()) const;
    QSize         getSZ(const QString& key, const QSize& def = QSize()) const;
    QByteArray    getBA(const QString& key) const; // Default would be pointless
    QDate         getDT(const QString& key, const QDate& def = QDate()) const;
    int           getIT(const QString& key, int def = 0) const;
    bool          getBL(const QString& key, bool def = true) const;
    double        getDB(const QString& key, double def = 0.0) const;

  protected:
    virtual QVariant  getValue(const QString& key, const QVariant& def) const;
};

#endif
