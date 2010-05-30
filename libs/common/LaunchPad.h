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

#ifndef LAUNCHPAD_HPP
#define LAUNCHPAD_HPP

#include "FWidget.h"

/***********************************************************************
*
* Holds a collection of user defineable buttons to start any
* external application with current viewing FI as parameter
* FIXME: add icons
************************************************************************/

class LaunchPad : public FWidget
{
  Q_OBJECT

  public:
                  LaunchPad(const QString& name, FClass* parent);
    virtual      ~LaunchPad();

  public slots:
    void          loadSettings();
    void          saveSettings();
    void          newSelection(int fiId, int marketId);

  protected:
    void          execCmd(const QString command, SymbolTuple* st);

    QString       mName;
    QBoxLayout*   mLayout;
    QButtonGroup  mButtons;
    QStringList   mCommands;
    QStringList   mSymbolTypes;
    QList<bool>   mMultis;
    int           mFiId;
    int           mMarketId;

  protected slots:
    void          buttonClicked(int id);
};

#endif
