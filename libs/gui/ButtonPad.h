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

#ifndef BUTTONPAD_HPP
#define BUTTONPAD_HPP

#include <QButtonGroup>
class QBoxLayout;
class QToolButton;

#include "FWidget.h"
class SettingsFile;

/***********************************************************************
*
*   Holds a collection of user definable buttons to start any action.
*   - Provide a context menu edit window,
*   - Is usable in toolbars or as standalone widget.
*
*   FIXME: Add icons
*
************************************************************************/

class ButtonPad : public FWidget
{
  Q_OBJECT

  public:
                    ButtonPad(const QString& name, FClass* parent);
                    ButtonPad(const QString& name, FClass* parent, const QString& className);
    virtual        ~ButtonPad();

    virtual void    loadSettings();
    void            addToToolBar(QToolBar* tb);

  public slots:
    void            orientationChanged(Qt::Orientation o); // Only used when standalone widget

  protected:
    virtual void    saveSettings();
    SettingsFile*   openSettings();
    void            closeSettings();
    QToolButton*    newButton(const QString& name, int id = -1);
    void            deleteButton(QAbstractButton* btn);
    void            setButtonName(QAbstractButton* btn, const QString& name);
    virtual QToolButton* addDummyButton();

    QButtonGroup    mButtons;
    QBoxLayout*     mLayout;            // Only used when standalone widget

  protected slots:
    virtual void    buttonClicked(int id);
    virtual void    buttonContextMenu(const QPoint& pos);

  private:
    SettingsFile*   mSettings;
};

#endif
