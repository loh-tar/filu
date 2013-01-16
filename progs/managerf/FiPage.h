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

#ifndef FIPAGE_H
#define FIPAGE_H

class QModelIndex;

#include "ManagerPage.h"

class IndicatorWidget;
class SearchFiWidget;
class SymbolTableView;

class FiPage : public ManagerPage
{
  Q_OBJECT

  public:
                     FiPage(FClass* parent);
    virtual         ~FiPage();

  protected:
    void             createPage();
    void             showEvent(QShowEvent* /*event*/);

    SearchFiWidget*  mLookUp;
    SymbolTuple*     mSymbols;
    SymbolTableView* mSymbolView;
    FiTuple*         mFi;
    IndicatorWidget* mPlotSheet;
    BarTuple*        mBars;

  protected slots:
    void            fiClicked(int fiId, int marketId);
    void            symbolClicked(const QModelIndex&);
};

#endif
