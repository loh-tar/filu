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

#include "SymbolTableView.h"

#include "SymbolTuple.h"

/******************************************************************
*
*               SymbolTableModel::
*
*******************************************************************/

int SymbolTableModel::rowCount(const QModelIndex& parent) const
{
  if(!mSymbols) return 0;

  return mSymbols->count();
}

int SymbolTableModel::columnCount(const QModelIndex& parent) const
{
  return 5;
}

QVariant SymbolTableModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (index.column() >= 5 or index.row() >= mSymbols->count())
      return QVariant();

  if (role == Qt::DisplayRole)
  {
    switch(index.column())
      {
      case 0:
        return mSymbols->mFiId[index.row()];
      case 1:
        return mSymbols->mMarketId[index.row()];
      case 2:
        return mSymbols->mCaption[index.row()];
      case 3:
        return mSymbols->mMarket[index.row()];
      case 4:
        return mSymbols->mOwner[index.row()];
    }
  }

  return QVariant();

}

QVariant SymbolTableModel::headerData(
                            int section,
                            Qt::Orientation orientation,
                            int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal)
    switch(section)
      {
      case 0:
        return "FI-ID";
      case 1:
        return "Market-ID";
      case 2:
        return "Caption";
      case 3:
        return "Market";
      case 4:
        return "Owner";
    }
  else
      return QString("%1").arg(section);

 // FIXME: gcc Warnung: Kontrollfluss erreicht Ende einer Nicht-void-Funktion

}
/******************************************************************
*
*                         SymbolTableView::
*
*******************************************************************/
SymbolTableView::SymbolTableView(SymbolTuple* symbols, QWidget* parent)
               : QTableView(parent)
               , mSymbolTableModel(0)
{
  setContent(symbols);
}

SymbolTableView::~SymbolTableView()
{
  if(mSymbolTableModel) delete mSymbolTableModel;
}

void SymbolTableView::setContent(SymbolTuple* symbols)
{
  if(mSymbolTableModel) delete mSymbolTableModel;
  mSymbolTableModel = new SymbolTableModel(symbols, this);
  setModel(mSymbolTableModel);
  setColumnHidden(0, true);
  setColumnHidden(1, true);
  //resizeColumnsToContents();
  setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  setColumnWidth(0,200);
  setColumnWidth(1,300);
  setColumnWidth(2,110);
  setColumnWidth(3,110);
  setColumnWidth(4,110);
}

QSize SymbolTableView::sizeHint() const
{
  return QSize(380,200);
}
