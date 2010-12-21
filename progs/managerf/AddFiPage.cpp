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

#include "AddFiPage.h"

#include "LineEdit.h"
#include "SearchField.h"
#include "Script.h"
#include "Importer.h"

AddFiPage::AddFiPage(FClass* parent)
         : ManagerPage(parent)
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
  mIcon = QIcon(":/icons/configure.xpm");
  mIconText = tr("Add FI");

  mScripter = new Script(this);
  connect(mScripter, SIGNAL(newData(QStringList *))
          , this, SLOT(fillResultTable(QStringList *)));
  connect(mScripter, SIGNAL(finished())
          , this, SLOT(scriptFinished()));

  mImporter = new Importer(this);
  if(mImporter->hasError()) addErrorText(mImporter->errorText(), eWarning);

  QGroupBox* searchGroup = new QGroupBox(tr("Add a new FI to the Data Base"));

  mSearchCancelBtn = new QPushButton;
  mSearchCancelBtn->setText(tr("Search"));
  connect(mSearchCancelBtn, SIGNAL(clicked()), this, SLOT(searchOrCancel()));

  mProviderSelector = new QComboBox;
  QDir dir(mRcFile->getST("ProviderPath"));
  mProviderSelector->insertItems(0, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot));
  mProviderSelector->setCurrentIndex(mProviderSelector->findText("Filu"));

  mSearchField = new SearchField(this);

  connect(mSearchField, SIGNAL(returnPressed()), this, SLOT(search()));

  mTypeSelector = new QComboBox;
  mTypeSelector->insertItem(1, "Search FI");
  mTypeSelector->insertItem(2, "Search Index");

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

  mResultList = new QTableWidget;
  mResultList->verticalHeader()->hide();
  mResultList->setShowGrid(false);
  connect(mResultList, SIGNAL(cellClicked(int, int)), this, SLOT(selectResultRow(int, int)));

  //
  // The add area
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
  QStringList types;
  mFilu->getFiType(types);
  mType->addItems(types);

  // Read all markets out of the DB
  mFilu->setMarketName("");
  MarketTuple* markets = mFilu->getMarket();
  QStringList marketList;
  if(markets)
  {
    while(markets->next()) marketList.append(markets->name());
    marketList.sort();
    delete markets;
  }
  else
  {
    QString msg = tr("No Markets found");
    if(!check4FiluError("AddFiPage::createPage: " + msg))
    {
      addErrorText("AddFiPage::createPage: " + msg);
    }
  }

  // Build the edit line layout
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
    QString msg = tr("No SymbolTypes found");
    if(!check4FiluError("AddFiPage::createPage1: " + msg))
    {
      addErrorText("AddFiPage::createPage2: " + tr("No SymbolTypes found"));
    }
  }

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
  topLine->addWidget(mTypeSelector);
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
  searchLayout->addLayout(editNameLO);
  searchLayout->addLayout(editSymbolLO);

  //
  // Build the main layout

  searchGroup->setLayout(searchLayout);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(searchGroup);
  mainLayout->addStretch(1);
  setLayout(mainLayout);
}

void AddFiPage::showEvent(QShowEvent * /*event*/)
{
  mSearchField->setFocus();
}

void AddFiPage::selectResultRow(int row, int /*column*/)
{
  mResultList->selectRow(row);

  // Place all table entries in the QHash
  for(int i = 0; i < mResultList->columnCount(); ++i)
  {
    mPreparedHeaderData.insert(mPreparedHeader.at(i), mResultList->item(row, i)->text());
  }
  //qDebug() << mPreparedHeaderData;

  mRefSymbol->setText(mPreparedHeaderData.value("RefSymbol0"));
  mName->setText(mPreparedHeaderData.value("Name"));

  QString val = mPreparedHeaderData.value("Type");
  int idx = mType->findText(val);
  if((idx < 0) and !val.isEmpty()) emit message(Q_FUNC_INFO, tr("Unknown FiType: ") + val, eWarning);
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
    if((idx < 0) and !val.isEmpty()) emit message(Q_FUNC_INFO, tr("Unknown Market: ") + val, eWarning);
    mPSMGrp.market(i)->setCurrentIndex(idx);

    val = mPreparedHeaderData.value("Provider" + suffix);
    idx = mPSMGrp.provider(i)->findText(val);
    if((idx < 0) and !val.isEmpty()) emit message(Q_FUNC_INFO, tr("Unknown SymbolType: ") + val, eWarning);
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
  mSearchCancelBtn->setText("Cancel");

  mProvider = mProviderSelector->currentText();

  if(mTypeSelector->currentText() == "Search FI")
  {
    mDisplayType = "Stock";
    searchFi();
  }
  else if(mTypeSelector->currentText() == "Search Index")
  {
    mDisplayType = "Index";
    searchIdx();
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
  emit message(Q_FUNC_INFO, tr("Search FI matched to '") + mSearchField->text() + "'...");
  QStringList parms(mSearchField->text());
  mScripter->showWaitWindow();
  mScripter->askProvider(mProvider, "fetchFi", parms);
}

void AddFiPage::searchIdx()
{
  emit message(Q_FUNC_INFO, tr("Search Index matched to '") + mSearchField->text() + "'...");
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

bool AddFiPage::importFails(const QString& func, const QString& data)
{
  mImporter->import(data);

  if(mImporter->hasError())
  {
    emit message(func, mImporter->errorText().join("\n"), eError);
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
      // mPreparedHeader will became: "Symbol0", "Name", "Symbol1", "Market1", "Quality", "Notice", "Provider0", "Market0", "Provider1"
      // mPreparedHeaderData will became: QHash(("Market0", "NoMarket")("Market1", "")("Provider0", "Reuters")("Symbol0", "")("Provider1", "Yahoo")("Symbol1", "")("Notice", "")("Quality", "")("Name", ""))
      mImporter->reset();
      importFails(Q_FUNC_INFO, data->at(r)); // Only to check for wrong MakeNameNice.conf.
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
  mResultList->horizontalHeader()->show();

  delete data; // No longer needed
  mResultList->update();
}

void AddFiPage::loadSettings()
{
  mRcFile->beginGroup("AddFiPage");

  mProviderSelector->setCurrentIndex(mProviderSelector->findText(mRcFile->getST("LastProvider")));
  mTypeSelector->setCurrentIndex(mTypeSelector->findText(mRcFile->getST("LastScript")));

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
  mRcFile->set("LastScript", mTypeSelector->currentText());

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
    if(mSearchCancelBtn->text() == "Cancel")
    {
      mScripter->stopRunning();
      emit message(Q_FUNC_INFO, tr("Script canceled"));
    }
    else search();
  }
}

void AddFiPage::scriptFinished()
{
  mSearchCancelBtn->setText(tr("Search"));

  if(mScripter->hasError())
  {
    // For 'historical reasons', and because it looks so cool,
    // we fill the result table with the error message...
    QString errorMsg = mScripter->errorText().at(0); // We keep it simple, assume no more than one line
    QStringList* result = new QStringList;
    result->append("[Header]Error");
    result->append(errorMsg);
    fillResultTable(result);

    // ...and of cause the log book
    emit message(Q_FUNC_INFO, errorMsg, eError);
  }
  else
  {
    emit message(Q_FUNC_INFO, mHitCounter.text());
  }
}

void AddFiPage::searchCompBtnClicked(int idx)
{
  mNewQuery = true;
  mSearchCancelBtn->setText("Cancel");
  mProvider = mPSMGrp.provider(idx)->currentText();
  mSearchField->setText(mPSMGrp.symbol(idx)->text());
  mDisplayType = "Index";
  searchIdx();
}

void AddFiPage::addToDB()
{
  // Build a hopefully useful log message
  QStringList msg;

  if(!mName->text().isEmpty())
  {
    msg.append(tr("Add new FI to DB: "));
    msg.append("Name=" + mName->text() + ", ");
    msg.append("Type=" + mType->currentText() + ", ");
    if(!mRefSymbol->text().isEmpty()) msg.append("RefSymbol=" + mRefSymbol->text() + ", ");
  }
  else
  {
    msg.append(tr("Add more Symbols to DB: "));
    if(!mRefSymbol->text().isEmpty()) msg.append("RefSymbol=" + mRefSymbol->text() + ", ");
  }

  for(int i = 0; i < mPSMGrp.size(); ++i)
  {
    if(mPSMGrp.symbol(i)->text().isEmpty()) continue;

    QString suffix = QString::number(i);
    if(0 == i) suffix = ""; // Looks nicer without "0"

    msg.append("Symbol" + suffix + "=" +
                mPSMGrp.symbol(i)->text() + "-" +
                mPSMGrp.provider(i)->currentText() + "-" +
                mPSMGrp.market(i)->currentText() + ", ");
  }
  // Remove last ", "
  QString last = msg.at(msg.size() - 1);
  last.chop(2);
  msg.replace((msg.size() - 1), last);
  emit message(Q_FUNC_INFO, msg.join(""));

  // Build Header and Data Line
  QString header = "[Header]";
  QString data;

  if(!mRefSymbol->text().isEmpty())
  {
    header.append("RefSymbol;");
    data.append(mRefSymbol->text() + ";");
  }

  if(!mName->text().isEmpty())
  {
    header.append("Name;Type;");
    data.append(mName->text() + ";");
    data.append(mType->currentText() + ";");
  }

  for(int i = 0; i < mPSMGrp.size(); ++i)
  {
    if(mPSMGrp.symbol(i)->text().isEmpty()) continue;

    header.append("Provider;Symbol;Market;");
    data.append(mPSMGrp.provider(i)->currentText() + ";" +
                mPSMGrp.symbol(i)->text() + ";" +
                mPSMGrp.market(i)->currentText() + ";");
  }
  // Do it very nice, remove last ";"
  header.chop(1);
  data.chop(1);

  // Import the stuff
  mImporter->reset();
  if(importFails(Q_FUNC_INFO, header)) return;
  if(importFails(Q_FUNC_INFO, data)) return;

  // Looks good, clear the edit fields
  mRefSymbol->setText("");
  mName->setText("");
  for(int i = 0; i < mPSMGrp.size(); ++i)
  {
    mPSMGrp.symbol(i)->setText("");
    mPSMGrp.searchBtn(i)->hide();
  }

  emit message(Q_FUNC_INFO, tr("New FI added to DB"));
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
  emit message(Q_FUNC_INFO, txt);

  QString header = "[Header]";
  header.append(mResultList->horizontalHeaderItem(0)->text());
  for(int i = 1; i < mResultList->columnCount(); ++i)
  {
    header.append(";" + mResultList->horizontalHeaderItem(i)->text());
  }

  emit message(Q_FUNC_INFO, header);
  mImporter->reset();
  if(importFails(Q_FUNC_INFO, header)) return;

  if("Index" == mDisplayType)
  {
    txt = tr("FIs will added as components of %1").arg(mSearchField->text());
    emit message(Q_FUNC_INFO, txt);
    if(importFails(Q_FUNC_INFO, "[CompList]" + mSearchField->text())) return;
  }

  for(int r = 0; r < mResultList->rowCount(); ++r)
  {
    QStringList data;
    for(int c = 0; c < mResultList->columnCount(); ++c)
    {
      data.append(mResultList->item(r, c)->text());
    }
    emit message(Q_FUNC_INFO, data.join(";"));
    if(importFails(Q_FUNC_INFO, data.join(";")) and ("Index" == mDisplayType)) return; // Ignore the Error if no ComponentList
  }

  if("Index" == mDisplayType)
  {
    emit message(Q_FUNC_INFO, tr("End of Component list"));
    if(importFails(Q_FUNC_INFO, "[CompListEnd]")) return;
  }
}

void AddFiPage::addToDBbyTWIB(QString psm, int row)
{
  QStringList psmSplitted = psm.split(" ");

  if(mDisplayType == "Stock")
  {
    qDebug() << "add to db";

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
        symbol->setCaption(mResultList->item(row, sl.at(i - 1))->text());
        symbol->setMarket("");
        symbol->setOwner("");
      }

      for(int i = 1; i <= msl.size(); ++i)
      {
        symbol->next();
        symbol->setCaption(mResultList->item(row, msl.at(i - 1))->text());
        symbol->setMarket("");
        symbol->setOwner("");
      }

      for(int i = 1; i <= psml.size(); ++i)
      {
        TWIB* twib = dynamic_cast<TWIB*>(mResultList->cellWidget(row, psml.at(i - 1)));
        if(!twib) continue;
        QStringList txt = twib->text().split(" ");
        symbol->next();
        symbol->setCaption(txt.at(1));
        symbol->setMarket("");
        symbol->setOwner("");
      }
    }
    else
    {
      symbol = new SymbolTuple(1);
    }

    symbol->next();
    symbol->setCaption(psmSplitted.at(1));
    symbol->setMarket(psmSplitted.at(2));
    symbol->setOwner(psmSplitted.at(0));

    fi.next(); // Set on first position

    fi.setSymbol(symbol);
    fi.setName(mResultList->item(row, mResultKeys.value("Name"))->text());
    fi.setType(mResultList->item(row, mResultKeys.value("Type"))->text());

    // Here is the beef
    mFilu->addFiCareful(fi);

    if(mFilu->hadTrouble())
    {
      //printError("-addfi");
      qDebug() << "agentf -addFi: Oops! new FI not added to DB";
      qDebug() << "\tare you sure that FI type, market and symbol type exist?";
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
