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

AddFiPage::AddFiPage(ManagerF* parent) : ManagerPage(parent)
{
  mIcon = QIcon(":/icons/configure.xpm");
  mIconText = tr("Add new FI");
  createPage();
}

AddFiPage::AddFiPage(FWidget* parent) : ManagerPage(parent)
{
  mIcon = QIcon(":/icons/configure.xpm");
  mIconText = tr("Add new FI");
  createPage();
}

AddFiPage::~AddFiPage()
{
  delete mScripter;
}

void AddFiPage::createPage()
{
  readSettings();

  mScripter = new Script(this);
  connect(mScripter, SIGNAL(newData(QStringList *))
          , this, SLOT(fillResultTable(QStringList *)));
  connect(mScripter, SIGNAL(finished())
          , this, SLOT(scriptFinished()));

  QGroupBox* searchGroup = new QGroupBox(tr("Add new FI to Data Base"));

  mSearchCancelBtn = new QPushButton;
  mSearchCancelBtn->setText("Search");
  connect(mSearchCancelBtn, SIGNAL(clicked()), this, SLOT(searchOrCancel()));

  mProviderSelector = new QComboBox;
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
  // the add area
  mAddBtn = new QPushButton;
  mAddBtn->setText("Add to DB");
  connect(mAddBtn, SIGNAL(clicked()), this, SLOT(addToDB()));

  QFontMetrics f(font());
  int w = f.width("X");
  mAnySymbol = new QLineEdit;
  mName = new QLineEdit;
  mName->setMinimumWidth(w * 30);
  mType = new QLineEdit;
  mSymbol = new QLineEdit;
  mMarket = new QComboBox;
  mMarket->setMinimumContentsLength(10);
  mMarket->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
  mSymbolType = new QComboBox;
  // read all symbol types out of the DB
  QSqlQuery* query = mFilu->execSql("GetAllSymbolTypes");
  if(!check4FiluError("AddFiPage::createPage: ERROR while exec GetAllSymbolTypes.sql"))
  {
    if(query)
    {
      while(query->next())
      {
         mSymbolType->insertItem(0, query->value(0).toString());
      }
    }
  }

  QGridLayout* addEditLineLO = new QGridLayout;
  addEditLineLO->addWidget( new QLabel("RefSymbol"), 0, 0);
  addEditLineLO->addWidget(mAnySymbol, 1, 0);
  addEditLineLO->addWidget( new QLabel("Name"), 0, 1);
  addEditLineLO->addWidget(mName, 1, 1);
  addEditLineLO->addWidget( new QLabel("Type"), 0, 2);
  addEditLineLO->addWidget(mType, 1, 2);
  addEditLineLO->addWidget( new QLabel("Symbol"), 0, 3);
  addEditLineLO->addWidget(mSymbol, 1, 3);
  addEditLineLO->addWidget( new QLabel("Market"), 0, 4);
  addEditLineLO->addWidget(mMarket, 1, 4);
  addEditLineLO->addWidget( new QLabel("Provider"), 0, 5);
  addEditLineLO->addWidget(mSymbolType, 1, 5);
  addEditLineLO->addWidget(mAddBtn, 1, 6);

  mFilu->setMarketName("");
  MarketTuple* markets = mFilu->getMarket();
  if(markets)
  {
    QStringList sl;
    while(markets->next()) sl.append(markets->name());
    sl.sort();
    mMarket->addItems(sl);
    delete markets;
  }
  else qDebug() << "AddFiPage::createPage: no markets found";

  //
  // build the main layout
  QGridLayout* searchLayout = new QGridLayout;
  searchLayout->addWidget(mProviderSelector  , 0, 0);
  searchLayout->addWidget(mSearchField       , 0, 2);
  searchLayout->addWidget(mTypeSelector      , 0, 1);
  searchLayout->addWidget(mSearchCancelBtn   , 0, 3);
  searchLayout->addWidget(mResultList    , 1, 0, 1, 4);
  searchLayout->setColumnStretch(2, 1);
  searchLayout->addLayout(addEditLineLO, 2, 0, 1, 4);

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

  if(mResultKeys.contains("Symbol"))
    mAnySymbol->setText(mResultList->item(row, mResultKeys.value("Symbol"))->text());
  else mAnySymbol->setText("");

  if(mResultKeys.contains("Name"))
    mName->setText(mResultList->item(row, mResultKeys.value("Name"))->text());
  else mName->setText("");

  if(mResultKeys.contains("Type"))
    mType->setText(mResultList->item(row, mResultKeys.value("Type"))->text());
  else if(mDisplayType == "Index") mType->setText("Index");
        else mType->setText("");

  if(mResultKeys.contains("MySymbol"))
    mSymbol->setText(mResultList->item(row, mResultKeys.value("MySymbol"))->text());
  else mSymbol->setText("");

  if(mResultKeys.contains("Market"))
    mMarket->setCurrentIndex(mMarket->findText(mResultList->item(row, mResultKeys.value("Market"))->text()));
  else
    mMarket->setCurrentIndex(mMarket->findText("OTC"));

  mSymbolType->setCurrentIndex(mSymbolType->findText(mProvider));
}

void AddFiPage::search()
{
qDebug() << "AddFiPage::search()";
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
  QStringList parms(mSearchField->text());
  mScripter->showWaitWindow();
  mScripter->askProvider(mProvider, "fetchFi", parms);
}

void AddFiPage::searchIdx()
{
  QStringList parms(mSearchField->text());
  QStringList* result = mScripter->askProvider(mProvider, "fetchIdx", parms);
  if(!result)
  {
    result = new QStringList;
    result->append("HEADER=Error");
    mScripter->getErrorMessage(*result);
  }

  fillResultTable(result);
}

void AddFiPage::fillResultTable(QStringList* data)
{
  if(mNewQuery)
  {
    mNewQuery = false;
    while(mResultList->rowCount()) mResultList->removeRow(0);
    while(mResultList->columnCount()) mResultList->removeColumn(0);

    mResultKeys.clear();
/*
    // create headers
    QStringList headers = data->at(0).split(";");
    for(int i = 0; i < headers.size(); ++i)
    {
      mResultList->insertColumn(i);
      mResultKeys.insert(headers.at(i), i);
    }

    mResultList->setHorizontalHeaderLabels(headers);
    data->removeAt(0);
    if(data->size() == 0) return;
    */
  }

  int r, c, re; // row, column, existing rows
  re = mResultList->rowCount();
  for(r = 0; r < data->size(); ++r)  // rows
  {
    QStringList row = data->at(r).split(";");
    if(row.at(0).startsWith("HEADER="))
    {
      row[0].remove("HEADER=");

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

    for(c = 0; c < row.size(); ++c)     // columns
    {
      if(c > mResultList->columnCount() - 1)
        mResultList->insertColumn(c);

      if(row.at(c).isEmpty()) continue;

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
  }

  mResultList->horizontalHeader()->hide();
  mResultList->resizeColumnsToContents();
  mResultList->horizontalHeader()->show();

  delete data; // no longer needed
  mResultList->update();
}

void AddFiPage::readSettings()
{
  mProviderPath = mRcFile->getST("ProviderPath");
}

void AddFiPage::searchOrCancel()
{
  if(mSearchCancelBtn->hasFocus())
  {
    qDebug() << "searchOrCancel";
    if(mSearchCancelBtn->text() == "Cancel") mScripter->stopRunning();
    else search();
  }
}

void AddFiPage::scriptFinished()
{
  mSearchCancelBtn->setText("Search");
}

void AddFiPage::addToDB()
{
//   if(mDisplayType == "Stock")
//   {
    qDebug() << "add to db";

    FiTuple fi(1);

    SymbolTuple* symbol;

    if(!mAnySymbol->text().isEmpty())
    {
      symbol = new SymbolTuple(2);
      symbol->next();
      symbol->setCaption(mAnySymbol->text());
      symbol->setMarket("");
      symbol->setOwner("");
    }
    else
    {
      symbol = new SymbolTuple(1);
    }

    symbol->next();
    symbol->setCaption(mSymbol->text());
    symbol->setMarket(mMarket->currentText());
    symbol->setOwner(mSymbolType->currentText());

    fi.next(); // set on first position

    fi.setSymbol(symbol);
    fi.setName(mName->text());
    fi.setType(mType->text());

    // here is the beef
    mFilu->addFiCareful(fi);
    if(mFilu->hadTrouble())
    {
      //printError("-addfi");
      qDebug() << "AddFiPage::addToDB: Oops! new FI not added to DB";
      qDebug() << "\tare you sure that FI type, market and symbol type exist?";
    }
//   }
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
    QList<int> msl = mResultKeys.values("MySymbol");
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
        QWidget* w = mResultList->cellWidget(row, psml.at(i - 1));
        if(!w) continue;
        QStringList txt = ((TWIB*)w)->text().split(" ");
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

    fi.next(); // set on first position

    fi.setSymbol(symbol);
    fi.setName(mResultList->item(row, mResultKeys.value("Name"))->text());
    fi.setType(mResultList->item(row, mResultKeys.value("Type"))->text());

    // here is the beef
    mFilu->addFiCareful(fi);

    if(mFilu->hadTrouble())
    {
      //printError("-addfi");
      qDebug() << "agentf -addFi: Oops! new FI not added to DB";
      qDebug() << "\tare you sure that FI type, market and symbol type exist?";
    }
  }
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
