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

#ifndef MANAGERPAGE_H
#define MANAGERPAGE_H

#include <QtGui>
#include <QDebug>

//#include "FClass.h"
#include "FWidget.h"
// FIXME: why is include needed and class not enough???
// it compiles well but cause an segfault at runtime when accessing Filu
//#include "ManagerF.h"
class ManagerF;

//class FWidget;

class ManagerPage : public FWidget
{
  Q_OBJECT

  public:
                  ManagerPage(ManagerF* parent);
                  ManagerPage(FWidget* parent);
    virtual      ~ManagerPage();

    virtual void  loadSettings();
    virtual void  saveSettings();

    void          setPageIcon(QListWidgetItem* button);

    signals:      // FIXME: 'error' is currently unused. The idea was that the
                  // messages could temporary colored like some KDE progs do to
                  // force more attention. I fail already to color the background
                  // of the QLabel mMessage :-/
                  // Furthermore could bool changed to enum like in Filu.h
    void          message(const QString& msg, const bool error = false);

  protected:
    QIcon         mIcon;
    QString       mIconText;
};

#endif
