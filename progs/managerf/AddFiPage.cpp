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

  QGroupBox* searchGroup = new QGroupBox(tr("Add a new FI to the Data Base"));

  mSearchCancelBtn = new QPushButton;
  mSearchCancelBtn->setText("Search");
  connect(mSearchCancelBtn, SIGNAL(clicked()), this, SLOT(searchOrCancel()));

  mProviderSelector = new QComboBox;
  mProviderPath = mRcFile->getST("ProviderPath");
  QDir dir(mProviderPath);
  mProviderSelector->insertItems(0, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot));
  mProviderSelector->setCurrentIndex(mProviderSelector->findText("Filu"));

  mSearchField = new SearchField;

  connect(mSearchField, SIGNAL(returnPressed()), this, SLOT(search()));

  mTypeSelector = new QComboBox;
  mTypeSelector->insertItem(1, "Search FI");
  mTypeSelector->insertItem(2, "Search Index");

  mResultList = new QTableWidget;
  mResultList->verticalHeader()->hide();
  mResultList->setShowGrid(false);
  connect(mResultList, SIGNAL(cellClicked(int, int)), this, SLOT(selectResultRow(int, int)));

  //
  // The add area
  mAddBtn = new QPushButton;
  mAddBtn->setText(tr("Add to DB"));
  connect(mAddBtn, SIGNAL(clicked()), this, SLOT(addToDB()));

  QFontMetrics f(font());
  int w = f.width("X");
  mRefSymbol = new SearchField;
  mName = new QLineEdit;
  mName->setMinimumWidth(w * 30);

  mType = new QComboBox;
  QStringList types;
  mFilu->getFiType(types);
  mType->addItems(types);

  // Read all symbol types out of the DB
  SymbolTypeTuple* symbolTypes = mFilu->getSymbolTypes(Filu::eAllTypes);
  if(!check4FiluError("AddFiPage::createPage: " + tr("ERROR while exec GetSymbolTypes.sql")))
  {
    if(!symbolTypes)
    {
      // Purposely no tr()
      addErrorText("AddFiPage::createPage: You should never read this:", eCritical);
    }
  }

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
    emit message("AddFiPage::createPage: " + tr("No markets found"));
  }

  // Build the edit line layout
  QGridLayout* addEditLineLO = new QGridLayout;
  addEditLineLO->addWidget( new QLabel("RefSymbol"), 0, 0);
  addEditLineLO->addWidget(mRefSymbol              , 1, 0);
  addEditLineLO->setColumnStretch(0, 3);
  addEditLineLO->addWidget( new QLabel("Name")     , 0, 1);
  addEditLineLO->addWidget(mName                   , 1, 1, 1, 3); // Span over three columns
  addEditLineLO->setColumnStretch(3, 2);                          // Expand the empty column
  addEditLineLO->addWidget( new QLabel("Type")     , 0, 4);
  addEditLineLO->addWidget(mType                   , 1, 4);

  addEditLineLO->addWidget(mAddBtn, 1, 5);

  addEditLineLO->addWidget( new QLabel("Symbol"), 2, 0);
  addEditLineLO->addWidget( new QLabel("Market"), 2, 1);
  addEditLineLO->addWidget( new QLabel("Provider"), 2, 2);

  for(int i = 0; i < 3; ++i)
  {
    mPSMGrp.addOne();

    if(symbolTypes)
    {
      symbolTypes->rewind();
      while(symbolTypes->next())
      {
        mPSMGrp.provider(i)->insertItem(0, symbolTypes->caption());
      }
    }

    mPSMGrp.market(i)->addItems(marketList);

    addEditLineLO->addWidget(mPSMGrp.symbol(i), i + 3, 0);
    addEditLineLO->addWidget(mPSMGrp.market(i), i + 3, 1);
    addEditLineLO->addWidget(mPSMGrp.provider(i), i + 3, 2);
  }

  //
  // Build the main layout
  QGridLayout* searchLayout = new QGridLayout;
  searchLayout->addWidget(mProviderSelector  , 0, 0);
  searchLayout->addWidget(mTypeSelector      , 0, 1);
  searchLayout->addWidget(mSearchField       , 0, 2);
  searchLayout->setColumnStretch(2, 2);
  searchLayout->addWidget(mSearchCancelBtn   , 0, 3);
  searchLayout->addWidget(&mHitCounter       , 0, 4);
  searchLayout->addWidget(mResultList        , 1, 0, 1, 6); // Span over two more than existing columns...
  searchLayout->setColumnStretch(4, 2);                     // ...and stretch...
  searchLayout->setColumnStretch(5, 2);                     // ...these more
  searchLayout->addLayout(addEditLineLO      , 2, 0, 1, 5);

  searchGroup->setLayout(searchLayout);

//  QHBoxLayout* configLayout = new QHBoxLayout;
//  configLayout->addWidget(searchGroup);
  //configLayout->addStretch(1);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(searchGroup);
  //mainLayout->addLayout(configLayout);
  mainLayout->addStretch(1);
  setLayout(mainLayout);
}

void AddFiPage::showEvent(QShowEvent * /*event*/)
{
  mSearchField->setFocus();
}

void AddFiPage::selectResultRow( int row, int /*column*/)
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

  int idx = mType->findText(mPreparedHeaderData.value("Type"));
  if(idx < 0) emit message("AddFiPage::selectResultRow: " + tr("Unknown FiType: ") + mPreparedHeaderData.value("Type"), eWarning);
  mType->setCurrentIndex(idx);

  // Search for Symbol/Market/Provider with or without a number suffix
  for(int i = 0; i < mPSMGrp.size(); ++i)
  {
    QString suffix = QString::number(i);
    QString symbol = mPreparedHeaderData.value("Symbol" + suffix);


    mPSMGrp.symbol(i)->setText(symbol);
    if(symbol.isEmpty()) continue; // No Symbol, don't set Provider/Market but don't break, be shure all Symbols are cleared

    idx = mPSMGrp.market(i)->findText(mPreparedHeaderData.value("Market" + suffix));
    if(idx < 0) emit message("AddFiPage::selectResultRow: " + tr("Unknown Market: ") + mPreparedHeaderData.value("Market" + suffix), eWarning);
    mPSMGrp.market(i)->setCurrentIndex(idx);

    idx = mPSMGrp.provider(i)->findText(mPreparedHeaderData.value("Provider" + suffix));
    if(idx < 0) emit message("AddFiPage::selectResultRow: " + tr("Unknown SymbolType: ") + mPreparedHeaderData.value("Provider" + suffix), eWarning);
    mPSMGrp.provider(i)->setCurrentIndex(idx);
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

void AddFiPage::searchFi()
{
  emit message("AddFiPage::searchFi: " + tr("Search FI matched to '") + mSearchField->text() + "'...");
  QStringList parms(mSearchField->text());
  mScripter->showWaitWindow();
  mScripter->askProvider(mProvider, "fetchFi", parms);
}

void AddFiPage::searchIdx()
{
  emit message("AddFiPage::searchIdx: " + tr("Search Index matched to '") + mSearchField->text() + "'...");
  QStringList parms(mSearchField->text());
  mScripter->showWaitWindow();
  mScripter->askProvider(mProvider, "fetchIdx", parms);
//   QStringList* result = mScripter->askProvider(mProvider, "fetchIdx", parms);
//   if(!result)
//   {
//     result = new QStringList;
//     result->append(mScripter->errorText());
//     result->prepend("[Header]Error");
//
//     fillResultTable(result);
//   }
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
      mImporter->import(data->at(r));
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

    for(c = 0; c < row.size(); ++c)  // Columns
    {
      if(c > mResultList->columnCount() - 1)
        mResultList->insertColumn(c);

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
      emit message("AddFiPage::searchOrCancel: " + tr("Script canceled"));
    }
    else search();
  }
}

void AddFiPage::scriptFinished()
{
  mSearchCancelBtn->setText("Search");

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
    //emit message("AddFiPage::scriptFinished: " + tr("Huston, we have a problem."), eError);
    emit message("AddFiPage::scriptFinished: " + errorMsg, eError);
  }
  else
  {
    emit message("AddFiPage::scriptFinished: " + mHitCounter.text());
  }
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
  emit message(msg.join(""));

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
  mImporter->import(header);
  mImporter->import(data);

  emit message("FIXME Add error/success message");
  // emit message("Fail to add FI");
  // emit message("AddFiPage::addToDB: " +tr("New FI added to DB"));

  // Looks good, clear the edit fields
  mRefSymbol->setText("");
  mName->setText("");
  for(int i = 0; i < mPSMGrp.size(); ++i) mPSMGrp.symbol(i)->setText("");

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

PSMGrp::PSMGrp() : mCount(0)
{}

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
  ++mCount;

  mProvider.append(new QComboBox);
  mSymbol.append(new SearchField);
  mMarket.append(new QComboBox);

  return mCount;
}

int PSMGrp::size()
{
  return mCount;
}

QComboBox* PSMGrp::provider(int i)
{
  return (i < 0) or (i > (mCount -1)) ? 0 : mProvider.at(i);
}

SearchField* PSMGrp::symbol(int i)
{
  return (i < 0) or (i > (mCount -1)) ? 0 : mSymbol.at(i);
}

QComboBox* PSMGrp::market(int i)
{
  return (i < 0) or (i > (mCount -1)) ? 0 : mMarket.at(i);
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
