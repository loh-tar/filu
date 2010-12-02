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

#ifndef FITUPLE_HPP
#define FITUPLE_HPP

#include <QTableView>

#include "Tuple.h"

class SymbolTuple;

class FiTuple : public Tuple
{
  public:
                  FiTuple(int size);
    virtual      ~FiTuple();

    int           id();
    int           typeId();
    QString       type();
    QString       name();
    SymbolTuple*  symbol();
    //QString issueDate();
    //QString maturityDate();

    void          setId(int id);
    void          setTypeId(int type);
    void          setType(const QString& type);
    void          setName(const QString& name);
    void          setSymbol(SymbolTuple* symbol);

    friend class  Filu;
    friend class  FiTableModel;

  private:
    // Holds the beef
    int*          Id;
    int*          TypeId;
    QString*      Type;
    QString*      Name;
    SymbolTuple** Symbol;
    //QString* IssueDate;
    //QString* MaturityDate;
};

class FiTableModel : public QAbstractTableModel
{
  Q_OBJECT

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
  Q_OBJECT

  public:
                  FiTableView(FiTuple* fi, QWidget* parent = 0);
   virtual       ~FiTableView();

   void           setContent(FiTuple* fi);
   QSize          sizeHint() const;

  private:
    FiTableModel* mFiModel;
};

#endif
