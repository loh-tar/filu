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

#include <QApplication>
#include <QMouseEvent>

#include "SqlTableView.h"

SqlTableView::SqlTableView(QWidget* parent)
            : QTableView(parent)
            , mModel(new QSqlQueryModel)
            , mCurrentRow(-1)
{
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setHorizontalScrollMode(ScrollPerPixel);
  setVerticalScrollMode(ScrollPerPixel);
  setModel(mModel);

  connect(this, SIGNAL(clicked(const QModelIndex &)),
          this, SLOT(click(const QModelIndex &)));

  connect(this, SIGNAL(doubleClicked(const QModelIndex &)),
          this, SLOT(doubleClick(const QModelIndex &)));
  }

SqlTableView::~SqlTableView()
{
  delete mModel;
}

void SqlTableView::setQuery(QSqlQuery* q)
{
  mModel->setQuery(*q);
  mCurrentRow = -1;
}

void SqlTableView::selectRowWithValue(const QVariant& val, int column)
{
  // Select the row containing the given value
  int r = 0; // Row
  while(mModel->hasIndex(r, column))
  {
    if(val == mModel->data(mModel->index(r, column)))
    {
      selectRow(r);
      emit clicked(mModel->index(r, column));
      break;
    }
    ++r;
  }
}

void SqlTableView::clearSelection()
{
  QTableView::clearSelection();
  mCurrentRow = -1;
}

void SqlTableView::mousePressEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    mDragStartPosition = event->pos();
  }

  QTableView::mousePressEvent(event);
}

void SqlTableView::mouseMoveEvent( QMouseEvent* event )
{
  if( event->buttons() & Qt::LeftButton )
  {
    int distance = (event->pos() - mDragStartPosition).manhattanLength();
    if(distance > QApplication::startDragDistance())
      startDrag();
  }

  QTableView::mouseMoveEvent(event);
}

void SqlTableView::startDrag()
{
  QMimeData* mimeData = new QMimeData;
  mimeData->setData("application/Filu_SqlTableView", "foo");
  QDrag* drag = new QDrag( this );
  drag->setMimeData(mimeData);
  if(drag->start(Qt::CopyAction))
  {
    //qDebug() << "SqlTableView::startDrag, target was" << drag->target ();
    //DoSomething()
  }
}

void SqlTableView::dropEvent(QDropEvent* event)
{
  if(event->mimeData()->hasText())
  {
    //DoSomething();
    //qDebug() << " SqlTableView::dropEvent" << event->mimeData()->text();
    event->setDropAction( Qt::CopyAction );
    event->accept();
  }
}

void SqlTableView::dragEnterEvent(QDragEnterEvent* event)
{
  event->setDropAction(Qt::CopyAction);
  event->accept();
//   qDebug() << "dragEnterEvent";
}

void SqlTableView::dragMoveEvent(QDragMoveEvent* event)
{
  event->setDropAction(Qt::CopyAction);
  event->accept();
//   qDebug() << "dragMoveEvent";
}

void SqlTableView::currentChanged(const QModelIndex& current,
                                   const QModelIndex &/*previous*/)
{
  //qDebug() << "SqlTableView::currentChanged()" << current.row() << previous.row();

  if(current.row() == mCurrentRow) return;
  if(current.row() == -1) return;

  mCurrentRow = current.row();
  emit newSelection(current);

  scrollTo(current);
}

void SqlTableView::click(const QModelIndex& current)
{
  if(current.row() == mCurrentRow) return;
  doubleClick(current);
}

void SqlTableView::doubleClick(const QModelIndex& current)
{
  mCurrentRow = current.row();
  emit newSelection(current);
}
