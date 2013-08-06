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

#ifndef RCFILE_HPP
#define RCFILE_HPP

#include <QPoint>
#include <QSize>

#include "SettingsFile.h"
class Newswire;

/***********************************************************************
 *
 *   Besides of read/write the config file, watch he the Filu home
 *   and create a new one if not exist
 *
 ************************************************************************/

class RcFile : public SettingsFile
{
  public:
                  RcFile(Newswire* parent);
    virtual      ~RcFile();

    void          saveGroup();
    void          restoreGroup();
    void          takeConfigParms(const QHash<QString, QVariant>& forced);
    void          checkFiluHome();

  protected:
    QVariant      getValue(const QString& key, const QVariant&) const;

    Newswire*     mNewswire;
    QString       mSavedGroup;
    QHash<QString, QVariant>  mForced;
};

#endif
