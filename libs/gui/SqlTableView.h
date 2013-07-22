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

#ifndef SQLTABLEVIEW_HPP
#define SQLTABLEVIEW_HPP

#include <QTableView>
#include <QSqlQueryModel>

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
    virtual          ~SqlTableView();

    void              setQuery(QSqlQuery* q);
    QSqlQueryModel*   model() { return mModel; }
    void              selectRowWithValue(const QVariant& val, int column);

  signals:
    void        newSelection(const QModelIndex&);

  public slots:
    void        clearSelection();

  protected:
    void        mousePressEvent(QMouseEvent* pEvent);
    void        mouseMoveEvent(QMouseEvent* pEvent);
    void        dropEvent(QDropEvent* pEvent);
    void        dragEnterEvent(QDragEnterEvent* pEvent);
    void        dragMoveEvent(QDragMoveEvent* pEvent);
    void        startDrag();
    QPoint      mDragStartPosition;

    QSqlQueryModel* mModel;
    int             mCurrentRow;

  protected slots:
    void        currentChanged(const QModelIndex& current, const QModelIndex& previous);
    void        click(const QModelIndex& current);
    void        doubleClick(const QModelIndex& current);
};

#endif
