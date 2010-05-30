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

#ifndef SYMBOLTUPLE_HPP
#define SYMBOLTUPLE_HPP

#include <QTableView>

#include "Tuple.h"

class SymbolTuple : public Tuple
{
  public:
                SymbolTuple(int size);
    virtual    ~SymbolTuple();

//    int id();
//    int typeId();
    int         fiId();
    int         marketId();
    QString     caption();
//    QString issueDate();
//    QString maturityDate();
    QString     market();
    QString     owner();

    void        setFiId(const int id);
    void        setMarketId(const int id);
    void        setCaption(const QString &);
    void        setMarket(const QString &);
    void        setOwner(const QString &);

    // holds the beef
//    int* SymbolId;
//    int* StypeId;
    int*        FiId;
    int*        MarketId;
    QString*    Caption;
//    QString*  Issuedate;
//    QString*  Maturitydate;
    QString*    Market;
    QString*    Owner;
};

class SymbolTableModel : public QAbstractTableModel
{
  Q_OBJECT

  public:
                SymbolTableModel(SymbolTuple* symbols, QObject* parent = 0)
                    : QAbstractTableModel(parent)
                    , mSymbols(symbols) {}

    virtual    ~SymbolTableModel() {}

    int         rowCount(const QModelIndex& parent = QModelIndex()) const;
    int         columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant    data(const QModelIndex& index, int role) const;

    QVariant    headerData(int section, Qt::Orientation orientation,
                           int role = Qt::DisplayRole) const;

  private:
    SymbolTuple* mSymbols;
};

class SymbolTableView : public QTableView
{
  Q_OBJECT

  public:
                SymbolTableView(SymbolTuple* symbols, QWidget* parent = 0);
    virtual    ~SymbolTableView();

    void        setContent(SymbolTuple* symbols);
    QSize       sizeHint() const;

  private:
    SymbolTableModel* mSymbolTableModel;
};

#endif
