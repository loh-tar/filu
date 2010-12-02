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

#include "FiGroupWidget.h"

FiGroupWidget::FiGroupWidget(FClass* parent)
             : FWidget(parent)
{
  mEditing = false;

  QPixmap icon(10, 10);

  mGroupView = new MyTableWidget(this);
  //mGroupView->setDragEnabled(false);
  //mGroupView->setAcceptDrops(false);
  mGroupView->setEditTriggers(QAbstractItemView::EditKeyPressed);

  connect(mGroupView, SIGNAL(currentRowChanged(int))
          , this, SLOT(groupRowChanged(int)));
  connect(mGroupView, SIGNAL(cellDoubleClicked(int, int))
          , this, SLOT(openGroup(int, int)));
  connect(mGroupView, SIGNAL(cellChanged(int, int))
          , this, SLOT(groupEdited(int, int)));
  connect(mGroupView, SIGNAL(dragToNirvana())
          , this, SLOT(removeGroup()));

  mCurrendGroup = new QLabel(this);
  QToolButton* btn = new QToolButton(this);
  btn->setAutoRaise(true);
  btn->setArrowType(Qt::UpArrow);
  btn->setToolTip("Up to Mother Group");
  connect(btn, SIGNAL(clicked(bool)), this, SLOT(groupUp()));

  QToolButton* newGroupBtn = new QToolButton(this);
  newGroupBtn->setAutoRaise(true);
  icon.fill(Qt::green);
  newGroupBtn->setIcon(icon);
  newGroupBtn->setToolTip("Add New Group");
  connect(newGroupBtn, SIGNAL(clicked(bool)), this, SLOT(newGroup()));

  //mCurrendGroup->setAlignment(Qt::AlignVertical_Mask);
  //mCurrendGroup->setOrientation(Qt::Vertical);
  QGridLayout* gbox = new QGridLayout;
  gbox->setMargin(0);
  gbox->addWidget(mCurrendGroup, 0, 0);
  gbox->addWidget(newGroupBtn, 0, 1);
  gbox->addWidget(btn, 0, 2);
  gbox->addWidget(mGroupView, 1, 0, 1, 3);
  QWidget* gboxw = new QWidget;
  gboxw->setLayout(gbox);

  mMemberView = new MyTableWidget;

  connect( mMemberView, SIGNAL(currentRowChanged(int))
          , this, SLOT(memberRowChanged(int)));
  connect( mMemberView, SIGNAL(dragInFromTableView(QTableView*))
          , this, SLOT(userDragInData(QTableView*)));
  connect(mMemberView, SIGNAL(dragToNirvana())
          , this, SLOT(removeFromGroup()));

  mSplitter = new QSplitter(Qt::Vertical);
  //mSplitter->addWidget(mGroupView);
  mSplitter->addWidget(gboxw);
  mSplitter->addWidget(mMemberView);
  mSplitter->setStretchFactor(0, 1);
  mSplitter->setStretchFactor(1, 3);

  QGridLayout* layout = new QGridLayout;
  layout->setMargin(0);
  layout->addWidget(mSplitter, 0, 0);
  //layout->addWidget(, 0, 1);

  setLayout(layout);

  getGroups(0);
}

FiGroupWidget::~FiGroupWidget()
{}

void FiGroupWidget::loadSettings()
{
  mSplitter->restoreState(mRcFile->value("SplitterSizes").toByteArray());
}

void FiGroupWidget::saveSettings()
{
  mRcFile->setValue("SplitterSizes", mSplitter->saveState());
}

void FiGroupWidget::groupClicked(int row, int)
{
  getGMembers(mGroupView->item(row, 0)->text().toInt());
}

void FiGroupWidget::groupRowChanged(int row)
{
  getGMembers(mGroupView->item(row, 0)->text().toInt());
}

void FiGroupWidget::openGroup(int row, int)
{
  mMotherNames.push(mCurrendGroup->text());
  mCurrendGroup->setText(mGroupView->item(row, 1)->text());
  mMotherIds.push(mGroupView->item(row, 2)->text().toInt());
  getGroups(mGroupView->item(row, 0)->text().toInt());
}

void FiGroupWidget::groupUp()
{
  if(mMotherIds.isEmpty()) return;

  getGroups(mMotherIds.pop());
  setActiveGroup(mCurrendGroup->text());
  mCurrendGroup->setText(mMotherNames.pop());
}

void FiGroupWidget::newGroup()
{
  mEditing = false;

  int row = mGroupView->rowCount();
  mGroupView->insertRow(row);

  QTableWidgetItem* item = new QTableWidgetItem;
  item->setText(QString::number(0));
  mGroupView->setItem(row, 0, item);

  item = new QTableWidgetItem;
  item->setText("");
  mGroupView->setItem(row, 1, item);
  mGroupView->editItem(item);
  mGroupView->setCurrentItem(item);

  item = new QTableWidgetItem;
  item->setText(QString::number(mCurrendGroupId));
  mGroupView->setItem(row, 2, item);

  mEditing = true;
}

void FiGroupWidget::groupEdited(int row, int /*column*/)
{
  if(!mEditing) return;

  mGroupView->resizeColumnsToContents();

  int groupId = mGroupView->item(row, 0)->text().toInt();
  QString name = mGroupView->item(row, 1)->text();
  int motherId = mGroupView->item(row, 2)->text().toInt();
//qDebug() << groupId << name << motherId;
  mFilu->putGroup(groupId, name, motherId);
  getGroups(mCurrendGroupId);
  setActiveGroup(name);
}

void FiGroupWidget::setActiveGroup(const QString& group)
{
  QList<QTableWidgetItem *> list = mGroupView->findItems(group, Qt::MatchExactly);
  if(list.isEmpty())
  {qDebug() << group << "nix gefunden!?"; return;}
  mGroupView->setCurrentItem(list.at(0));
  int row = mGroupView->currentRow();
  getGMembers(mGroupView->item(row, 0)->text().toInt());
}

void FiGroupWidget::memberClicked(int row, int)
{
  //qDebug() << "FiGroupWidget::memberClicked()" << row;
  emit selected(mMemberView->item(row, 2)->text()
              , mMemberView->item(row, 3)->text());

  emit selected(mMemberView->item(row, 1)->text().toInt()
              , mMemberView->item(row, 4)->text().toInt());
}

void FiGroupWidget::memberRowChanged(int row)
{
  //qDebug() << "FiGroupWidget::memberRowChanged()" << row;
  emit selected(mMemberView->item(row, 2)->text()           // Symbol
              , mMemberView->item(row, 3)->text());         // Market

  emit selected(mMemberView->item(row, 1)->text().toInt()   // FiId
              , mMemberView->item(row, 4)->text().toInt()); // MarketId
}

void FiGroupWidget::getGroups(int groupId)
{
  mEditing = false;
  mCurrendGroupId = groupId;

  QSqlQuery* query = mFilu->getGroups(groupId);

  mGroupView->erase();

  while(mGroupView->columnCount() < 3) mGroupView->insertColumn(0);

  if(!query)
  {
    getGMembers(groupId);
    return;
  }

  QTableWidgetItem* item;
  while(query->next())
  {
    int row = mGroupView->rowCount();
    mGroupView->insertRow(row);
    item = new QTableWidgetItem;
    item->setText(query->value(0).toString());
    mGroupView->setItem(row, 0, item);

    item = new QTableWidgetItem;
    item->setText(query->value(1).toString());
    mGroupView->setItem(row, 1, item);

    item = new QTableWidgetItem;
    item->setText(query->value(2).toString());
    mGroupView->setItem(row, 2, item);
  }

  setActiveGroup(mGroupView->item(0, 1)->text());

  QSqlRecord rec = query->record();
  QStringList header;
  for(int i = 0; i < rec.count(); ++i) header << rec.fieldName(i);

  mGroupView->setHorizontalHeaderLabels(header);
  mGroupView->hideColumn(0);
  mGroupView->hideColumn(2);
  mGroupView->resizeColumnsToContents();

  mEditing = true;
}

void FiGroupWidget::getGMembers(int groupId)
{
  if(groupId == 0) return;

  mGroupViewedId = groupId;

  QSqlQuery* query = mFilu->getGMembers(groupId);

  mMemberView->erase();

  while(mMemberView->columnCount() < 5) mMemberView->insertColumn(0);

  if(!query) return;

  QTableWidgetItem* item;
  while(query->next())
  {
    int row = mMemberView->rowCount();
    mMemberView->insertRow(row);

    item = new QTableWidgetItem;
    item->setText(query->value(0).toString());
    mMemberView->setItem(row, 0, item);

    item = new QTableWidgetItem;
    item->setText(query->value(1).toString());
    mMemberView->setItem(row, 1, item);

    item = new QTableWidgetItem;
    item->setText(query->value(2).toString());
    mMemberView->setItem(row, 2, item);

    item = new QTableWidgetItem;
    item->setText(query->value(3).toString());
    mMemberView->setItem(row, 3, item);

    item = new QTableWidgetItem;
    item->setText(query->value(4).toString());
    mMemberView->setItem(row, 4, item);
  }

  QSqlRecord rec = query->record();
  QStringList header;
  for(int i = 0; i < rec.count(); ++i) header << rec.fieldName(i);

  mMemberView->setHorizontalHeaderLabels(header);
  mMemberView->hideColumn(0);
  mMemberView->hideColumn(1);
  mMemberView->hideColumn(4);
  mMemberView->resizeColumnsToContents();
}

void FiGroupWidget::userDragInData(QTableView* tv)
{
  //if(tv == mMemberView) return;

  QAbstractItemModel* m = tv->horizontalHeader()->model();

  int fiIdColumn = 0;
  for(; fiIdColumn < tv->horizontalHeader()->count(); ++fiIdColumn)
  {
    //qDebug() << m->headerData(i, Qt::Horizontal).toString();
    if(m->headerData(fiIdColumn, Qt::Horizontal).toString() == "fi_id") break;
  }

  QModelIndexList mil = tv->selectionModel()->selectedIndexes();
  for(int i = 0; i< mil.size(); ++i)
  {
    if(mil.at(i).column() != fiIdColumn) continue;
    //qDebug() << "mil:" << mil.at(i).row() << mil.at(i).column() << mil.at(i).data().toString();
    mFilu->addToGroup(mGroupViewedId, mil.at(i).data().toInt());
  }

  getGMembers(mGroupViewedId);
}

void FiGroupWidget::removeFromGroup()
{
  QModelIndexList mil = mMemberView->selectionModel()->selectedIndexes();
  for(int i = 0; i< mil.size(); ++i)
  {
    if(mil.at(i).column() != 0) continue;
    //qDebug() << "mil:" << mil.at(i).row() << mil.at(i).column() << mil.at(i).data().toString();
    mFilu->deleteRecord(":user", "gmember", mil.at(i).data().toInt());
  }

  getGMembers(mGroupViewedId);
}

void FiGroupWidget::removeGroup()
{
  //qDebug() << "removeGroup";
  QModelIndexList mil = mGroupView->selectionModel()->selectedIndexes();

  for(int i = 0; i< mil.size(); ++i)
  {
    if(mil.at(i).column() != 0) continue;
    QString message = tr("\nAre you sure to delete '<group>' and all its contents?.\n");
    message.replace("<group>", mGroupView->item(mil.at(i).row(), 1)->text());

    int ret = QMessageBox::question(this, tr("performerf - question box"),
                   message,
                   QMessageBox::Yes | QMessageBox::No);

    if(ret == QMessageBox::Yes)
      mFilu->deleteRecord(":user", "group", mil.at(i).data().toInt());
  }

  if(mMotherIds.isEmpty()) getGroups(0);
  else getGroups(mMotherIds.top());
}

MyTableWidget::MyTableWidget(QWidget* parent)
             : QTableWidget(parent)
{
  setDragEnabled(true);
  setAcceptDrops(true);
  horizontalHeader()->hide();
  verticalHeader()->hide();
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setHorizontalScrollMode(ScrollPerPixel);
  setVerticalScrollMode(ScrollPerPixel);
  setEditTriggers(QAbstractItemView::NoEditTriggers);

  connect(this, SIGNAL(currentItemChanged(QTableWidgetItem *, QTableWidgetItem *))
  , this, SLOT(currentItemChangedSlot(QTableWidgetItem *, QTableWidgetItem *)));

  connect(this, SIGNAL(itemClicked(QTableWidgetItem *))
  , this, SLOT(itemClickedSlot(QTableWidgetItem *)));

  connect(this, SIGNAL(itemDoubleClicked(QTableWidgetItem *))
  , this, SLOT(itemDoubleClickedSlot(QTableWidgetItem *)));
}

MyTableWidget::~MyTableWidget()
{}

void MyTableWidget::erase()
{
  disconnect(this, SIGNAL(currentItemChanged(QTableWidgetItem *, QTableWidgetItem *))
  , this, SLOT(currentItemChangedSlot(QTableWidgetItem *, QTableWidgetItem *)));

  while(rowCount()) removeRow(0);

  mCurrentRow = -1; // Mark as unvalid

  connect(this, SIGNAL(currentItemChanged(QTableWidgetItem *, QTableWidgetItem *))
  , this, SLOT(currentItemChangedSlot(QTableWidgetItem *, QTableWidgetItem *)));
}

void MyTableWidget::mousePressEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    mDragStartPosition = event->pos();
  }
  QTableWidget::mousePressEvent(event);
}

void MyTableWidget::mouseMoveEvent(QMouseEvent* event)
{
  if(event->buttons() & Qt::LeftButton)
  {
    int distance = (event->pos() - mDragStartPosition).manhattanLength();
    if(distance > QApplication::startDragDistance())
      startDrag();
  }
  else QTableWidget::mouseMoveEvent(event);
}

void MyTableWidget::startDrag()
{
  QMimeData* mimeData = new QMimeData;
  mimeData->setData("application/Filu_MyTableWidget", "");
  QDrag* drag = new QDrag( this );
  drag->setMimeData( mimeData );
  Qt::DropAction action = drag->start(Qt::ActionMask);
  if(action == Qt::CopyAction)
  {
    //qDebug() << "MyTableWidget-target was:" << drag->target();
  }
  else
  {
    //qDebug() << "MyTableWidget-target else:" << drag->target();
    if(!drag->target()) emit dragToNirvana();
  }
}

void MyTableWidget::dropEvent( QDropEvent* event )
{
  //qDebug() << "dropEvent" << event->mimeData()->formats() << "from:" << event->source();
  if(event->mimeData()->hasFormat("application/Filu_SqlTableView")
    or event->mimeData()->hasFormat("application/Filu_MyTableWidget"))
  {
    //qDebug() << "MyTableWidget::dropEvent" << event->mimeData()->text();
    //event->setDropAction( Qt::CopyAction );
    event->accept();
    //event->acceptProposedAction();
    //qDebug() << ":" << (static_cast<QTableView*>(event->source()))->model()->headerData(0, Qt::Horizontal);
    //qDebug() << "event->dropAction() =" << event->dropAction();
    if(event->source() != this) emit dragInFromTableView(static_cast<QTableView*>(event->source()));
  }
  else
  {
    //qDebug() << "MyTableWidget::dropEvent, ignored";
    //event->ignore();
    event->accept();
  }
}

void MyTableWidget::dragEnterEvent(QDragEnterEvent* event)
{
  if(event->mimeData()->hasFormat("application/Filu_SqlTableView")
     or event->mimeData()->hasFormat("application/Filu_MyTableWidget"))
  {
    event->setDropAction(Qt::CopyAction);
    event->accept();
    //qDebug() << "MyTableWidget::dragEnterEvent";
  }
  else
  {
    //qDebug() << "MyTableWidget::dragEnterEvent, ignored";
    event->ignore();
  }
}

void MyTableWidget::dragMoveEvent(QDragMoveEvent* event)
{
//qDebug() << "MyTableWidget::dragMoveEvent";
  if(event->source() != this)
  {
    event->setDropAction(Qt::CopyAction);
    event->accept();
  }
  else
  {
    //qDebug() << "MyTableWidget::dragMoveEvent";
    event->setDropAction(Qt::CopyAction);
    event->accept();
  }
}

void MyTableWidget::currentItemChangedSlot(QTableWidgetItem* current, QTableWidgetItem* previous)
{
  if(NULL == current)
  {
    //qDebug() << "MyTableWidget::currentItemChangedSlot current is NULL";
    return;
  }

  if(NULL == previous)
  {
    //qDebug() << "MyTableWidget::currentItemChangedSlot" << current->row() << "NULL";
    return;
  }
  //else qDebug() << "MyTableWidget::currentItemChangedSlot" << current->row() << previous->row();

  if(current->row() != mCurrentRow)
  {
    mCurrentRow = current->row();
    emit currentRowChanged(mCurrentRow);
  }
}

void MyTableWidget::itemClickedSlot(QTableWidgetItem* item)
{
  //qDebug() << "MyTableWidget::itemClickedSlot" << item->row();
  if(item->row() != mCurrentRow)
  {
    mCurrentRow = item->row();
    emit currentRowChanged(mCurrentRow);
  }
}

void MyTableWidget::itemDoubleClickedSlot(QTableWidgetItem* item)
{
  emit currentRowChanged(item->row());
}
