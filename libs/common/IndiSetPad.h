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

#ifndef INDISETPAD_HPP
#define INDISETPAD_HPP

#include "ButtonPad.h"

/***********************************************************************
*
*   Holds a collection of user defineable buttons to switch the used
*   set of indicators of an IndiWidgetGroup
*
************************************************************************/

class IndiSetPad : public ButtonPad
{
  Q_OBJECT

  public:
                  IndiSetPad(const QString& name, FClass* parent);
    virtual      ~IndiSetPad();

    int           loadSettings();

  signals:
    void          setupChosen(const QString& setup);

  public slots:

  protected:
    int           saveSettings();

    QList<int>    mIndiCount;

  protected slots:
    void          buttonClicked(int id);
    void          buttonContextMenu(const QPoint& pos);
};

#endif
