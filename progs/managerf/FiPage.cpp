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
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
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
#include "SqlTableView.h"
#include "SymbolTableView.h"
#include "SymbolTuple.h"
#include "SymbolTypeTuple.h"
#include "Validator.h"

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

  QToolButton* saveBtn = new QToolButton;
  saveBtn->setToolTip(tr("Save Symbol Changes"));
  saveBtn->setAutoRaise(true);
  saveBtn->setIcon(QIcon::fromTheme("document-save"));
//   saveBtn->setShortcut(QKeySequence(QKeySequence::Save));
  connect(saveBtn, SIGNAL(clicked()), this, SLOT(saveSymbol()));

  QToolButton* newBtn = new QToolButton;
  newBtn->setToolTip(tr("Add New Symbol"));
  newBtn->setAutoRaise(true);
  newBtn->setIcon(QIcon::fromTheme("list-add"));
//   newBtn->setShortcut(QKeySequence(QKeySequence::Save));
  connect(newBtn, SIGNAL(clicked()), this, SLOT(newSymbol()));

  QToolButton* delBtn = new QToolButton;
  delBtn->setToolTip(tr("Delete Symbol"));
  delBtn->setAutoRaise(true);
  delBtn->setIcon(QIcon::fromTheme("edit-delete"));
//   delBtn->setShortcut(QKeySequence("Ctrl+Del"));
  connect(delBtn, SIGNAL(clicked()), this, SLOT(deleteSymbol()));

  QHBoxLayout* hbox = new QHBoxLayout; // Symbol Edit Line
  hbox->setMargin(0);
  hbox->addWidget(saveBtn);
  hbox->addWidget(newBtn);
  hbox->addWidget(mSymbol);
  hbox->addWidget(mMarket);
  hbox->addWidget(mProvider);
  hbox->addWidget(delBtn);
  hbox->addStretch(1);

  mSymbolView = new SymbolTableView(mSymbols);
  connect(mSymbolView, SIGNAL(newSelection(const QModelIndex &)),
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
  QFormLayout* flay = new QFormLayout;  // Build edit area
  flay->addRow(tr("Date"),      mSplitDate = new QLineEdit);
  flay->addRow(tr("Ratio"),     mRatio = new QLineEdit);
  flay->addRow(tr("Comment"),   mSplitComment = new QLineEdit);

  // Add an "empty" line
  int charWidth = QFontMetrics(font()).averageCharWidth();
  QWidget* widget = new QWidget;
  widget->setMinimumHeight(charWidth * 2);
  flay->addRow(widget);

  flay->addRow(tr("Pre:Post"),  mPrePost = new QLineEdit);

  mSplitDate->setMaximumWidth(charWidth * 15);
  mSplitDate->setInputMask("D999-9D-9D");
  connect(mPrePost, SIGNAL(editingFinished()), this, SLOT(prePostEdited()));
  mPrePost->setMaximumWidth(charWidth * 10);
  mPrePost->setInputMask("D0:D0");
  mSplitComment->setMinimumWidth(charWidth * 30);

  QHBoxLayout* hbox = new QHBoxLayout;  // Build button line
  hbox->setMargin(0);

  QToolButton* btn = new QToolButton;
  hbox->addWidget(btn);
  btn->setToolTip(tr("Save Split Changes"));
  btn->setAutoRaise(true);
  btn->setIcon(QIcon::fromTheme("document-save"));
//   btn->setShortcut(QKeySequence(QKeySequence::Save));
  connect(btn, SIGNAL(clicked()), this, SLOT(saveSplit()));

  btn = new QToolButton;
  hbox->addWidget(btn);
  btn->setToolTip(tr("Add Split (Search for Split)"));
  btn->setAutoRaise(true);
  btn->setIcon(QIcon::fromTheme("list-add"));
//   btn->setShortcut(QKeySequence(QKeySequence::Save));
  connect(btn, SIGNAL(clicked()), this, SLOT(newSplit()));

  hbox->addStretch();

  btn = new QToolButton;
  hbox->addWidget(btn);
  btn->setToolTip(tr("Delete Split"));
  btn->setAutoRaise(true);
  btn->setIcon(QIcon::fromTheme("edit-delete"));
//   btn->setShortcut(QKeySequence(QKeySequence::Save));
  connect(btn, SIGNAL(clicked()), this, SLOT(deleteSplit()));

  flay->addRow("", hbox);               // Complete edit area..

  hbox = new QHBoxLayout;               // ..but add a stretch to the right
  hbox->setMargin(0);
  hbox->addLayout(flay);
  hbox->addStretch(1);
//   hbox->setStretch(0, 2);

  widget = new QWidget;
  widget->setLayout(hbox);

  mSplitView = new SqlTableView;
  mSplitView->verticalHeader()->hide();
  connect(mSplitView, SIGNAL(newSelection(const QModelIndex &))
          , this, SLOT(splitClicked(const QModelIndex &)));

  QSplitter* splitter = new QSplitter(Qt::Horizontal);
  splitter->addWidget(mSplitView);
  splitter->addWidget(widget);

  return splitter;
}

QWidget* FiPage::makeBarTab()
{
  QLabel* l = new QLabel("Sorry, not yet implemented");
  l->setAlignment(Qt::AlignCenter);
  return l;
}

QWidget* FiPage::makeCoTab()
{
  return makeBarTab();
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
  mMainBox->setTitle(QString("%1 (%2 FiId:%3)").arg(mFi->name(), mFi->type())
                                               .arg(mFi->id()));

  mFiType->clear();
  mFiType->addItems(mFilu->getFiTypeNames());

  mFiName->setText(mFi->name());
  mFiType->setCurrentIndex(mFiType->findText(mFi->type()));
  mExpiryDate->setText(mFi->expiryDate().toString(Qt::ISODate));

  setSymbolTable();
  setSplitTable();
}

void FiPage::setSymbolTable(int symbolId/* = 0*/)
{
  if(mSymbols) delete mSymbols;
  mSymbols = 0;
  mSymbolId = 0;

  if(mFi) mSymbols = mFilu->getSymbols(mFi->id());

  mSymbolView->setContent(mSymbols);

  if(!mSymbols) return; // Should never happens, but anyway

  if(symbolId)
  {
    mSymbolView->selectSymbol(symbolId);
    return;
  }

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

  mSymbols->rewindToId(mSymbolId);

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

void FiPage::newSymbol()
{
  mSymbolId = 0;
  mSymbol->setText("");
  mSymbol->setFocus();
  mSymbolView->clearSelection();
}

void FiPage::saveSymbol()
{
  if(!mSymbols) return;

  if(mSymbolId)
  {
    if(mSymbols->caption() == mSymbol->text()          and
       mSymbols->market()  == mMarket->currentText()   and
       mSymbols->owner()   == mProvider->currentText()     )
    {
      emitMessage(FUNC, tr("No changes to save"));
      return;
    }
  }

  int ret = mFilu->addSymbol(mSymbol->text(), mMarket->currentText(), mProvider->currentText()
                             , mFi->id(), mSymbolId);

  if(sadFilu(FUNC, tr("Fail to save symbol"), eWarning)) return;

  mFilu->updateField("quality", Filu::ePlatinum, "symbol", ret);

  QString msg;
  if(mSymbolId)
  {
    msg = tr("Symbol updated: Id=%1\n"
                "\tFrom: Caption=%2 Market=%3 Owner=%4\n"
                "\tTo:   Caption=%5 Market=%6 Owner=%7")
                .arg(mSymbolId)
                .arg(mSymbols->caption())
                .arg(mSymbols->market())
                .arg(mSymbols->owner())
                .arg(mSymbol->text())
                .arg(mMarket->currentText())
                .arg(mProvider->currentText());
  }
  else
  {
    mSymbolId = ret;
    msg = tr("Symbol added: Id=%1 Caption=%2 Market=%3 Owner=%4 FI=%5")
            .arg(mSymbolId)
            .arg(mSymbol->text())
            .arg(mMarket->currentText())
            .arg(mProvider->currentText())
            .arg(mFi->name());
  }

  emitMessage(FUNC, msg);
  record(FUNC, msg);

  setSymbolTable(mSymbolId);
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
  mFilu->updateField("quality", Filu::ePlatinum, "fi", mFi->id());

  QString msg = tr("FI updated: Id=%1\n"
                   "\tFrom: Name=%2 Type=%3 ExpiryDate=%4\n"
                   "\tTo:   Name=%5 Type=%6 ExpiryDate=%7")
                .arg(mFi->id())
                .arg(mFi->name(), mFi->type(), mFi->expiryDate().toString(Qt::ISODate))
                .arg(mFiName->text(), mFiType->currentText(), mExpiryDate->text());

  emitMessage(FUNC, msg);
  record(FUNC, msg);

  fiClicked(mFi->id());
  mLookUp->search();
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

void FiPage::setSplitTable()
{
  mSplitId = -1;
  mSplitDate->clear();
  mRatio->clear();
  mPrePost->clear();
  mSplitComment->clear();
  mFilu->setSqlParm(":fiId", mFi->id());
  mFilu->setSqlParm(":fromDate", "1000-01-01");
  mFilu->setSqlParm(":toDate", "3000-01-01");
  mSplitView->setQuery(mFilu->execSql("GetSplits"));
  mSplitView->hideColumn(3);
  mSplitView->hideColumn(4);
  mSplitView->hideColumn(5);
  mSplitView->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
  mSplitView->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
  mSplitView->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
  mSplitView->resizeRowsToContents();
  mSplitView->selectRow(0);
}

void FiPage::splitClicked(const QModelIndex & index)
{
  mSplitId = index.sibling(index.row(), 3).data().toInt();
  mSplitDate->setText(index.sibling(index.row(), 0).data().toString());
  mRatio->setText(QString::number(index.sibling(index.row(), 1).data().toDouble()));
  mSplitComment->setText(index.sibling(index.row(), 2).data().toString());

  QRegExp rx("\\b(\\d+:\\d+)\\b");
  rx.indexIn(index.sibling(index.row(), 2).data().toString());
  QString pp = rx.cap(1);
  mPrePost->setText(pp);
}

void FiPage::prePostEdited()
{
  Validator v(this);

  mRatio->setText(QString::number(v.dSplitPrePost(mPrePost->text())));

  // No tr() here. I think its pointless to translate
  QString cmt = QString("Split %1").arg(mPrePost->text());

  if(v.dSplitPrePost(mPrePost->text()) > 1) cmt.prepend("Reverse ");

  mSplitComment->setText(cmt);
}

void FiPage::newSplit()
{
  if(mSplitId != 0)
  {
    mSplitId = 0;
    if(mBars) mBars->rewind(0);         // Set on first data set
  }

  mRatio->setText("");
  mSplitComment->setText(tr("No Split found"));
  mPrePost->setText("1:1");
  mPrePost->setFocus();
  mSplitView->clearSelection();

  if(!mBars) return;

  // Examine data to find a possible split
  double yh = mBars->high();  // Yesterday High
  double yl = mBars->low();   //           Low
  double ratio = 1.3;         // Which gap will we take as split
  bool   found = false;
  while(mBars->next())
  {
    bool splitUp   = mBars->high() > yh * ratio ? true : false;
    bool splitDown = mBars->low()  < yl / ratio ? true : false;
    if(splitUp or splitDown)
    {
      int split = static_cast<int>(0.5 + yh / mBars->high());
      int merge = static_cast<int>(0.5 + mBars->high() / yh);
      //qDebug() << "ratio" << mBars->date().toString(Qt::ISODate) << yh / mBars->high() << mBars->high() / yh <<  split << merge;
      if(split > 1)
      {
        mPrePost->setText(QString(" 1:%1 ").arg(QString::number(split)));
      }
      else if(merge > 1)
      {
        mPrePost->setText(QString("%1:1").arg(QString::number(merge)));
      }
      else
      {
        continue;
      }

      mSplitDate->setText(mBars->date().toString(Qt::ISODate));
      prePostEdited(); // Will fill the other fields
      found = true;
      break;
    }

    yh = mBars->high();
    yl = mBars->low();
  }

  if(!found) mSplitDate->setText(QDate::currentDate().toString(Qt::ISODate));
}

void FiPage::saveSplit()
{
  QString msg = "Split %5: Id=%1 Date=%2 Comment=%3 FI=%4";

  if(mSplitId > 0)
  {
    // FIXME Right now we have no fitting addSplit() function
    mFilu->updateField("sdate", mSplitDate->text(), "split", mSplitId);
    mFilu->updateField("sratio", mRatio->text(), "split", mSplitId);
    mFilu->updateField("scomment", mSplitComment->text(), "split", mSplitId);
    mFilu->updateField("quality", Filu::ePlatinum, "split", mSplitId);

    msg = msg.arg(mSplitId)
             .arg(mSplitDate->text())
             .arg(mSplitComment->text())
             .arg(mFi->name())
             .arg(tr("updated"));
  }
  else
  {
    mSplitId = mFilu->addSplit(mSymbol->text()
                             , mSplitDate->text()
                             , mRatio->text().toDouble()
                             , mSplitComment->text()
                             , Filu::ePlatinum);

    msg = msg.arg(mSplitId)
             .arg(mSplitDate->text())
             .arg(mSplitComment->text())
             .arg(mFi->name())
             .arg(tr("added"));
  }

  emitMessage(FUNC, msg);
  record(FUNC, msg);

  int id = mSplitId; // setSplitTable() will change mSplitId
  setSplitTable();
  mSplitView->selectRowWithValue(id, 3);
  loadBars();
}

void FiPage::deleteSplit()
{
  if(mSplitId < 1) return;

  int ret = QMessageBox::warning(this, tr("FI Page")
            , tr("\nDelete Split '%1 (%2)'?\t").arg(mSplitDate->text(), mSplitComment->text())
            , QMessageBox::Yes | QMessageBox::Cancel
            , QMessageBox::Cancel);

  if(ret != QMessageBox::Yes) return;

  mFilu->deleteRecord("split", mSplitId);
  // FIXME Add check if success/error after delete

  QString msg = QString("Split %5: Id=%1 Date=%2 Comment=%3 FI=%4")
                       .arg(mSplitId)
                       .arg(mSplitDate->text())
                       .arg(mSplitComment->text())
                       .arg(mFi->name())
                       .arg(tr("deleted"));

  emitMessage(FUNC, msg);
  record(FUNC, msg);

  setSplitTable();
  loadBars();
}
