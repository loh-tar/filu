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

#ifndef MANAGERPAGE_H
#define MANAGERPAGE_H

class QGroupBox;
class QListWidgetItem;

#include "FWidget.h"

class ManagerPage : public FWidget
{
  Q_OBJECT

  public:
                    ManagerPage(FClass* parent, const QString& className);
    virtual        ~ManagerPage();

    virtual void    loadSettings();
    virtual void    saveSettings();

    void            setPageIcon(QListWidgetItem* button);
    const QString&  iconText() { return mIconText; };

    void            emitMessage(const QString& func, const QString& txt, const MsgType type = eInfoMsg)
                    {
                      emit message(makeMessage(func, txt, type));
                    };

    signals:
    void            message(const Message& message);

  protected:
    void            hideEvent(QHideEvent* /*event*/);
    void            showEvent(QShowEvent* /*event*/);

    QWidget*        mFocusWidget;
    QIcon           mIcon;
    QString         mIconText;
    QGroupBox*      mMainBox;
};

#endif
