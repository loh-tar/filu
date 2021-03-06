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

#include <QButtonGroup>
#include <QComboBox>
#include <QDir>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include "AddFiPage.h"

#include "FiTuple.h"
#include "FiluU.h"
#include "Importer.h"
#include "LineEdit.h"
#include "MarketTuple.h"
#include "RcFile.h"
#include "Script.h"
#include "SearchField.h"
#include "SymbolTuple.h"
#include "SymbolTypeTuple.h"

static const QString cCancel = QObject::tr("Cancel");
static const QString cSearch = QObject::tr("Search");

AddFiPage::AddFiPage(FClass* parent)
         : ManagerPage(parent, FUNC)
         , mHitCounter(this)
{
  createPage();
}

AddFiPage::~AddFiPage()
{
  delete mScripter;
  delete mImporter;
}

void AddFiPage::createPage()
{
  mIcon = QIcon(":/icons/add-fi.png");
  mIconText = tr("Add FI");

  mScripter = new Script(this);
  connect(mScripter, SIGNAL(newData(QStringList *))
          , this, SLOT(fillResultTable(QStringList *)));
  connect(mScripter, SIGNAL(finished())
          , this, SLOT(scriptFinished()));

  mImporter = new Importer(this);
  if(mImporter->hasError()) addErrors(mImporter->errors());

  //
  // Create The Main Layout Widgets
  QSplitter* splitter  = new QSplitter;
  QWidget* upperWidget = new QWidget;
  QWidget* lowerWidget = new QWidget;
  splitter->setOrientation(Qt::Vertical);
  splitter->addWidget(upperWidget);
  splitter->addWidget(lowerWidget);

  //
  // Create the search line widgets and there connections
  mProviderSelector = new QComboBox;
  mProviderSelector->setToolTip(tr("Choose a provider"));
  mProviderSelector->insertItems(0, mScripter->providerList());
  mProviderSelector->setCurrentIndex(mProviderSelector->findText("Filu"));
  connect(mProviderSelector, SIGNAL(currentIndexChanged(const QString&))
        , this, SLOT(providerChanged(const QString&)));

  mProviderFuncSelector = new QComboBox;
  mProviderFuncSelector->setToolTip(tr("Choose a provider function"));
  connect(mProviderFuncSelector, SIGNAL(currentIndexChanged(const QString&))
        , this, SLOT(providerFuncChanged(const QString&)));

  QPushButton* funcInfoBtn = new QPushButton;
  funcInfoBtn->setText("?");
  funcInfoBtn->setToolTip(tr("Show provider function infos"));
  connect(funcInfoBtn, SIGNAL(clicked()), this, SLOT(showProviderFuncInfo()));

  mSearchField = new SearchField(upperWidget);
  connect(mSearchField, SIGNAL(returnPressed()), this, SLOT(search()));

  mSearchCancelBtn = new QPushButton;
  mSearchCancelBtn->setText(cSearch);
  connect(mSearchCancelBtn, SIGNAL(clicked()), this, SLOT(searchOrCancel()));

  QPushButton* insertRowBtn = new QPushButton;
  insertRowBtn->setText("+");
  insertRowBtn->setToolTip(tr("Insert new row"));
  connect(insertRowBtn, SIGNAL(clicked()), this, SLOT(insertRow()));

  QPushButton* removeRowBtn = new QPushButton;
  removeRowBtn->setText("-");
  removeRowBtn->setToolTip(tr("Remove selected row"));
  connect(removeRowBtn, SIGNAL(clicked()), this, SLOT(removeRow()));

  QPushButton* addAllBtn = new QPushButton;
  addAllBtn->setText(tr("Add all to DB"));
  addAllBtn->setToolTip(tr("Add all listed FIs to the Database"));
  connect(addAllBtn, SIGNAL(clicked()), this, SLOT(addAllToDB()));

  //
  // The Result List
  mResultList = new QTableWidget;
  mResultList->verticalHeader()->hide();
  mResultList->setShowGrid(false);
  mResultList->setSelectionBehavior(QAbstractItemView::SelectRows);
  connect(mResultList, SIGNAL(cellClicked(int, int)), this, SLOT(selectResultRow(int, int)));

  //
  // The Edit Area
  QPushButton* addBtn = new QPushButton;
  addBtn->setText(tr("Add to DB"));
  addBtn->setToolTip(tr("Add this one FI with all Symbols to the Database"));
  connect(addBtn, SIGNAL(clicked()), this, SLOT(addToDB()));

  connect(&mPSMGrp, SIGNAL(searchCompBtnClicked(int)), this, SLOT(searchCompBtnClicked(int)));

  QFontMetrics f(font());
  int w = f.width("X");
  mRefSymbol = new LineEdit;
  mRefSymbol->setMinimumWidth(w * 15);
  mName = new LineEdit;
  mName->setMinimumWidth(w * 40);

  mType = new QComboBox;
  mType->addItems(mFilu->getFiTypeNames());

  // Build The Edit Area Layout
  QGridLayout* editNameLO = new QGridLayout;
  editNameLO->addWidget( new QLabel("RefSymbol"), 0, 0);
  editNameLO->addWidget(mRefSymbol              , 1, 0);
  editNameLO->addWidget( new QLabel("Name")     , 0, 1);
  editNameLO->addWidget(mName                   , 1, 1);
  editNameLO->addWidget( new QLabel("Type")     , 0, 2);
  editNameLO->addWidget(mType                   , 1, 2);
  editNameLO->addWidget(addBtn                 , 1, 3);
  editNameLO->setColumnStretch(4, 2); // Stretch non exiting column (add spacerItem)

  QGridLayout* editSymbolLO = new QGridLayout;
  editSymbolLO->addWidget( new QLabel("Symbol"), 0, 0);
  editSymbolLO->addWidget( new QLabel("Market"), 0, 1);
  editSymbolLO->addWidget( new QLabel("Provider"), 0, 2);

  // Read all symbol types out of the DB
  SymbolTypeTuple* symbolTypes = mFilu->getSymbolTypes(Filu::eAllTypes);
  if(!symbolTypes)
  {
    check4FiluError(FUNC, tr("No SymbolTypes found"));
  }

  QStringList marketList = mFilu->getMarketNames();

  for(int i = 0; i < 3; ++i)
  {
    mPSMGrp.addOne();

    if(symbolTypes)
    {
      symbolTypes->rewind();
      while(symbolTypes->next())
      {
        mPSMGrp.provider(i)->insertItem(0, symbolTypes->caption(), symbolTypes->isProvider());
      }
    }

    mPSMGrp.market(i)->addItems(marketList);

    editSymbolLO->addWidget(mPSMGrp.symbol(i), i + 1, 0);
    editSymbolLO->addWidget(mPSMGrp.market(i), i + 1, 1);
    editSymbolLO->addWidget(mPSMGrp.provider(i), i + 1, 2);
    editSymbolLO->addWidget(mPSMGrp.searchBtn(i), i + 1, 3);
  }
  editSymbolLO->setColumnStretch(4, 2); // Stretch non exiting column (add spacerItem)

  if(symbolTypes) delete symbolTypes;

  QHBoxLayout* topLine = new QHBoxLayout;
  topLine->addWidget(mProviderSelector);
  topLine->addWidget(mProviderFuncSelector);
  topLine->addWidget(funcInfoBtn);
  topLine->addWidget(mSearchField);
  topLine->addWidget(mSearchCancelBtn);
  topLine->addWidget(&mHitCounter);
  topLine->addStretch(1);
  topLine->addWidget(removeRowBtn);
  topLine->addWidget(insertRowBtn);
  topLine->addWidget(addAllBtn);

  QVBoxLayout* searchLayout = new QVBoxLayout;
  searchLayout->addLayout(topLine);
  searchLayout->addWidget(mResultList);

  QVBoxLayout* editLayout = new QVBoxLayout;
  editLayout->addLayout(editNameLO);
  editLayout->addLayout(editSymbolLO);
  editLayout->addStretch(1);

  //
  // Build the main layout
  upperWidget->setLayout(searchLayout);
  lowerWidget->setLayout(editLayout);

  QVBoxLayout* groupBoxLayout = new QVBoxLayout;
  groupBoxLayout->addWidget(splitter);

  mMainBox = new QGroupBox(tr("Add a new FI to the Data Base"));
  mMainBox->setLayout(groupBoxLayout);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(mMainBox);
  setLayout(mainLayout);

  setFocusWidget(mSearchField);
}

void AddFiPage::providerChanged(const QString& provider)
{
  QString oldFunc = mProviderFuncSelector->currentText();
  mProviderFuncSelector->blockSignals(true);
  mProviderFuncSelector->clear();

  if(!mProviderFunctions.contains(provider))
  {
    mProviderFunctions.insert(provider, mScripter->functionList(provider));
  }

  mProviderFuncSelector->addItems(mProviderFunctions.value(provider));

  int idx = mProviderFuncSelector->findText(oldFunc);
  if(idx > -1)
  {
    mProviderFuncSelector->setCurrentIndex(idx);
    providerFuncChanged(oldFunc);
  }
  else
  {
    providerFuncChanged(mProviderFuncSelector->currentText());
  }

  mProviderFuncSelector->blockSignals(false);
}

void AddFiPage::providerFuncChanged(const QString& func)
{
  QString provider = mProviderSelector->currentText();
  if(!mProviderFuncInfo.contains(provider + func))
  {
    mProviderFuncInfo.insert(provider + func, mScripter->functionInfo(provider, func));
  }
}

void AddFiPage::showProviderFuncInfo()
{
  QString providerFunc = mProviderSelector->currentText();
  providerFunc.append(mProviderFuncSelector->currentText());
  const QHash<QString, QString> funcInfo = mProviderFuncInfo.value(providerFunc);

  QString mask = ("%1;%2");
  QStringList* result = new QStringList;
  result->append("[Header]Meta;Value");

                                    result->append(mask.arg("Name", funcInfo.value("Name")));
  if(funcInfo.contains("Version"))  result->append(mask.arg("Version", funcInfo.value("Version")));
  if(funcInfo.contains("Date"))     result->append(mask.arg("Date", funcInfo.value("Date")));
  if(funcInfo.contains("Author"))   result->append(mask.arg("Author", funcInfo.value("Author")));
                                    result->append(mask.arg("Purpose", funcInfo.value("Purpose")));
                                    result->append(mask.arg("Comment", funcInfo.value("Comment")));

  //if(funcInfo.contains("")) result->append(mask.arg("", funcInfo.value("")));

  mNewQuery = true;
  fillResultTable(result);
}

void AddFiPage::selectResultRow(int row, int /*column*/)
{
  mResultList->selectRow(row);

  // Place all table entries in the QHash
  for(int i = 0; i < mResultList->columnCount()/**/; ++i)
  {
    if(i > mPreparedHeader.size()) break; // Result list has more rows than Header promise
    mPreparedHeaderData.insert(mPreparedHeader.at(i), mResultList->item(row, i)->text());
  }
  //qDebug() << mPreparedHeaderData;

  // Quick and dirty hack to add market insert possibility
  if("Market" == mDisplayType)
  {
    mRefSymbol->setText("");
    mName->setText(mPreparedHeaderData.value("Currency"));
    int idx = mType->findText("Currency");
    mType->setCurrentIndex(idx);

    // Clear all symbol fields
    for(int i = 0; i < mPSMGrp.size(); ++i)
    {
      mPSMGrp.symbol(i)->setText("");
      mPSMGrp.searchBtn(i)->hide();
    }

    mPSMGrp.symbol(0)->setText(mPreparedHeaderData.value("CurrencySymbol"));
    idx = mPSMGrp.market(0)->findText("Forex");
    mPSMGrp.market(0)->setCurrentIndex(idx);
    idx = mPSMGrp.provider(0)->findText("ECB");
    mPSMGrp.provider(0)->setCurrentIndex(idx);
    return;
  }

  mRefSymbol->setText(mPreparedHeaderData.value("RefSymbol0"));
  mName->setText(mPreparedHeaderData.value("Name"));

  QString val = mPreparedHeaderData.value("Type");
  int idx = mType->findText(val);
  if((idx < 0) and !val.isEmpty()) emitMessage(FUNC, tr("FiType '%1' is unknown.").arg(val), eWarning);
  mType->setCurrentIndex(idx);

  // Search for Symbol/Market/Provider with or without a number suffix
  for(int i = 0; i < mPSMGrp.size(); ++i)
  {
    QString suffix = QString::number(i);
    QString symbol = mPreparedHeaderData.value("Symbol" + suffix);

    mPSMGrp.symbol(i)->setText(symbol);
    if(symbol.isEmpty())
    {
      mPSMGrp.searchBtn(i)->hide();
      continue; // No Symbol, don't set Provider/Market but don't break, be shure all Symbols are cleared
    }

    val = mPreparedHeaderData.value("Market" + suffix);
    idx = mPSMGrp.market(i)->findText(val);
    if((idx < 0) and !val.isEmpty()) emitMessage(FUNC, tr("Market '%1' is unknown.").arg(val), eWarning);
    mPSMGrp.market(i)->setCurrentIndex(idx);

    val = mPreparedHeaderData.value("Provider" + suffix);
    idx = mPSMGrp.provider(i)->findText(val);
    if((idx < 0) and !val.isEmpty()) emitMessage(FUNC, tr("SymbolType '%1' is unknown.").arg(val), eWarning);
    mPSMGrp.provider(i)->setCurrentIndex(idx);

    bool isProvider = mPSMGrp.provider(i)->itemData(idx).toBool();
    if(isProvider and (mPreparedHeaderData.value("Type") == "Index"))
    {
      mPSMGrp.searchBtn(i)->show();
    }
    else
    {
      mPSMGrp.searchBtn(i)->hide();
    }
  }
}

void AddFiPage::search()
{
  mNewQuery = true;
  mSearchCancelBtn->setText(cCancel);

  mProvider = mProviderSelector->currentText();

  if(mProviderFuncSelector->currentText() == "Fi")
  {
    mDisplayType = "Stock";
    searchFi();
  }
  else if(mProviderFuncSelector->currentText() == "CompList")
  {
    mDisplayType = "Index";
    searchIdx();
  }
  else if(mProviderFuncSelector->currentText() == "Market")
  {
    mDisplayType = "Market";
    searchMarket();
  }
  else
  {
    QStringList* result = new QStringList;
    result->append("[Header]Sorry");
    result->append("Not yet implemented");
    fillResultTable(result);
    mSearchCancelBtn->setText(cSearch);
  }
}

void AddFiPage::insertRow()
{
  int row = mResultList->currentRow();
  if(row < 0) row = 0;

  mResultList->insertRow(row);

  for(int c = 0; c < mResultList->columnCount(); ++c)
  {
    mResultList->setItem(row, c, new QTableWidgetItem(""));
  }

  selectResultRow(row, 0); // 0 is Dummy
}

void AddFiPage::removeRow()
{
  int row = mResultList->currentRow();
  if(row < 0) return;

  mResultList->removeRow(row);
  if(row > (mResultList->rowCount() - 1)) --row;
  if(row > -1) selectResultRow(row, 0); // 0 is a Dummy
}

void AddFiPage::searchFi()
{
//   mSearchCancelBtn->setText(cCancel);
  QString msg = tr("Search FI at %1 matched to '%2'...");
  emitMessage(FUNC, msg.arg(mProvider).arg(mSearchField->text()));
  QStringList parms(mSearchField->text());
  mScripter->showWaitWindow();
  mScripter->askProvider(mProvider, "fetchFi", parms);
}

void AddFiPage::searchIdx()
{
//   mSearchCancelBtn->setText(cCancel);
  QString msg = tr("Search Components at %1 for '%2'...");
  emitMessage(FUNC, msg.arg(mProvider).arg(mSearchField->text()));
  QStringList parms(mSearchField->text());
  mScripter->showWaitWindow();
  mScripter->askProvider(mProvider, "fetchCompList", parms);
//   QStringList* result = mScripter->askProvider(mProvider, "fetchCompList", parms);
//   if(!result)
//   {
//     result = new QStringList;
//     result->append(mScripter->errorText());
//     result->prepend("[Header]Error");
//
//     fillResultTable(result);
//   }
}

void AddFiPage::searchMarket()
{
//   mSearchCancelBtn->setText(cCancel);
  QString msg = tr("Search Market at %1 matched to '%2'...");
  emitMessage(FUNC, msg.arg(mProvider).arg(mSearchField->text()));
  QStringList parms(mSearchField->text());
  mScripter->showWaitWindow();
  mScripter->askProvider(mProvider, "fetchMarket", parms);
}

bool AddFiPage::importFails(const QString& func, const QString& data)
{
  mImporter->import(data);

  if(mImporter->hasError())
  {
    emitMessage(func, mImporter->formatMessages(), eError);
    return true;
  }

  return false;
}

void AddFiPage::fillResultTable(QStringList* data)
{
  if(mNewQuery)
  {
    mNewQuery = false;
    while(mResultList->rowCount()) mResultList->removeRow(0);
    while(mResultList->columnCount()) mResultList->removeColumn(0);

    mResultKeys.clear();
  }

  int r, c, re; // row, column, existing rows
  re = mResultList->rowCount();
  for(r = 0; r < data->size(); ++r)  // Rows
  {
    QStringList row = data->at(r).split(";");
    if(row.at(0).startsWith("[Header]"))
    {
      // Prepare the Header line
      // data->at(r) looks like : "[Header]Reuters;Name;Yahoo;Market;Quality;Notice
      // mPreparedHeader will becomes: "Symbol0", "Name", "Symbol1", "Market1", "Quality", "Notice", "Provider0", "Market0", "Provider1"
      // mPreparedHeaderData will becomes: QHash(("Market0", "NoMarket")("Market1", "")("Provider0", "Reuters")("Symbol0", "")("Provider1", "Yahoo")("Symbol1", "")("Notice", "")("Quality", "")("Name", ""))
      mImporter->reset();
      importFails(FUNC, data->at(r)); // Only to check for wrong MakeNameNice.conf.
                                             // But ignore the error if some
      mImporter->getPreparedHeaderData(mPreparedHeader, mPreparedHeaderData);
      //qDebug() << data->at(r);
      //qDebug() << mPreparedHeader;
      //qDebug() << mPreparedHeaderData;

      row[0].remove("[Header]");

      for(int i = mResultList->columnCount(); i < row.size(); ++i)
      {
        mResultList->insertColumn(i);
        mResultKeys.insert(row.at(i), i);
      }

      mResultList->setHorizontalHeaderLabels(row);
      data->removeAt(0);
      --r;
      continue;
    }

    mResultList->insertRow(r + re);

    if(!mResultList->columnCount()) continue; // Something is wrong with the Header

    for(c = 0; c < row.size(); ++c)  // Columns
    {
      if(c > mResultList->columnCount() - 1) mResultList->insertColumn(c);

      if(row.at(c).isEmpty())
      {
        // No data for that column,
        // insert a dummy to prevent a segfault when later the table is read
        QTableWidgetItem* newItem = new QTableWidgetItem("");
        mResultList->setItem(r + re, c, newItem);
        continue;
      }

      if(mResultList->horizontalHeaderItem(c)->text() == "Provider-Symbol-Market")
      {
        TWIB* box = new TWIB(row.at(c), r + re);
        connect(box, SIGNAL(requested(QString, int)), this, SLOT(addToDBbyTWIB(QString, int)));
        mResultList->setCellWidget(r + re, c, box);
      }
      else
      {
        if(mResultList->horizontalHeaderItem(c)->text() == "Name")
        {
          mImporter->makeNameNice(row[c]);
        }
        QTableWidgetItem* newItem = new QTableWidgetItem(row.at(c));
        mResultList->setItem(r + re, c, newItem);
      }
      //mResultList->item(r + re, c)->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    }

    for(; c < mResultList->columnCount(); ++c)
    {
      // More Header Keys listed than data,
      // insert a dummy to prevent a segfault when later the table is read
      QTableWidgetItem* newItem = new QTableWidgetItem("");
      mResultList->setItem(r + re, c, newItem);
    }
  }

  mHitCounter.setCount(r + re);

  mResultList->horizontalHeader()->hide();
  mResultList->resizeColumnsToContents();
  mResultList->resizeRowsToContents();
  mResultList->horizontalHeader()->show();

  delete data; // No longer needed
  mResultList->update();
}

void AddFiPage::loadSettings()
{
  mRcFile->beginGroup("AddFiPage");

  mProviderSelector->blockSignals(true);
  mProviderFuncSelector->blockSignals(true);

  mProviderSelector->setCurrentIndex(mProviderSelector->findText(mRcFile->getST("LastProvider")));
  providerChanged(mRcFile->getST("LastProvider"));
  mProviderFuncSelector->setCurrentIndex(mProviderFuncSelector->findText(mRcFile->getST("LastScript")));

  mProviderFuncSelector->blockSignals(false);
  mProviderSelector->blockSignals(false);

  mType->setCurrentIndex(mType->findText(mRcFile->getST("Type")));

  for(int i = 0; i < mPSMGrp.size(); ++i)
  {
    QString suffix = QString::number(i);

    mPSMGrp.market(i)->setCurrentIndex(mPSMGrp.market(i)->findText(mRcFile->getST("Market" + suffix)));
    mPSMGrp.provider(i)->setCurrentIndex(mPSMGrp.provider(i)->findText(mRcFile->getST("Provider" + suffix)));
  }

  mRcFile->endGroup(); // "AddFiPage"
}

void AddFiPage::saveSettings()
{
  mRcFile->beginGroup("AddFiPage");

  mRcFile->set("LastProvider", mProviderSelector->currentText());
  mRcFile->set("LastScript", mProviderFuncSelector->currentText());

  mRcFile->set("Type", mType->currentText());

  for(int i = 0; i < mPSMGrp.size(); ++i)
  {
    QString suffix = QString::number(i);

    mRcFile->set("Market" + suffix, mPSMGrp.market(i)->currentText());
    mRcFile->set("Provider" + suffix, mPSMGrp.provider(i)->currentText());
  }

  mRcFile->endGroup(); // "AddFiPage"
}

void AddFiPage::searchOrCancel()
{
  if(mSearchCancelBtn->hasFocus())
  {
    if(mSearchCancelBtn->text() == cCancel)
    {
      mScripter->stopRunning();
      emitMessage(FUNC, tr("Script canceled."));
    }
    else search();
  }
}

void AddFiPage::scriptFinished()
{
  mSearchCancelBtn->setText(cSearch);

  if(mScripter->hasError())
  {
    // For 'historical reasons', and because it looks so cool,
    // we fill the result table with the error message...
    QString errorMsg = mScripter->formatMessages("%x");
    QStringList* result = new QStringList;
    result->append("[Header]Error");
    result->append(errorMsg);
    fillResultTable(result);

    // ...and of cause the log book
    emitMessage(FUNC, errorMsg, eError);
  }
  else
  {
    emitMessage(FUNC, mHitCounter.text());
  }
}

void AddFiPage::searchCompBtnClicked(int idx)
{
  mNewQuery = true;
  mSearchCancelBtn->setText(cCancel);
  mProvider = mPSMGrp.provider(idx)->currentText();
  mSearchField->setText(mPSMGrp.symbol(idx)->text());
  mDisplayType = "Index";
  searchIdx();
}

void AddFiPage::addToDB()
{
  if("Market" == mDisplayType)
  {
    addMarketToDB();
    return;
  }

  // Build a hopefully useful log message
  QStringList msg;

  if(!mName->text().isEmpty())
  {
    QString intro = tr("Add new FI to DB:");
    intro.append(" Name=" + mName->text());
    intro.append(", Type=" + mType->currentText());
    if(!mRefSymbol->text().isEmpty()) intro.append(", RefSymbol=" + mRefSymbol->text());

    msg.append(intro);
  }
  else
  {
    QString intro = tr("Add more Symbols to DB:");
    if(!mRefSymbol->text().isEmpty()) intro.append(" RefSymbol=" + mRefSymbol->text());
    msg.append(intro);
  }

  for(int i = 0; i < mPSMGrp.size(); ++i)
  {
    if(mPSMGrp.symbol(i)->text().isEmpty()) continue;

    QString suffix = QString::number(i);
    if(0 == i) suffix = ""; // Looks nicer without "0"

    msg.append("Symbol" + suffix + "=" +
                mPSMGrp.symbol(i)->text() + "-" +
                mPSMGrp.provider(i)->currentText() + "-" +
                mPSMGrp.market(i)->currentText());
  }

  emitMessage(FUNC, msg.join(", "));

  // Build Header and Data Line
  QStringList header;
  QStringList data;

  if(!mRefSymbol->text().isEmpty())
  {
    header << "RefSymbol";
    data   << mRefSymbol->text();
  }

  if(!mName->text().isEmpty())
  {
    header << "Name" << "Type";
    data << mName->text() << mType->currentText();
  }

  for(int i = 0; i < mPSMGrp.size(); ++i)
  {
    if(mPSMGrp.symbol(i)->text().isEmpty()) continue;

    header << "Provider" << "Symbol" << "Market";
    data << mPSMGrp.provider(i)->currentText()
         << mPSMGrp.symbol(i)->text()
         << mPSMGrp.market(i)->currentText();
  }

  header[0] = "[Header]" + header.at(0);

  // Import the stuff
  mImporter->reset();
  if(importFails(FUNC, header.join(";"))) return;
  if(importFails(FUNC, data.join(";"))) return;

  // Looks good, clear the edit fields
  mRefSymbol->setText("");
  mName->setText("");
  for(int i = 0; i < mPSMGrp.size(); ++i)
  {
    mPSMGrp.symbol(i)->setText("");
    mPSMGrp.searchBtn(i)->hide();
  }

  emitMessage(FUNC, tr("New FI added to DB."));
}

void AddFiPage::addMarketToDB()
{
  // Quick and dirty hack to add market insert possibility
  // Build Header and Data Line
  QStringList header;
  QStringList data;

  header << "MarketName" << "MarketSymbol" << "Currency" << "CurrencySymbol"
         << "OpenTime" << "CloseTime" << "Lunch" << "Location";

  data.append(mPreparedHeaderData.value("MarketName"));
  data.append(mPreparedHeaderData.value("MarketSymbol"));
  data.append(mPreparedHeaderData.value("Currency"));
  data.append(mPreparedHeaderData.value("CurrencySymbol"));
  data.append(mPreparedHeaderData.value("OpenTime"));
  data.append(mPreparedHeaderData.value("CloseTime"));
  data.append(mPreparedHeaderData.value("Lunch"));
  data.append(mPreparedHeaderData.value("Location"));

  header[0] = "[Header]" + header.at(0);

  // Import the stuff
  mImporter->reset();
  if(importFails(FUNC, header.join(";"))) return;
  if(importFails(FUNC, data.join(";"))) return;

  // Looks good, clear the edit fields
  // and reload markets
  QStringList marketList = mFilu->getMarketNames();
  mRefSymbol->setText("");
  mName->setText("");
  for(int i = 0; i < mPSMGrp.size(); ++i)
  {
    mPSMGrp.symbol(i)->setText("");
    mPSMGrp.market(i)->clear();
    mPSMGrp.market(i)->addItems(marketList);
    mPSMGrp.searchBtn(i)->hide();
  }

  emitMessage(FUNC, tr("New market added to DB."));
}

void AddFiPage::addAllToDB()
{
  if(mResultList->columnCount() < 1) return;

  QString txt = tr("\nAdd all %1 FIs to the DataBase?\t").arg(mResultList->rowCount());
  int ret = QMessageBox::question(this, mIconText, txt
                                  , QMessageBox::Ok | QMessageBox::Cancel
                                  , QMessageBox::Cancel);

  if(ret != QMessageBox::Ok) return;

  txt = tr("Going to add %1 FIs...").arg(mResultList->rowCount());
  emitMessage(FUNC, txt);

  QString header = "[Header]";
  header.append(mResultList->horizontalHeaderItem(0)->text());
  for(int i = 1; i < mResultList->columnCount(); ++i)
  {
    header.append(";" + mResultList->horizontalHeaderItem(i)->text());
  }

  emitMessage(FUNC, header);
  mImporter->reset();
  if(importFails(FUNC, header)) return;

  if("Index" == mDisplayType)
  {
    txt = tr("FIs will added as components of %1").arg(mSearchField->text());
    emitMessage(FUNC, txt);
    if(importFails(FUNC, "[CompList]" + mSearchField->text())) return;
  }

  for(int r = 0; r < mResultList->rowCount(); ++r)
  {
    QStringList data;
    for(int c = 0; c < mResultList->columnCount(); ++c)
    {
      data.append(mResultList->item(r, c)->text());
    }
    emitMessage(FUNC, data.join(";"));
    if(importFails(FUNC, data.join(";")) and ("Index" == mDisplayType)) return; // Ignore the Error if no ComponentList
  }

  if("Index" == mDisplayType)
  {
    emitMessage(FUNC, tr("End of Component list."));
    if(importFails(FUNC, "[CompListEnd]")) return;
  }
  else if("Market" == mDisplayType)
  {
    // Looks good, reload markets
    QStringList marketList = mFilu->getMarketNames();
    for(int i = 0; i < mPSMGrp.size(); ++i)
    {
      mPSMGrp.market(i)->clear();
      mPSMGrp.market(i)->addItems(marketList);
    }
  }
}

void AddFiPage::addToDBbyTWIB(QString psm, int row)
{
  QStringList psmSplitted = psm.split(" ");

  if(mDisplayType == "Stock")
  {
//     qDebug() << "add to db";

    FiTuple fi(1);

    SymbolTuple* symbol;
    QList<int> sl = mResultKeys.values("Symbol");
    QList<int> msl = mResultKeys.values("RefSymbol");
    QList<int> psml = mResultKeys.values("Provider-Symbol-Market");
    int count = sl.size() + msl.size() + psml.size() + 1;
    if(count > 1)
    {
      symbol = new SymbolTuple(count);

      for(int i = 1; i <= sl.size(); ++i)
      {
        symbol->next();
        symbol->set(mResultList->item(row, sl.at(i - 1))->text(), "", "");
      }

      for(int i = 1; i <= msl.size(); ++i)
      {
        symbol->next();
        symbol->set(mResultList->item(row, msl.at(i - 1))->text(), "", "");
      }

      for(int i = 1; i <= psml.size(); ++i)
      {
        TWIB* twib = dynamic_cast<TWIB*>(mResultList->cellWidget(row, psml.at(i - 1)));
        if(!twib) continue;
        QStringList txt = twib->text().split(" ");
        symbol->next();
        symbol->set(txt.at(1), "", "");
      }
    }
    else
    {
      symbol = new SymbolTuple(1);
    }

    symbol->next();
    symbol->set(psmSplitted.at(1), psmSplitted.at(2), psmSplitted.at(0));

    fi.next(); // Set on first position

    fi.set(mResultList->item(row, mResultKeys.value("Name"))->text()
         , mResultList->item(row, mResultKeys.value("Type"))->text()
         , symbol);

    // Here is the beef
    mFilu->addFiCareful(fi);

    if(mFilu->hasError())
    {
      //printError("-addfi");
//       qDebug() << "agentf -addFi: Oops! new FI not added to DB";
//       qDebug() << "\tare you sure that FI type, market and symbol type exist?";
    }
  }
}

PSMGrp::PSMGrp()
      : QObject()
      , mCount(0)
{
  connect(&mBtnGrp, SIGNAL(buttonClicked(int)), this, SIGNAL(searchCompBtnClicked(int)));
}

PSMGrp::~PSMGrp()
{
  for(int i = 0; i < mCount - 1; ++i)
  {
    delete mProvider.at(i);
    delete mSymbol.at(i);
    delete mMarket.at(i);
  }
}

int PSMGrp::addOne()
{
  mProvider.append(new QComboBox);
  mSymbol.append(new LineEdit);
  mMarket.append(new QComboBox);

  QPushButton* btn = new QPushButton(tr("Search Comp"));
  btn->setToolTip(tr("Search the Components of this Index"));
  btn->hide();
  mBtnGrp.addButton(btn, mCount);

  return ++mCount;
}

int PSMGrp::size()
{
  return mCount;
}

QComboBox* PSMGrp::provider(int i)
{
  return (i < 0) or (i > (mCount -1)) ? 0 : mProvider.at(i);
}

LineEdit* PSMGrp::symbol(int i)
{
  return (i < 0) or (i > (mCount -1)) ? 0 : mSymbol.at(i);
}

QComboBox* PSMGrp::market(int i)
{
  return (i < 0) or (i > (mCount -1)) ? 0 : mMarket.at(i);
}

QAbstractButton* PSMGrp::searchBtn(int i)
{
  return (i < 0) or (i > (mCount -1)) ? 0 : mBtnGrp.button(i);
}

TWIB::TWIB(const QString& txt, int row, QWidget* parent) : QWidget(parent)
{
  mButton = new QToolButton;
  mButton->setAutoRaise(true);
  mButton->setArrowType(Qt::DownArrow);
  connect(mButton, SIGNAL(clicked()), this, SLOT(clicked()));

  mField = new QLabel(txt);

  QGridLayout* layout = new QGridLayout;
  layout->setMargin(0);
  layout->addWidget(mButton, 0, 0);
  layout->addWidget(mField, 0, 1);

  setLayout(layout);

  mRow = row;
}

TWIB::~TWIB()
{

}

void TWIB::clicked()
{
  emit requested(mField->text(), mRow);
}

QString TWIB::text()
{
  return mField->text();
}
