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

#include "FiPage.h"

#include "IndicatorWidget.h"
#include "SearchField.h"
#include "SymbolTableView.h"
#include "FiTableView.h"

FiPage::FiPage(FClass* parent)
      : ManagerPage(parent)
      , mSymbols(0)
      , mSymbolView(0)
      , mFi(0)
      , mFiView(0)
      , mPlotSheet(0)
      , mBars(0)
{
  mIcon = QIcon(":/icons/fi-page.png");
  mIconText = tr("FI Details");
  createPage();
}

FiPage::~FiPage()
{
  if(mFi) delete mFi;
  if(mSymbols) delete mSymbols;
}

void FiPage::createPage()
{
  QGroupBox* fiGroup = new QGroupBox(tr("Look for FI"));

  QLabel* lookLabel = new QLabel(tr("Name or Symbol"));

  mLook4Edit = new SearchField(this);
  connect(mLook4Edit, SIGNAL(textChanged()), this, SLOT(search()));

  mSymbolView = new SymbolTableView(mSymbols);
  connect(mSymbolView, SIGNAL(clicked(const QModelIndex &)),
          this, SLOT(symbolClicked(const QModelIndex &)));

  mFiView = new FiTableView(mFi);
  connect(mFiView, SIGNAL(clicked(const QModelIndex &)),
          this, SLOT(fiClicked(const QModelIndex &)));

  mPlotSheet = new IndicatorWidget("FiPage", this);

  QGridLayout* gridLayout = new QGridLayout;
  int i = 0; // "count" rows
  gridLayout->addWidget(lookLabel, i, 0);
  gridLayout->addWidget(mLook4Edit, i, 1);
 // gridLayout->addWidget(new QPushButton, i, 3);
  gridLayout->setColumnStretch(1, 1);
  gridLayout->setColumnStretch(2, 1);

  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->setMargin(0);
  hbox->addWidget(mFiView);
  hbox->addWidget(mSymbolView);

  QWidget* hboxwid = new QWidget;

  hboxwid->setLayout(hbox);

  QSplitter* splitt1 = new QSplitter(Qt::Vertical);
  splitt1->addWidget(hboxwid);
  splitt1->addWidget(mPlotSheet);

  gridLayout->addWidget(splitt1, ++i, 0, 1, 4);

  fiGroup->setLayout(gridLayout);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(fiGroup);
  setLayout(mainLayout);
}

void FiPage::showEvent(QShowEvent * /*event*/)
{
  mLook4Edit->setFocus();
}

void FiPage::search()
{
  QString text = mLook4Edit->text();
  if(text.isEmpty()) return;

  mFilu->setMarketName("");
  mFilu->setProviderName("");
  mFilu->setFiId(0);
  mFilu->setFiType("");
  mFilu->setSymbolCaption(text);
  mFilu->setOnlyProviderSymbols(false);
  SymbolTuple* symbols;
  symbols = mFilu->getSymbols();
  if(symbols)
  {
    symbols->next();
    mFilu->setFiId(symbols->fiId());
    if(mFi) delete mFi;
    mFi = mFilu->getFi();
    mFiView->setContent(mFi);

    if(mSymbols) delete mSymbols;
    mSymbols = mFilu->getSymbols(symbols->fiId());
    mSymbolView->setContent(mSymbols);

    delete symbols;
  }
  else
  {
    mFilu->setFiName(text);
    //mFilu->setFiType("");
    if(mFi) delete mFi;
    mFi = mFilu->getFi(true);
    mFiView->setContent(mFi);

    if(mSymbols) delete mSymbols;
    mSymbols = 0;
    mSymbolView->setContent(mSymbols);

  }

}

void FiPage::fiClicked(const QModelIndex& index)
{
  static int oldFiId = -1;

  int fiId = index.sibling(index.row(), 0).data().toInt();

  if(oldFiId == fiId) return;

  oldFiId = fiId;

  if(mSymbols) delete mSymbols;
  mSymbols = mFilu->getSymbols(fiId);
  mSymbolView->setContent(mSymbols);
}

void FiPage::symbolClicked(const QModelIndex& index)
{
  static int oldFiId = -1;
  static int oldMarketId = -1;

  int fiId = index.sibling(index.row(), 0).data().toInt();
  int marketId = index.sibling(index.row(), 1).data().toInt();

  if(oldFiId == fiId and oldMarketId == marketId) return;

  oldFiId = fiId;
  oldMarketId = marketId;

  mPlotSheet->showFiIdMarketId(fiId, marketId);
/*
  if(mBars) delete mBars;
  mFilu->setFromDate("1900-01-01");
  mFilu->setFiId(fiId);
  mFilu->setMarketId(marketId);
  mBars = mFilu->getBars();
  mPlotSheet->useBarData(mBars);
  */
}

