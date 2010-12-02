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

#ifndef FIGROUPWIDGET_HPP
#define FIGROUPWIDGET_HPP

#include "FWidget.h"

class MyTableWidget;

/***********************************************************************
*
*
*
************************************************************************/
class FiGroupWidget : public FWidget
{
  Q_OBJECT

  public:
                    FiGroupWidget(FClass* parent);
    virtual        ~FiGroupWidget();

    void loadSettings();
    void saveSettings();

    signals:
    void selected(int fiId, int marketId);
    void selected(QString symbol, QString market);

  protected slots:
    void groupClicked(int row, int);
    void groupRowChanged(int newRow);
    void openGroup(int row, int);
    void memberClicked(int row, int);
    void memberRowChanged(int newRow);
    void groupUp();
    void newGroup();
    void groupEdited(int row, int /*column*/);

    void setActiveGroup(const QString& group);

    void getGroups(int groupId);
    void getGMembers(int groupId);


    void userDragInData(QTableView* tv);
    void removeFromGroup();
    void removeGroup();

  protected:
    MyTableWidget*  mGroupView;
    MyTableWidget*  mMemberView;
    QSplitter*      mSplitter;
    QLabel*         mCurrendGroup;
    int             mGroupViewedId;
    int             mCurrendGroupId;
    QStack<int>     mMotherIds;
    QStack<QString> mMotherNames;
    bool            mEditing;
};

/***********************************************************************
*
*
*
************************************************************************/
class MyTableWidget : public QTableWidget
{
  Q_OBJECT

  public:
                    MyTableWidget(QWidget* parent = 0);
    virtual        ~MyTableWidget();

    void erase();

  signals:
    void dragInFromTableView(QTableView*);
    void dragToNirvana();
    void currentRowChanged(int newRow);

  protected:
    void mousePressEvent(QMouseEvent* pEvent);
    void mouseMoveEvent(QMouseEvent* pEvent);
    void dropEvent(QDropEvent* pEvent);
    void dragEnterEvent(QDragEnterEvent* pEvent);
    void dragMoveEvent(QDragMoveEvent* pEvent);
    void startDrag();

    QPoint mDragStartPosition;
    int    mCurrentRow;

  protected slots:
    void currentItemChangedSlot(QTableWidgetItem*, QTableWidgetItem*);
    void itemClickedSlot(QTableWidgetItem*);
    void itemDoubleClickedSlot(QTableWidgetItem*);
};

#endif
