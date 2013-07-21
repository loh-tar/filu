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

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSplitter>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTabWidget>
#include <QVBoxLayout>

#include "FiPage.h"

#include "BarTuple.h"
#include "FiTuple.h"
#include "FiluU.h"
#include "IndicatorWidget.h"
#include "MarketTuple.h"
#include "SearchFiWidget.h"
#include "SymbolTableView.h"
#include "SymbolTuple.h"
#include "SymbolTypeTuple.h"

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
  mTab = new QTabWidget;
  mTab->setTabPosition(QTabWidget::South);
//   connect(mTab, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

  mTab->addTab(makeMainTab(),   tr("Main"));
  mTab->addTab(makeSplitTab(),  tr("Splits"));
  mTab->addTab(makeCoTab(),     tr("Chart Objects"));
  mTab->addTab(makeBarTab(),    tr("EODBars"));

  mPlotSheet = new IndicatorWidget("FiPage", this);

  QSplitter* splitter = new QSplitter(Qt::Vertical);
  splitter->addWidget(mTab);
  splitter->addWidget(mPlotSheet);

  QVBoxLayout* vbox = new QVBoxLayout;
  vbox->addWidget(splitter);

  mMainBox = new QGroupBox(tr("Choose a FI to view and edit the details"));
  mMainBox->setLayout(vbox);

  vbox = new QVBoxLayout;
  vbox->addWidget(mMainBox);
  setLayout(vbox);
}

QWidget* FiPage::makeMainTab()
{
  // Start on the right with the symbol area
  mProvider = new QComboBox;
  mProvider->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  mSymbol = new QLineEdit;
  mMarket = new QComboBox;
  mMarket->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  QToolButton*  saveBtn;
  saveBtn = new QToolButton;
  saveBtn->setToolTip(tr("Save Symbol Changes"));
  saveBtn->setAutoRaise(true);
  saveBtn->setIcon(QIcon::fromTheme("document-save"));
//   saveBtn->setShortcut(QKeySequence(QKeySequence::Save));
  connect(saveBtn, SIGNAL(clicked()), this, SLOT(saveSymbol()));

  QToolButton*  delBtn;
  delBtn = new QToolButton;
  delBtn->setToolTip(tr("Delete Symbol"));
  delBtn->setAutoRaise(true);
  delBtn->setIcon(QIcon::fromTheme("edit-delete"));
//   delBtn->setShortcut(QKeySequence("Ctrl+Del"));
  connect(delBtn, SIGNAL(clicked()), this, SLOT(deleteSymbol()));

  QHBoxLayout* hbox = new QHBoxLayout; // Symbol Edit Line
  hbox->setMargin(0);
  hbox->addWidget(saveBtn);
  hbox->addWidget(mSymbol);
  hbox->addWidget(mMarket);
  hbox->addWidget(mProvider);
  hbox->addWidget(delBtn);
  hbox->addStretch(1);

  mSymbolView = new SymbolTableView(mSymbols);
  connect(mSymbolView, SIGNAL(clicked(const QModelIndex &)),
          this, SLOT(symbolClicked(const QModelIndex &)));

  QVBoxLayout* vbox = new QVBoxLayout; // That's all with the symbols..
  vbox->setMargin(0);
  vbox->addLayout(hbox);
  vbox->addWidget(mSymbolView);

  hbox = new QHBoxLayout;              // ..but add a stretch to the right
  hbox->setMargin(0);
  hbox->addLayout(vbox);
  hbox->addStretch(1);

  QWidget* widget = new QWidget;       // Place all on a widget needed by the splitter
  widget->setLayout(hbox);

  mLookUp = new SearchFiWidget(this);  // No more to do on the left
  connect(mLookUp, SIGNAL(selected(int, int)),
          this, SLOT(fiClicked(int, int)));

  QSplitter* splitter = new QSplitter(Qt::Horizontal);
  splitter->addWidget(mLookUp);
  splitter->addWidget(widget);

  // Build the FI edit line
  saveBtn = new QToolButton;
  saveBtn->setToolTip(tr("Save FI Changes"));
  saveBtn->setAutoRaise(true);
  saveBtn->setIcon(QIcon::fromTheme("document-save"));
//   saveBtn->setShortcut(QKeySequence(QKeySequence::Save));
  connect(saveBtn, SIGNAL(clicked()), this, SLOT(saveFi()));

  delBtn = new QToolButton;
  delBtn->setToolTip(tr("Delete FI"));
  delBtn->setAutoRaise(true);
  delBtn->setIcon(QIcon::fromTheme("edit-delete"));
//   delBtn->setShortcut(QKeySequence("Ctrl+Del"));
  connect(delBtn, SIGNAL(clicked()), this, SLOT(deleteFi()));

  QToolButton* mLockBtn = new QToolButton;
  mLockBtn->setToolTip(tr("Lock FI, don't download bars"));
  mLockBtn->setAutoRaise(true);
  mLockBtn->setIcon(QIcon::fromTheme("object-unlocked"));
//   mLockBtn->setShortcut(QKeySequence(QKeySequence::Save));
  connect(mLockBtn, SIGNAL(clicked()), this, SLOT(lockFi()));

  QFontMetrics f(font());
  int w = f.width("X");
  mFiName = new QLineEdit;
  mFiName->setToolTip(tr("FI Name"));
  mFiName->setMinimumWidth(w * 40);

  mFiType = new QComboBox;
  mFiType->setSizeAdjustPolicy(QComboBox::AdjustToContents);
//   mFiType->addItems(mFilu->getFiTypeNames());
//   mFiType->setCurrentIndex(-1);
  mFiType->setToolTip(tr("FI Type"));

  mExpiryDate = new QLineEdit;
  mExpiryDate->setToolTip(tr("FI Expiry Date"));
  mExpiryDate->setMinimumWidth(w * 15);

  hbox = new QHBoxLayout;
  hbox->addWidget(saveBtn);
  hbox->addWidget(mFiName);
  hbox->addWidget(mFiType);
  hbox->addWidget(mLockBtn);
  hbox->addWidget(mExpiryDate);
  hbox->addWidget(delBtn);
  hbox->addStretch(1);

  widget = new QWidget;
  widget->setLayout(hbox);

  vbox = new QVBoxLayout;
  vbox->addWidget(splitter); // SearchFiWidget + SymbolTableView
  vbox->addWidget(widget);   // FI edit line
  vbox->setStretch(0, 1);

  widget = new QWidget;      // Complete Main Tab Widget
  widget->setLayout(vbox);

  mFocusWidget = mLookUp;

  return widget;
}

QWidget* FiPage::makeSplitTab()
{

  return new QWidget;
}

QWidget* FiPage::makeBarTab()
{
  return new QWidget;
}

QWidget* FiPage::makeCoTab()
{
  return new QWidget;
}

void FiPage::loadBars()
{
  if(mBars) delete mBars;
  mBars = mFilu->getBars(mFi->id(), mMarketId);
  mPlotSheet->showBarData(mBars);
}

void FiPage::fiClicked(int fiId, int /* marketId */)
{
  if(mFi) delete mFi;
  mFi = mFilu->getFi(fiId);
  mMainBox->setTitle(QString("%1 (%2)").arg(mFi->name(), mFi->type()));

  mFiType->clear();
  mFiType->addItems(mFilu->getFiTypeNames());

  mFiName->setText(mFi->name());
  mFiType->setCurrentIndex(mFiType->findText(mFi->type()));
  mExpiryDate->setText(mFi->expiryDate().toString(Qt::ISODate));

  setSymbolTable();
}

void FiPage::setSymbolTable()
{
  if(mSymbols) delete mSymbols;
  mSymbols = 0;
  mSymbolId = 0;

  if(mFi) mSymbols = mFilu->getSymbols(mFi->id());

  mSymbolView->setContent(mSymbols);

  if(!mSymbols) return; // Should never happens, but anyway

  // Search a symbol with a market != NoMarket
  mSymbols->rewind();
  bool found = false;
  while(mSymbols->next())
  {
    if(mSymbols->marketId() != 1)
    {
      found = true;
      break;
    }
  }

  if(!found)
  {
    // Hm, set to first symbol
    mSymbols->rewind();
    mSymbols->next();
  }

  mSymbolView->selectSymbol(mSymbols->id());
}

void FiPage::symbolClicked(const QModelIndex& index)
{
  mMarketId = index.sibling(index.row(), 4).data().toInt();
  mSymbolId = index.sibling(index.row(), 5).data().toInt();

  mProvider->clear();
  mProvider->addItems(mFilu->getSymbolTypeNames());
  mProvider->setCurrentIndex(mProvider->findText(index.sibling(index.row(), 2).data().toString()));

  mSymbol->setText(index.sibling(index.row(), 0).data().toString());

  mMarket->clear();
  mMarket->addItems(mFilu->getMarketNames());
  mMarket->setCurrentIndex(mMarket->findText(index.sibling(index.row(), 1).data().toString()));

  loadBars();
}

void FiPage::lockFi()
{
  if("3000-01-01" == mExpiryDate->text())
  {
    if(!mBars) return;
    mBars->rewind(mBars->size() - 1);
    mExpiryDate->setText(mBars->date().toString(Qt::ISODate));
  }
  else
  {
    mExpiryDate->setText("3000-01-01");
  }
}

void FiPage::saveSymbol()
{
  if(!mSymbols) return;

  mFilu->addSymbol(mSymbol->text(), mMarket->currentText(), mProvider->currentText()
                 , mFi->id(), mSymbolId);

  mSymbols->rewindToId(mSymbolId);

  QString msg = tr("Symbol updated: Id=%1\n"
                   "\tFrom: Caption=%2 Market=%3 Owner=%4\n"
                   "\tTo:   Caption=%5 Market=%6 Owner=%7")
                  .arg(mSymbolId)
                  .arg(mSymbols->caption())
                  .arg(mSymbols->market())
                  .arg(mSymbols->owner())
                  .arg(mSymbol->text())
                  .arg(mMarket->currentText())
                  .arg(mProvider->currentText());

  emitMessage(FUNC, msg);
  record(FUNC, msg);

  setSymbolTable();
}

void FiPage::deleteSymbol()
{
  if(!mSymbols) return;
  if(!mSymbolId) return;
  if(mSymbols->size() < 2)
  {
    emitMessage(FUNC, tr("I will not delete the one and only symbol"), eWarning);
    return;
  }

  mSymbols->rewindToId(mSymbolId);

  int ret = QMessageBox::warning(this, tr("FI Page")
            , tr("\nDelete Symbol '%1 %2 %3'?\t")
                .arg(mSymbols->caption())
                .arg(mSymbols->market())
                .arg(mSymbols->owner())
            , QMessageBox::Yes | QMessageBox::Cancel
            , QMessageBox::Cancel);

  if(ret != QMessageBox::Yes) return;

  mFilu->deleteRecord("symbol", mSymbols->id());
  // FIXME Add check if success/error after delete

  QString msg = tr("Symbol deleted: Id=%1 Caption=%2 Market=%3 Owner=%4")
                  .arg(mSymbols->id())
                  .arg(mSymbols->caption())
                  .arg(mSymbols->market())
                  .arg(mSymbols->owner());

  emitMessage(FUNC, msg);
  record(FUNC, msg);

  setSymbolTable();
}

void FiPage::saveFi()
{
  if(!mFi) return;

  // FIXME Right now we have no fitting addFi() function
  mFilu->addFi(mFiName->text(), mFiType->currentText(), mFi->id());
  mFilu->updateField("expirydate", mExpiryDate->text(), "fi", mFi->id());

  mLookUp->search();

  emitMessage(FUNC, tr("FI updated: Id=%1 Name=%2").arg(mFi->id()).arg(mFiName->text()));
}

void FiPage::deleteFi()
{
   if(!mFi) return;

   int ret = QMessageBox::warning(this, tr("FI Page")
              , tr("\nDelete FI '%1 (%2)'?\t").arg(mFi->name()).arg(mFi->type())
              , QMessageBox::Yes | QMessageBox::Cancel
              , QMessageBox::Cancel);

  if(ret != QMessageBox::Yes) return;

  mFilu->deleteRecord("fi", mFi->id());
  // FIXME Add check if success/error after delete

  QString msg = tr("FI deleted: Id=%1 Name=%2 Type=%3")
                      .arg(mFi->id())
                      .arg(mFi->name())
                      .arg(mFi->type());

  emitMessage(FUNC, msg);
  record(FUNC, msg);

  mFiName->setText("");
  mFiType->setCurrentIndex(-1);
  mExpiryDate->setText("");

  delete mFi;
  mFi = 0;
  setSymbolTable();
  mLookUp->setFocus();
  mLookUp->search();
}
