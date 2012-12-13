//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012  loh.tar@googlemail.com
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

#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>

/***********************************************************************
*
*   This is our QLineEdit with an extra clear button inside.
*
*   Thanks to Girish Ramakrishnan
*   http://labs.qt.nokia.com/2007/06/06/lineedit-with-a-clear-button/
*
************************************************************************/

class QToolButton;

class LineEdit : public QLineEdit
{
  Q_OBJECT

  public:
                  LineEdit(QWidget *parent = 0);
    virtual      ~LineEdit();

    void          doNotHideClearButton(bool noHide = true);

  protected:
    void          resizeEvent(QResizeEvent *);

    QToolButton*  mClearBtn;

  private slots:
    virtual void  updateClearBtn(const QString &text);
    virtual void  clearBtnClicked();
};

#endif
