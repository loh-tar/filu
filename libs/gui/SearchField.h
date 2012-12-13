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

#ifndef SEARCHFIELD_HPP
#define SEARCHFIELD_HPP

class QListWidget;
class QListWidgetItem;

#include "LineEdit.h"

/***********************************************************************
*
*   Yes, it's a search field, used at several places
*
************************************************************************/

class SearchField : public LineEdit
{
  Q_OBJECT

  public:
                  SearchField(QWidget* parent);
    virtual      ~SearchField();

    QRegExp       filter();

    signals:
    void          textChanged();

  private:
    QListWidget*  mHistory;

  private slots:
    void          clearBtnClicked();
    void          updateHistory();
    void          historyChosen(QListWidgetItem* item);
};

#endif
