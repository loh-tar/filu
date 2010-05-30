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

#ifndef SQLTABLEVIEW_HPP
#define SQLTABLEVIEW_HPP

#include <QtGui>

/***********************************************************************
*
*
*
************************************************************************/

class SqlTableView : public QTableView
{
  Q_OBJECT

  public:
                SqlTableView(QWidget* parent = 0);
    virtual    ~SqlTableView();

  signals:
    void        newSelection(const QModelIndex &);

  protected:
    void        mousePressEvent(QMouseEvent* pEvent);
    void        mouseMoveEvent(QMouseEvent* pEvent);
    void        dropEvent(QDropEvent* pEvent);
    void        dragEnterEvent(QDragEnterEvent* pEvent);
    void        dragMoveEvent(QDragMoveEvent* pEvent);
    void        startDrag();
    QPoint      mDragStartPosition;

    int         mCurrentRow;

  protected slots:
    void        currentChanged(const QModelIndex& current, const QModelIndex& previous);
    void        click(const QModelIndex& current);
};

#endif
