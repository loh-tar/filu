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

#ifndef RCFILE_HPP
#define RCFILE_HPP

#include <QtCore>

#include "FTool.h"

using namespace std;

class RcFile : public QSettings
{
  public:
                  RcFile();
    virtual      ~RcFile();

    void set(const QString& key, const QVariant& val);

    QString       getST(const QString& key);
    QPoint        getPT(const QString& key);
    QSize         getSZ(const QString& key);
    QByteArray    getBA(const QString& key);
    QDate         getDT(const QString& key);
    int           getIT(const QString& key);
    bool          getBL(const QString& key);
    double        getDB(const QString& key);

    QString       getGlobalST(const QString& key);

    void          saveGroup();
    void          restoreGroup();

  protected:
    void          checkFiluHome();
    bool          createDir(const QString& d);
    void          setFullPath(const QString& path, const QString& key);

    QTextStream   mConsole;/*(stdout);*/

    QHash<QString, QVariant>  mDefault;
};

#endif
