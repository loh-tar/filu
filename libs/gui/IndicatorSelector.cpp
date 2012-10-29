//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012  loh.tar@googlemail.com
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

#include "IndicatorSelector.h"

#include "SearchField.h"

IndicatorSelector::IndicatorSelector(FClass* parent)
                 : FWidget(parent, FUNC)
                 , mTALib(this)
{
  createPage();
}

IndicatorSelector::~IndicatorSelector()
{}

void IndicatorSelector::loadSettings()
{
  mRcFile->beginGroup("IndicatorSelector");
  mSplitter->restoreState(mRcFile->value("Splitter").toByteArray());
  mRcFile->endGroup();
}

void IndicatorSelector::saveSettings()
{
  mRcFile->beginGroup("IndicatorSelector");
  mRcFile->setValue("Splitter", mSplitter->saveState());
  mRcFile->endGroup();
}

void IndicatorSelector::createPage()
{
  mSF = new SearchField(this);
  connect(mSF, SIGNAL(textChanged()), this, SLOT(refreshFunctionList()));

  mTALib.getAllFunctionNames(mAllFunctions);
  mFunctionList = new QListWidget;
  connect(mFunctionList, SIGNAL(itemClicked(QListWidgetItem *)),
          this, SLOT(getFunctionInfo(QListWidgetItem *)));
  mFunctionList->addItems(mAllFunctions);

  mInfoList = new QListWidget;
  connect(mInfoList, SIGNAL(currentRowChanged(int)),
          this, SLOT(checkInfoSelection(int)));

  mSplitter = new QSplitter(Qt::Vertical);
  mSplitter->addWidget(mFunctionList);
  mSplitter->addWidget(mInfoList);
  mSplitter->setStretchFactor(0, 1);  // The mFunctionList
  mSplitter->setStretchFactor(1, 2);  // The mInfoList

  mAddBtn = new QPushButton;
  connect(mAddBtn, SIGNAL(clicked()), this, SLOT(addToIndicator()));
  mAddBtn->setText(tr("Add"));
  mAddBtn->setDisabled(true);

  mInclBtn = new QPushButton;
  connect(mInclBtn, SIGNAL(clicked()), this, SLOT(inclToIndicator()));
  mInclBtn->setText(tr("Include"));
  mInclBtn->setDisabled(true);

  QHBoxLayout* addBtnLayout = new QHBoxLayout;
  addBtnLayout->addWidget(mAddBtn);
  addBtnLayout->addWidget(mInclBtn);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setMargin(0);
  mainLayout->addWidget(mSF);
  mainLayout->addWidget(mSplitter);
  mainLayout->addLayout(addBtnLayout);
  mainLayout->setStretchFactor(mSplitter, 1);

  setLayout(mainLayout);
}

void IndicatorSelector::showEvent(QShowEvent * /*event*/)
{
  mSF->setFocus();
}

void IndicatorSelector::refreshFunctionList()
{
  mFunctionList->clear();
  QStringList list = mAllFunctions.filter(mSF->filter());
  mFunctionList->addItems(list);
  mFunctionList->scrollToItem(mInfoList->item(0));
}

void IndicatorSelector::getFunctionInfo(QListWidgetItem* item)
{
  QString function = item->text();
  FunctionInfo info;
  mTALib.getFunctionInfo(function, info);
  QStringList list;
  QString key, txt;
  int i;

  list.append("Name:\t" + info.value("LongName").toString());
  list.append("Group:\t" + info.value("Group").toString());
  list.append("Parms:\t" + info.value("Input").toString());
  list.append("OptParms:\t" + info.value("OptInput").toString());
  for(i = 0; i < info.value("nbOptInput").toInt(); ++i)
  {
    key = "OptInParm " + QString::number(i + 1);
    txt = "OptParm" + QString::number(i + 1);
    list.append(txt + ":\t" + info.value(key).toString());
  }

  list.append("");

  i = 1;
  txt = "Call";
  key = txt + QString::number(i);
  while(info.contains(key))
  {
    list.append(key + ":\t" + info.value(key).toString());
    key = txt + QString::number(++i);
  }

  i = 1;
  txt = "Include";
  key = txt + QString::number(i);
  while(info.contains(key))
  {
    list.append(key + ":\t" + info.value(key).toString());
    key = txt + QString::number(++i);
  }

  for(i = 0; i < info.value("nbOutput").toInt(); ++i)
  {
    key = "OutName" + QString::number(i + 1);
    txt = key + ":\t" + info.value(key).toString();
    key = "PlotType" + QString::number(i + 1);
    txt.append(" [" + info.value(key).toString() + "]");
    list.append(txt);

    key = "OutHint" + QString::number(i + 1);
    if(info.contains(key)) list.append(key + ":\t" + info.value(key).toString());
  }

/*
  // Testing only
  list.append("");
  FunctionInfoIterator fit(info);
  while (fit.hasNext())
  {
    fit.next();
    list.append(fit.key() + ": " + fit.value().toString());
  }
*/

  mInfoList->clear();
  mInfoList->addItems(list);
  mInfoList->scrollToItem(mInfoList->item(0));
}

void IndicatorSelector::checkInfoSelection(int r)
{
  if(r < 0)
  {
    mAddBtn->setDisabled(true);
    mInclBtn->setDisabled(true);
    return;
  }

  QString txt = mInfoList->currentItem()->text();
  if(txt.contains("INCLTALIB("))
  {
    mAddBtn->setDisabled(false);
    mInclBtn->setDisabled(false);
    return;
  }
  else
  {
    mAddBtn->setDisabled(true);
    mInclBtn->setDisabled(true);
  }

  if(txt.contains("TALIB("))   mAddBtn->setDisabled(false);
  else mAddBtn->setDisabled(true);
}

void IndicatorSelector::addToIndicator()
{
  QString txt = mInfoList->currentItem()->text();
  txt.remove(QRegExp(".*\t"));
  txt.append("\n");

  emit addText(&txt);
}

void IndicatorSelector::inclToIndicator()
{

}
