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

#ifndef DIALOGBUTTON_HPP
#define DIALOGBUTTON_HPP

#include <QPushButton>

/***********************************************************************
*
*   This button add a value to a signal when is clicked.
*   That is e.g. expected by QDialog::done(int r)
*
************************************************************************/

class DialogButton : public QPushButton
{
  Q_OBJECT

  public:
                  DialogButton(const QString& text, int clickValue);
    virtual      ~DialogButton();

  signals:
    void          clicked(int);

  public slots:
    void          slotClick();

  protected:
    int           mClickValue;

  protected slots:

};

#endif
