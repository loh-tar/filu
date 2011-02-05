//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011  loh.tar@googlemail.com
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

#ifndef FIPAGE_H
#define FIPAGE_H

#include "ManagerPage.h"

class IndicatorWidget;
class SearchField;
class SymbolTableView;
class FiTableView;

class FiPage : public ManagerPage
{
  Q_OBJECT

  public:
                     FiPage(FClass* parent);
    virtual         ~FiPage();

  protected:
    SearchField*     mLook4Edit;
    void             createPage();
    void             showEvent(QShowEvent* /*event*/);

    SymbolTuple*     mSymbols;
    SymbolTableView* mSymbolView;
    FiTuple*         mFi;
    FiTableView*     mFiView;
    IndicatorWidget* mPlotSheet;
    BarTuple*        mBars;

  protected slots:
    void            search();
    void            fiClicked(const QModelIndex&);
    void            symbolClicked(const QModelIndex&);
};

#endif
