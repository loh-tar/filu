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

#include "IndicatorPage.h"
#include "IndicatorEditor.h"
#include "IndicatorSelector.h"

IndicatorPage::IndicatorPage(ManagerF* parent) : ManagerPage(parent)
{
  mIcon = QIcon(":/icons/configure.xpm");
  mIconText = tr("Indicators");
  createPage();
}

IndicatorPage::IndicatorPage(FWidget* parent) : ManagerPage(parent)
{
  mIcon = QIcon(":/icons/configure.xpm");
  mIconText = tr("Indicators");
  createPage();
}

IndicatorPage::~IndicatorPage()
{}

void IndicatorPage::createPage()
{
  QGroupBox* pageGroup = new QGroupBox(tr("Indicators"));

  mEditor = new IndicatorEditor((FWidget*)this);

  mIndiSelector = new IndicatorSelector;
  connect(mIndiSelector, SIGNAL(addText(const QString *))
         , this, SLOT(addToIndicator(const QString *)));

  QSplitter* split = new QSplitter(Qt::Horizontal);
  split->addWidget(mEditor);
  split->addWidget(mIndiSelector);

  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(split);

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
