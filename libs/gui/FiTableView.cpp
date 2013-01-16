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

#include "FiTableView.h"

#include "FiTuple.h"

/******************************************************************
*
*               FiTableModel::
*
*******************************************************************/

FiTableModel::FiTableModel(FiTuple* fi, QObject* parent)
            : QAbstractTableModel(parent)
            , mFi(fi)
{}

int FiTableModel::rowCount(const QModelIndex& parent) const
{
  if(!mFi) return 0;

  return mFi->count();
}

int FiTableModel::columnCount(const QModelIndex& parent) const
{
  return 4;
}

QVariant FiTableModel::data(const QModelIndex& index, int role) const
{

  if (!index.isValid())
    return QVariant();

  if (index.column() >= 4 or index.row() >= mFi->count())
      return QVariant();

  if (role == Qt::DisplayRole)
  {
    switch(index.column())
    {
      case 0:
        return mFi->mId[index.row()];
      case 1:
        return mFi->mTypeId[index.row()];
      case 2:
        return mFi->mName[index.row()];
      case 3:
        return mFi->mType[index.row()];
    }
   }
  return QVariant();

}

QVariant FiTableModel::headerData(
                            int section,
                            Qt::Orientation orientation,
                            int role) const
{

  if(orientation == Qt::Horizontal)
  {

    if(role == Qt::DisplayRole)
    {
      switch(section)
      {
        case 0:
          return "FI-ID";
        case 1:
          return "Type-ID";
        case 2:
          return "Name";
        case 3:
          return "Type";
        // case 4:
        //   return "Owner";
      }
    }
  }

 /* if(role == Qt::SizeHintRole and orientation == Qt::Horizontal)
  {
    switch(section)
    {
      case 0:
        return QSize(50,0);
      case 1:
        return QSize(50,0);
      case 2:
        {
          QFont font = ((QTableView*)(static_cast<QObject*>(this))->parent())->font();
          QFontMetrics fm(font);
          qDebug() << "fm" << fm.size(0, QString(30,'X'));
          return fm.size(0,QString(30,'X'));
        }
      case 3:
        {
          QFont font = ((QTableView*)(static_cast<QObject*>(this))->parent())->font();
          QFontMetrics fm(font);
          qDebug() << "fm" << fm.size(0, QString(10,'X'));
          return fm.size(0,QString(10,'X'));
        }
      // case 4:
      //   return "Owner";
    }
  }*/
  else
  {
    if (role == Qt::DisplayRole) return QString("%1").arg(section);
  }

  return QVariant();

}

/******************************************************************
*
*                         FiTableView::
*
*******************************************************************/
FiTableView::FiTableView(FiTuple* fi, QWidget* parent)
               : QTableView(parent)
               , mFiModel(0)
{
  setContent(fi);
}

FiTableView::~FiTableView()
{
  if(mFiModel) delete mFiModel;
}

void FiTableView::setContent(FiTuple* fi)
{
  if(mFiModel) delete mFiModel;
  mFiModel = new FiTableModel(fi, this);
  setModel(mFiModel);
  setColumnHidden(0, true);
  setColumnHidden(1, true);
  //resizeColumnsToContents();
  setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  setColumnWidth(0,200);
  setColumnWidth(1,300);
  setColumnWidth(2,250);
  setColumnWidth(3,80);
}

QSize FiTableView::sizeHint() const
{
  return QSize(380,200);
}
