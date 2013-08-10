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

#include <QHeaderView>

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
  return 6;
}

QVariant SymbolTableModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (index.column() > 5 or index.row() >= mSymbols->count())
      return QVariant();

  if (role == Qt::DisplayRole)
  {
    switch(index.column())
      {
      case 0:
        return mSymbols->mCaption[index.row()];
      case 1:
        return mSymbols->mMarket[index.row()];
      case 2:
        return mSymbols->mOwner[index.row()];
      case 3:
        return mSymbols->mFiId[index.row()];
      case 4:
        return mSymbols->mMarketId[index.row()];
      case 5:
        return mSymbols->mId[index.row()];
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
        return "Caption";
      case 1:
        return "Market";
      case 2:
        return "Owner";
      case 3:
        return "FiId";
      case 4:
        return "MarketId";
      default: // case 5: // Don't gcc Warnung: Kontrollfluss erreicht Ende einer Nicht-void-Funktion
        return "SymbolId";
    }
  else
      return QString("%1").arg(section);
}
/******************************************************************
*
*                         SymbolTableView::
*
*******************************************************************/
SymbolTableView::SymbolTableView(SymbolTuple* symbols, QWidget* parent)
               : QTableView(parent)
               , mSymbolTableModel(0)
               , mCurrentRow(-1)
{

  connect(this, SIGNAL(clicked(const QModelIndex &)),
          this, SLOT(click(const QModelIndex &)));

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
  horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  setColumnHidden(3, true);
  setColumnHidden(4, true);
  setColumnHidden(5, true);
  verticalHeader()->hide();
//   resizeColumnsToContents();
  resizeRowsToContents();
  setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  setColumnWidth(0,110);
  setColumnWidth(1,110);
  setColumnWidth(2,110);
//   setColumnWidth(3,110);
//   setColumnWidth(4,110);
//   setColumnWidth(5,110);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  mCurrentRow = -1;
}

QSize SymbolTableView::sizeHint() const
{
  return QSize(380,200);
}

void SymbolTableView::selectSymbol(int id)
{
  // Select the row containing the given SymbolId
  QAbstractItemModel* m = model();
  int r = 0; // Row
  int c = 5; // Column (with symbol id)
  while(m->hasIndex(r, c))
  {
    if(id == m->data(m->index(r, c)).toInt())
    {
      selectRow(r);
      emit clicked(m->index(r, c));
      break;
    }
    ++r;
  }
}

void SymbolTableView::clearSelection()
{
  QTableView::clearSelection();
  mCurrentRow = -1;
}

void SymbolTableView::currentChanged(const QModelIndex& current,
                                   const QModelIndex &/*previous*/)
{
  if(current.row() == mCurrentRow) return;
  if(current.row() == -1) return;

  mCurrentRow = current.row();
  emit newSelection(current);

  scrollTo(current);
}

void SymbolTableView::click(const QModelIndex& current)
{
  if(current.row() == mCurrentRow) return;
  mCurrentRow = current.row();
  emit newSelection(current);
}
