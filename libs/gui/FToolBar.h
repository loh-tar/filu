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

#ifndef FTOOLBAR_HPP
#define FTOOLBAR_HPP

#include <QToolBar>

#include "FWidget.h"

/***********************************************************************
*
*   This class provide moving buttons
*
************************************************************************/

class FToolBar : public QToolBar, public FClass
{
  Q_OBJECT // Needed for propperly set metaObject()->className()

  public:
                FToolBar(const QString& title, FClass* parent);
    virtual    ~FToolBar() {}

    void        saveSettings();
    void        loadSettings();

  protected:
    bool        event(QEvent* event);
    bool        eventFilter(QObject* obj, QEvent* event);
    void        dropEvent(QDropEvent* event);
    void        dragEnterEvent(QDragEnterEvent* event);
    void        dragMoveEvent(QDragMoveEvent* event);
    void        startDrag();

    QAction*    mDragedAct;
};

#endif
