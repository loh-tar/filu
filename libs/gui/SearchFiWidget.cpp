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

#include <QAction>
#include <QActionGroup>
#include <QGridLayout>
#include <QHeaderView>
#include <QMenu>

#include "SearchFiWidget.h"

#include "SearchField.h"
#include "SqlTableView.h"
#include "FiluU.h"

/***********************************************************************
*
*
*
************************************************************************/
SearchFiWidget::SearchFiWidget(FClass* parent)
              : FWidget(parent, FUNC)
              , mHideNoMarket(false)
{
  init();
}

SearchFiWidget::~SearchFiWidget()
{}

void SearchFiWidget::init()
{
  mSearchField = new SearchField(this);
  mSearchField->setToolTip(tr("Enter pattern or FiId to search"));
  connect(mSearchField, SIGNAL(textChanged()), this, SLOT(search()));
  setFocusProxy(mSearchField);

  mTypeSelBtn = new FiTypeSelBtn(this);
  mTypeSelBtn->setToolTip(tr("Choose FiType to search"));
  connect(mTypeSelBtn, SIGNAL(selectionChanged()), this, SLOT(search()));

  mView  = new SqlTableView(this);
  mView->setDragEnabled(true);
  mView->setDropIndicatorShown(true);
  mView->horizontalHeader()->hide();
  mView->verticalHeader()->hide();
  mView->setGridStyle(Qt::NoPen);

  connect(mView, SIGNAL(newSelection(const QModelIndex &))
          , this, SLOT(clicked(const QModelIndex &)));

  QGridLayout* layout = new QGridLayout;
  layout->installEventFilter(this);
  layout->setMargin(0);
  layout->addWidget(mSearchField, 0, 0);
  layout->addWidget(mTypeSelBtn, 0, 1);
  layout->addWidget(mView, 1, 0, 1, 2);

  setLayout(layout);

  search();
}

void SearchFiWidget::setHideNoMarket(bool hide/* = true*/)
{
  mHideNoMarket = hide;
  search();
}

void SearchFiWidget::search()
{
  QSqlQuery* query = mFilu->searchFi(mSearchField->text()
                                   , mTypeSelBtn->selected()
                                   , mHideNoMarket);

  if(!query) query = mFilu->lastQuery();
  mView->setQuery(query);

  mView->horizontalHeader()->setResizeMode(3, QHeaderView::Stretch);
  mView->horizontalHeader()->setResizeMode(4, QHeaderView::ResizeToContents);
  mView->resizeRowsToContents();
  mView->hideColumn(0);
  mView->hideColumn(1);
  mView->hideColumn(2);
  mView->hideColumn(5);
  mView->hideColumn(6);
  mView->hideColumn(7);

  show();
}

void SearchFiWidget::clicked(const QModelIndex& index)
{
  int row = index.row();

  // Symbol, Market
  emit selected(mView->model()->index(row, 5).data().toString()
              , mView->model()->index(row, 6).data().toString());

  // FiId, MarketId
  emit selected(mView->model()->index(row, 0).data().toInt()
              , mView->model()->index(row, 1).data().toInt());

  mSearchField->setFocus();
}

/*
void SearchFiWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        mDragStartPosition = event->pos();
}

void SearchFiWidget::mouseMoveEvent(QMouseEvent* event)
{qDebug() << "mouseMoveEvent1";
    if (!(event->buttons() & Qt::LeftButton)) return;

    if ((event->pos() - mDragStartPosition).manhattanLength()
          < QApplication::startDragDistance())
        return;

qDebug() << "mouseMoveEvent2";
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;

    mimeData->setText("drag test");
    drag->setMimeData(mimeData);

    Qt::DropAction dropAction = drag->start(Qt::CopyAction | Qt::MoveAction);

}

bool SearchFiWidget::eventFilter(QObject* pFilterObj, QEvent* event)
{
  if((pFilterObj == mView))// && (event->type() == QEvent::MouseButtonPress))
  {
    //mousePressEvent(event);

  }
  qDebug() << "event" << pFilterObj << event->type();
  return QWidget::eventFilter(pFilterObj, event);
}
*/
/***********************************************************************
*
*
*
************************************************************************/
FiTypeSelBtn::FiTypeSelBtn(FClass* parent)
             : QToolButton(0)
             , FClass(parent, FUNC)
             , mType("")
{
  QMenu* menu = new QMenu(this);

  QActionGroup* actGroup = new QActionGroup(this);
  QAction* action = actGroup->addAction("All");
  action->setCheckable(true);
  action->setChecked(true);

  QStringList types = mFilu->getFiTypeNames();
  for(int i = 0; i < types.size(); ++i)
  {
    action = actGroup->addAction(types.at(i));
    action->setCheckable(true);
  }
  menu->addActions(actGroup->actions());

  connect(menu, SIGNAL(triggered(QAction *)), this, SLOT(selected(QAction *)));

  setPopupMode(QToolButton::InstantPopup);
  setMenu(menu);
  setAutoRaise(true);
  setArrowType(Qt::DownArrow);
}

FiTypeSelBtn::~FiTypeSelBtn()
{}

void FiTypeSelBtn::selected(QAction* action)
{
  mType = action->text();
  if(mType == "All") mType = "";
  emit selectionChanged();
}

QString FiTypeSelBtn::selected()
{
  return mType;
}
