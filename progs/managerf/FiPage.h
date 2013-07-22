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

class QComboBox;
class QLineEdit;
class QModelIndex;
class QTabWidget;
class QToolButton;

#include "ManagerPage.h"

class IndicatorWidget;
class SearchFiWidget;
class SqlTableView;
class SymbolTableView;

class FiPage : public ManagerPage
{
  Q_OBJECT

  public:
                     FiPage(FClass* parent);
    virtual         ~FiPage();

  protected slots:
    void            fiClicked(int fiId, int marketId = 0);
    void            symbolClicked(const QModelIndex& index);
    void            lockFi();
    void            newSymbol();
    void            saveSymbol();
    void            deleteSymbol();
    void            saveFi();
    void            deleteFi();
    void            splitClicked(const QModelIndex & index);
    void            prePostEdited();
    void            newSplit();
    void            saveSplit();
    void            deleteSplit();

  protected:
    void            createPage();
    void            loadBars();

    // General Stuff
    QTabWidget*       mTab;
    FiTuple*          mFi;
    int               mMarketId;
    BarTuple*         mBars;
    SymbolTuple*      mSymbols;
    IndicatorWidget*  mPlotSheet;

    // Main Tab Stuff
    QWidget*          makeMainTab();
    void              setSymbolTable(int symbolId = 0);

    SearchFiWidget*   mLookUp;
    int               mSymbolId;
    QComboBox*        mProvider;
    QLineEdit*        mSymbol;
    QComboBox*        mMarket;
    SymbolTableView*  mSymbolView;
    QLineEdit*        mFiName;
    QComboBox*        mFiType;
    QLineEdit*        mExpiryDate;
    QToolButton*      mLockBtn;

    // Split Tab Stuff
    QWidget*          makeSplitTab();
    void              setSplitTable();

    SqlTableView*     mSplitView;
    QLineEdit*        mSplitDate;
    QLineEdit*        mRatio;
    QLineEdit*        mPrePost;
    QLineEdit*        mSplitComment;
    int               mSplitId;

    // EODBar Tab Stuff
    QWidget*          makeBarTab();

    // Chart Object Tab Stuff
    QWidget*          makeCoTab();
};

#endif
