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

#include <QtGui>
#include <QDebug>

#include "InspectorF.h"
#include "RcFile.h"


InspectorF::InspectorF()
          : QMainWindow()
          , FClass("InspectorF")
{
  FWidget* cw = new FWidget(this);
  setCentralWidget(cw);

  // create the tab pages...

  //
  // ...the rules tab
  int r = 0, c = 0;
  QGridLayout* rulesLayout = new QGridLayout;
  rulesLayout->addWidget(new QLabel(tr("Trading Rule")), r, c);
  rulesLayout->addWidget(&mTradingRuleName, r, c + 1, 1, 2);
  rulesLayout->addItem(new QSpacerItem(0, 0), r, c + 3);

/*  QSplitter* editorSplitter = new QSplitter(Qt::Vertical);
  editorSplitter->addWidget(&mRuleEditor);
  editorSplitter->addWidget(&mIndicatorEditor);

  rulesLayout->addWidget(editorSplitter, ++r, c, 1, 4);
*/
  rulesLayout->addWidget(&mEditor, ++r, c, 1, 4);
  rulesLayout->setColumnStretch(2, 1);
  rulesLayout->setColumnStretch(3, 1);

  QWidget* rulesTab = new QWidget(cw);
  rulesTab->setLayout(rulesLayout);
  rulesTab->setAccessibleName("Rules");

/*  connect(&mRuleEditor, SIGNAL(textChanged()), this, SLOT(edited()));
  connect(&mIndicatorEditor, SIGNAL(textChanged()), this, SLOT(edited()));*/
  connect(&mEditor, SIGNAL(edited()), this, SLOT(edited()));

  //
  // ...the testing tab
  r = 0, c = 0;
  QGridLayout* testingLayout = new QGridLayout;
  testingLayout->addWidget(new QLabel(tr("From Date")), r, c);
  testingLayout->addWidget(&mFromDate, r, c + 1);
  testingLayout->addWidget(new QLabel(tr("To Date")), r, c + 2);
  testingLayout->addWidget(&mToDate, r, c + 3);
  testingLayout->addItem(new QSpacerItem(0, 0), r, c + 4);

  mFromDate.setDisplayFormat("yyyy-MM-dd");
  mToDate.setDisplayFormat("yyyy-MM-dd");

//   QSplitter* displaySplitter = new QSplitter(Qt::Vertical);
//   displaySplitter->addWidget(&mRuleDisplay);
//   displaySplitter->addWidget(&mIndicatorDisplay);


  testingLayout->addWidget(&mDisplay, ++r, c, 1, 5);
  testingLayout->setColumnStretch(4, 1);

  QWidget* testingTab = new QWidget(cw);
  testingTab->setLayout(testingLayout);
  testingTab->setAccessibleName("Testing");

  //
  // ...detail tab
  QSplitter* detailTab = new QSplitter(Qt::Vertical);
  detailTab->addWidget(&mDetailView);
  detailTab->addWidget(&mDetailDisplay);

  mDetailDisplay.setReadOnly(true);
  mDetailDisplay.setOrientation(Qt::Horizontal);

  // add all pages to the tab widget
  mTabWidget = new QTabWidget;
  mTabWidget->addTab(rulesTab, tr("Strategy"));
  mTabWidget->addTab(testingTab, tr("Testing"));
  mTabWidget->addTab(&mReport, tr("Report"));
  mTabWidget->addTab(&mResultsView, tr("Last Results"));
  mTabWidget->addTab(&mPerformanceView, tr("Best Performance"));
  mTabWidget->addTab(&mScoreView, tr("Best Score"));
  mTabWidget->addTab(detailTab, tr("Details"));

  mResultsView.setModel(&mResults);
  mPerformanceView.setModel(&mPerformance);
  mScoreView.setModel(&mScore);
  mDetailView.setModel(&mDetail);

  mReport.setAccessibleName("Report");
  mResultsView.setAccessibleName("Last");
  mPerformanceView.setAccessibleName("Performance");
  mScoreView.setAccessibleName("Score");
  detailTab->setAccessibleName("Details");

  connect(&mResultsView, SIGNAL(newSelection(const QModelIndex &)), this, SLOT(resultSelected(const QModelIndex &)));
  connect(&mPerformanceView, SIGNAL(newSelection(const QModelIndex &)), this, SLOT(resultSelected(const QModelIndex &)));
  connect(&mScoreView, SIGNAL(newSelection(const QModelIndex &)), this, SLOT(resultSelected(const QModelIndex &)));

  //
  // create the buttons and progess bar at the bottom

//   QPushButton* closeButton = new QPushButton(tr("Close"));
//   connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

  mProgessBar = new QProgressBar;
  mProgessBar->setFormat(tr("%v of %m"));

  mTestButton = new QPushButton(tr("Run Serial Test"));
  connect(mTestButton, SIGNAL(clicked()), this, SLOT(runTest()));

  //QPushButton* simButton = new QPushButton(tr("Run Simulation"));
  //connect(simButton, SIGNAL(clicked()), this, SLOT(runTest()));

  connect(mTabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

  QHBoxLayout* buttonsLayout = new QHBoxLayout;
  //buttonsLayout->addStretch(1);
  buttonsLayout->addWidget(mProgessBar);
  buttonsLayout->addWidget(mTestButton);
  //buttonsLayout->addWidget(closeButton);

  //
  // add all stuff to the main widget
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(mTabWidget);
  mainLayout->addLayout(buttonsLayout);

  cw->setLayout(mainLayout);

  setWindowTitle(tr("InspectorF - The Filu Back Tester"));

  init();

 }

InspectorF::~InspectorF()
{
  // is it a bug in QTabWidget? signal is emmitted when go die.
  // The result is in our case a segfault
  disconnect(mTabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

  saveSettings();

  if(mBackTester) delete mBackTester;
  delete mTrader;
}

void InspectorF::init()
{
  mTrader = new Trader(this);

  mConstMatcher.setPattern("(\\b\\d+\\.?\\d*)");
  mEditedMatcher.setPattern("\\{([\\d\\.\\-\\i\\;\\s]+)\\}");
  //mErrorMatcher.setPattern();

  readSettings();

  QDir dir(mTradingRulePath);
  QStringList allFiles = dir.entryList(QDir::Files, QDir::Name);
  qDebug() << allFiles;
  mTradingRuleName.insertItem(1, allFiles.at(0));
  mTradingRuleName.setEditable(true);
  connect(&mTradingRuleName, SIGNAL(activated(const QString &)), this, SLOT(loadRule(const QString &)));
  mTradingRuleName.setCurrentIndex(0);

//   dir.setPath(mIndicatorPath);
//   allFiles = dir.entryList(QDir::Files, QDir::Name);
//   mIndicatorName.insertItems(0, allFiles);

//   QSqlQuery* query = mFilu->getGroups(-1); // get all groups
//   if(query)
//   {
//     while(query->next())
//     {
//       mFiGroup.addItem(query->value(1).toString(), query->value(0));
//     }
//   }

//   mRuleEditor.setLineWrapMode(QTextEdit::NoWrap);
//   mRuleEditor.installEventFilter(this);
//
//   mIndicatorEditor.setLineWrapMode(QTextEdit::NoWrap);
//   mIndicatorEditor.installEventFilter(this);
//
//   mRuleDisplay.setLineWrapMode(QTextEdit::NoWrap);
//   mRuleDisplay.setReadOnly(true);
//   mRuleDisplay.installEventFilter(this);
//
//   mIndicatorDisplay.setLineWrapMode(QTextEdit::NoWrap);
//   mIndicatorDisplay.setReadOnly(true);
//   mIndicatorDisplay.installEventFilter(this);

  mDisplay.setReadOnly(true);

  mBackTester = new BackTester;
  connect(mBackTester, SIGNAL(loopsNeedet(int)), this, SLOT(loopsNeedet(int)));
  connect(mBackTester, SIGNAL(loopDone(int)), this, SLOT(loopDone(int)));
  connect(mBackTester, SIGNAL(strategyDone()), this, SLOT(newData()));
  connect(mBackTester, SIGNAL(error()), this, SLOT(backTestError()));

  edited();
}

void InspectorF::readSettings()
{
  mTradingRulePath = mRcFile->getST("TradingRulePath");
  mIndicatorPath = mRcFile->getST("IndicatorPath");
  mRcFile->beginGroup("Inspector");
  QSize size;
  size = mRcFile->getSZ("InspectorSize");
  resize(size);
  QPoint pos;
  pos = mRcFile->getPT("InspectorPosition");
  move(pos);
  QByteArray ba;
  ba = mRcFile->getBA("InspectorState");
  restoreState(ba);

  // restore rule page
  mTradingRuleName.setEditText(mRcFile->value("Rule").toString());
  mEditor.mRule.insertPlainText(mRcFile->value("Rule").toString());
  mEditor.mRule.moveCursor(QTextCursor::Start);

  mEditor.mIndi.insertPlainText(mRcFile->value("Indicator").toString());
  mEditor.mIndi.moveCursor(QTextCursor::Start);

  // restore testing page
  mFromDate.setDate(mRcFile->value("FromDate").toDate());
  mToDate.setDate(mRcFile->value("ToDate").toDate());

  mRcFile->endGroup();
}

void InspectorF::saveSettings()
{
  mRcFile->beginGroup("Inspector");
  mRcFile->set("InspectorSize", size());
  mRcFile->set("InspectorPosition", pos());
  mRcFile->set("InspectorState", saveState());

  mRcFile->setValue("TradingRule", mTradingRuleName.currentText());
  mRcFile->setValue("Rule", mEditor.mRule.toPlainText());
  mRcFile->setValue("Indicator", mEditor.mIndi.toPlainText());

  mRcFile->setValue("FromDate", mFromDate.date());
  mRcFile->setValue("ToDate", mToDate.date());

  mRcFile->endGroup();
}

void InspectorF::loadRule(const QString& fileName)
{
  if(fileName.isEmpty()) return;

  QFile file(mTradingRulePath + fileName);

  if (!mTrader->useRuleFile(fileName))
  {
    QStringList error = mTrader->errorText();
    QMessageBox::critical(this, tr("Indicator Editor"),
                         error.join("\n"),
                         QMessageBox::Close);
    return;
  }

  //QStringList vl;
  //mTrader->getVariablesList(vl);
  //qDebug() << vl;
  mTrader->getIndicator(mOrigIndicator);
  //qDebug() << mOrigIndicator;

  QStringList mOrigRule;
  mTrader->getRule(mOrigRule);
  //qDebug() << mOrigRule;

  // mark all constants
  mOrigRule.replaceInStrings(mConstMatcher, "{\\1}");
  mEditor.mRule.clear();
  mEditor.mRule.insertPlainText(mOrigRule.join("\n"));
  mEditor.mRule.moveCursor(QTextCursor::Start);

  mOrigIndicator.replaceInStrings(mConstMatcher, "{\\1}");
  mEditor.mIndi.clear();
  mEditor.mIndi.insertPlainText(mOrigIndicator.join("\n"));
  mEditor.mIndi.moveCursor(QTextCursor::Start);

  mEdited = true;
}

void InspectorF::tabChanged(int/* = 0*/)
{
  QString page = mTabWidget->currentWidget()->accessibleName();

  if(page == "Testing")
  {
    if(!mEdited) return;
    mEdited = false;
    mTestButton->setEnabled(true);

    // we start with the rule file
    QString txt = mEditor.mRule.toPlainText();

    // restore unchanged constants
    txt.replace(QRegExp("\\{([\\d\\.]+)\\}"),"\\1");

    //mRuleDisplay.setPlainText(txt);
    mDisplay.mRule.setPlainText(txt);

    // all again for the indicator file
    txt = mEditor.mIndi.toPlainText();
    txt.replace(QRegExp("\\{([\\d\\.]+)\\}"),"\\1");
    //mIndicatorDisplay.setPlainText(txt);
    mDisplay.mIndi.setPlainText(txt);

    mBackTester->prepare( mDisplay.mRule.toPlainText(),
                          mDisplay.mIndi.toPlainText(),
                          mFromDate.date(), mToDate.date() );

    return;
  }//if(page == "Testing")

  if(page == "Last")
  {
    if(!mNewData.contains("Last")) mNewData.insert("Last", true);
    if(mNewData.value("Last") == false) return;
    mNewData.insert("Last", false);

    QSqlQuery* query = mFilu->execSql("GetLastResults");

    if(!query) return;

    mResults.setQuery(*query);

    mResultsView.resizeColumnsToContents();
    //mResultsView.hideColumn(0);
    //mResultsView.hideColumn(1);
    //resize(mView->columnWidth(2) + mView->columnWidth(3) + mView->columnWidth(4) + mView->columnWidth(5) + 10, height());
    //show();

    return;
  }

  if(page == "Performance")
  {
    if(!mNewData.contains("Performance")) mNewData.insert("Performance", true);
    if(mNewData.value("Performance") == false) return;
    mNewData.insert("Performance", false);

    QSqlQuery* query = mFilu->execSql("GetBestPerformance");

    if(!query) return;
    mPerformance.setQuery(*query);

    mPerformanceView.resizeColumnsToContents();
    //mPerformanceView.hideColumn(0);
    //mPerformanceView.hideColumn(1);
    //resize(mView->columnWidth(2) + mView->columnWidth(3) + mView->columnWidth(4) + mView->columnWidth(5) + 10, height());
    //show();

    return;
  }

  if(page == "Score")
  {
    if(!mNewData.contains("Score")) mNewData.insert("Score", true);
    if(mNewData.value("Score") == false) return;
    mNewData.insert("Score", false);

    QSqlQuery* query = mFilu->execSql("GetBestScore");

    if(!query) return;
    mScore.setQuery(*query);

    mScoreView.resizeColumnsToContents();
    //mScoreView.hideColumn(0);
    //mScoreView.hideColumn(1);
    //resize(mView->columnWidth(2) + mView->columnWidth(3) + mView->columnWidth(4) + mView->columnWidth(5) + 10, height());
    //show();

    return;
  }

  if(page == "Details")
  {
    if(mShowingStrategyId == mSelectedStrategyId) return;

    mShowingStrategyId = mSelectedStrategyId;

    QSqlQuery* query = mFilu->getBTDetails(mShowingStrategyId);

    if(!query) return;
    mDetail.setQuery(*query);

    mDetailView.resizeColumnsToContents();
    //mDetailView.hideColumn(0);
    mDetailView.hideColumn(1);
    mDetailView.hideColumn(12); // rule
    mDetailView.hideColumn(13); // indicator
    //resize(mView->columnWidth(2) + mView->columnWidth(3) + mView->columnWidth(4) + mView->columnWidth(5) + 10, height());
    //show();

    QString rule = mDetailView.indexAt(QPoint(0,0)).sibling(0, 12).data().toString();
    QString indi = mDetailView.indexAt(QPoint(0,0)).sibling(0, 13).data().toString();

    mDetailDisplay.mRule.setPlainText(rule);
    mDetailDisplay.mIndi.setPlainText(indi);
    return;
  }
}

void InspectorF::runTest()
{
  if(mEdited) return;

  mBackTester->setDates(mFromDate.date(), mToDate.date());
  mBackTester->calc();
}

void InspectorF::edited()
{
  mEdited = true;
  mTestButton->setEnabled(false); // FIXME: why does not paint button grayed?
  //mTestButton->update(); does not help
  //mTestButton->repaint(); does not help
}

void InspectorF::loopsNeedet(int ln)
{
  mProgessBar->reset();
  mProgessBar->setRange(0, ln);
  mProgessBar->setValue(0);
  qDebug() << "InspectorF::loopsNeedet()" << ln << mProgessBar->format();
}

void InspectorF::loopDone(int ld)
{
  mProgessBar->setValue(ld);
  //qDebug() << "InspectorF::loopDone()" << ld;
}

void InspectorF::newData()
{
  QHash<const QString, bool>::iterator it = mNewData.begin();
  while(it != mNewData.end())
  {
    it.value() = true;
    ++it;
  }

  tabChanged(); // update tab
}

void InspectorF::resultSelected(const QModelIndex& index)
{
  int row = index.row();
  mSelectedStrategyId = index.sibling(row, 0).data().toString();
  //qDebug() << "InspectorF::resultSelected()" << mSelectedStrategyId;
}

void InspectorF::backTestError()
{
  clearErrors();
  addErrorText(mBackTester->errorText());
  mReport.insertPlainText(errorText().join("\n") + "\n");
  mReport.moveCursor(QTextCursor::Start);

  mTabWidget->setCurrentWidget(&mReport);
}
