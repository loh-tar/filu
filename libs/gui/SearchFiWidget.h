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

#ifndef SEARCHFIWIDGET_HPP
#define SEARCHFIWIDGET_HPP

#include <QPoint>
#include <QSqlQueryModel>
#include <QToolButton>

#include "FWidget.h"

class SearchField;
class SqlTableView;

/***********************************************************************
*
*
*
************************************************************************/
class FiTypeSelBtn : public QToolButton, public FClass
{
  Q_OBJECT

  public:
                FiTypeSelBtn(FClass* parent);
    virtual    ~FiTypeSelBtn();

    QString     selected();

    signals:
    void        selectionChanged();

  public slots:
    //void clicked(const QModelIndex& index);
    void        selected(QAction* action);

  protected:
    QString     mType;
};

/***********************************************************************
*
*
*
************************************************************************/

class SearchFiWidget : public FWidget
{
  Q_OBJECT

  public:
                SearchFiWidget(FClass* parent);
    virtual    ~SearchFiWidget();

    void        setHideNoMarket(bool hide = true);

    signals:
    void        selected(int fiId, int marketId);
    void        selected(QString symbol, QString market);

  public slots:
    void        search();
    void        clicked(const QModelIndex& index);

  protected:
    void        init();
    //void mousePressEvent(QMouseEvent* event);
    //void mouseMoveEvent(QMouseEvent* event);
    //bool eventFilter(QObject* pFilterObj, QEvent* pEvent);

    //QTableView*   mView;
    SqlTableView*   mView;
    QSqlQueryModel* mModel;
    SearchField*    mSearchField;
    FiTypeSelBtn*   mTypeSelBtn;
    QPoint          mDragStartPosition;
    int             mCurrentRow;
    bool            mHideNoMarket;
};

#endif
