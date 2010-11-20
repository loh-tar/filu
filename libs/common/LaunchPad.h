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

#include "ButtonPad.h"

/***********************************************************************
*
*   Holds a collection of user defineable buttons to start any
*   external application with current viewing FI as parameter
*
************************************************************************/

class LaunchPad : public ButtonPad
{
  Q_OBJECT

  public:
                  LaunchPad(const QString& name, FClass* parent);
    virtual      ~LaunchPad();

    int           loadSettings();

  public slots:
    void          newSelection(int fiId, int marketId);

  protected:
    int           saveSettings();
    void          execCmd(const QString command, SymbolTuple* st);

    QStringList   mCommands;
    QStringList   mSymbolTypes;
    QList<bool>   mMultis;
    int           mFiId;
    int           mMarketId;

  protected slots:
    void          buttonClicked(int id);
    void          buttonContextMenu(const QPoint& pos);
};

#endif
