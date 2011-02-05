//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011  loh.tar@googlemail.com
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

#include "IndicatorPage.h"
#include "IndicatorEditor.h"
#include "IndicatorSelector.h"

IndicatorPage::IndicatorPage(FClass* parent)
             : ManagerPage(parent, FUNC)
{
  createPage();
}

IndicatorPage::~IndicatorPage()
{}

void IndicatorPage::loadSettings()
{
  mRcFile->beginGroup("IndicatorPage");
  mSplitter->restoreState(mRcFile->value("Splitter").toByteArray());
  mEditor->loadSettings();
  mIndiSelector->loadSettings();
  mRcFile->endGroup();
}

void IndicatorPage::saveSettings()
{
  mRcFile->beginGroup("IndicatorPage");
  mRcFile->setValue("Splitter", mSplitter->saveState());
  mEditor->saveSettings();
  mIndiSelector->saveSettings();
  mRcFile->endGroup();
}

void IndicatorPage::createPage()
{
  mIcon = QIcon(":/icons/indicator.png");
  mIconText = tr("Indicators");

  QGroupBox* pageGroup = new QGroupBox(tr("Indicators"));

  mEditor = new IndicatorEditor(this);

  mIndiSelector = new IndicatorSelector(this);
  connect(mIndiSelector, SIGNAL(addText(const QString *))
         , this, SLOT(addToIndicator(const QString *)));

  mSplitter = new QSplitter(Qt::Horizontal);
  mSplitter->addWidget(mEditor);
  mSplitter->addWidget(mIndiSelector);

  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(mSplitter);

  pageGroup->setLayout(layout);

  QHBoxLayout* dummyLayout = new QHBoxLayout;
  dummyLayout->addWidget(pageGroup);

  setLayout(dummyLayout);
}

//void IndicatorPage::showEvent(QShowEvent * /*event*/)
/*{
  mSF->setFocus();
}
*/

void IndicatorPage::addToIndicator(const QString* txt)
{
  mEditor->includeText(*txt);
}
