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

#ifndef MANAGERF_H
#define MANAGERF_H

#include "FWidget.h"

class LogBookPage;

class ManagerF : public FMainWindow
{
     Q_OBJECT

  public:
                ManagerF(const QString connectionName = "ManagerF");
    virtual    ~ManagerF();

  public slots:
    void        changePage(QListWidgetItem* current, QListWidgetItem* previous);
    void        messageBox(const QString& msg, const bool error = false);

  private:
    void        createIcons();

    QListWidget*    mPageIcons;
    QStackedWidget* mPageStack;
    QLabel*         mMessage;
    LogBookPage*    mLogBookPage;
};

#endif
