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

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVBoxLayout>

#include "FiPage.h"

#include "FiTuple.h"
#include "FiluU.h"
#include "IndicatorWidget.h"
#include "SearchFiWidget.h"
#include "SymbolTableView.h"
#include "SymbolTuple.h"

FiPage::FiPage(FClass* parent)
      : ManagerPage(parent, FUNC)
      , mSymbols(0)
      , mSymbolView(0)
      , mFi(0)
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

  mSymbolView = new SymbolTableView(mSymbols);
  connect(mSymbolView, SIGNAL(clicked(const QModelIndex &)),
          this, SLOT(symbolClicked(const QModelIndex &)));

  mLookUp = new SearchFiWidget(this);
  connect(mLookUp, SIGNAL(selected(int, int)),
          this, SLOT(fiClicked(int, int)));

  mPlotSheet = new IndicatorWidget("FiPage", this);

  QGridLayout* gridLayout = new QGridLayout;
  int i = 0; // "count" rows
  gridLayout->setColumnStretch(1, 1);
  gridLayout->setColumnStretch(2, 1);

  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->setMargin(0);
  hbox->addWidget(mLookUp);
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
  mLookUp->setFocus();
}

void FiPage::fiClicked(int fiId, int /* marketId */)
{
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
}

