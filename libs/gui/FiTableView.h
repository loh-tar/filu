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

#ifndef FITABLEMODEL_HPP
#define FITABLEMODEL_HPP

#include <QTableView>

class FiTuple;

class FiTableModel : public QAbstractTableModel
{
  //Q_OBJECT

  public:
                  FiTableModel(FiTuple* fi, QObject* parent = 0);
    virtual      ~FiTableModel() {}

    int           rowCount(const QModelIndex& parent = QModelIndex()) const;
    int           columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant      data(const QModelIndex& index, int role) const;

    QVariant      headerData(int section, Qt::Orientation orientation,
                           int role = Qt::DisplayRole) const;

  private:
    FiTuple*      mFi;
};

class FiTableView : public QTableView
{
  //Q_OBJECT

  public:
                  FiTableView(FiTuple* fi, QWidget* parent = 0);
   virtual       ~FiTableView();

   void           setContent(FiTuple* fi);
   QSize          sizeHint() const;

  private:
    FiTableModel* mFiModel;
};

#endif
