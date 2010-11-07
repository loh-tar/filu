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

#include "ManagerF.h"
//#include "ConfigPage.h"
#include "FiPage.h"
#include "IndicatorPage.h"
#include "AddFiPage.h"
#include "LogBookPage.h"

ManagerF::ManagerF(const QString connectionName/* = "ManagerF"*/)
         : QDialog(), FClass(connectionName)
{
  mPageIcons = new QListWidget;
  mPageIcons->setViewMode(QListView::IconMode);
  mPageIcons->setIconSize(QSize(96, 84));
  mPageIcons->setMovement(QListView::Static);
  mPageIcons->setMaximumWidth(128);
  mPageIcons->setSpacing(12);


  mPageStack = new QStackedWidget;
  mPageStack->addWidget(new FiPage(this));
  mPageStack->addWidget(new AddFiPage(this));
  //mPageStack->addWidget(new ConfigPage(this));
  mPageStack->addWidget(new IndicatorPage(this));
// mPageStack->addWidget(new ...);

  // special treatment may to FIXME
  mLogBookPage = new LogBookPage(this);
  mPageStack->addWidget(mLogBookPage);

  QPushButton* closeButton = new QPushButton(tr("Close"));
  createIcons();
  mPageIcons->setCurrentRow(0);

  connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

  QHBoxLayout* horizontalLayout = new QHBoxLayout;
  horizontalLayout->addWidget(mPageIcons);
  horizontalLayout->addWidget(mPageStack, 1);

  mMessage = new QLabel;
  mMessage->setFrameStyle(QFrame::Panel | QFrame::Sunken);

  QHBoxLayout* buttonsLayout = new QHBoxLayout;
  buttonsLayout->addWidget(mMessage);
  buttonsLayout->setStretch(0, 5);
  buttonsLayout->addWidget(closeButton);
  buttonsLayout->setStretch(1, 1);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addLayout(horizontalLayout);
  mainLayout->addLayout(buttonsLayout);
  setLayout(mainLayout);

  setWindowTitle(tr("ManagerF - The Filu Manager"));
 }

ManagerF::~ManagerF()
{
  mRcFile->beginGroup("Manager");

  for(int i = 0; i < mPageStack->count(); i++)
  {
    ManagerPage* mp = static_cast<ManagerPage*>(mPageStack->widget(i));
    mp->saveSettings();
  }

  mRcFile->endGroup();
}

void ManagerF::createIcons()
{
  mRcFile->beginGroup("Manager");

  for(int i = 0; i < mPageStack->count(); i++)
  {
    QListWidgetItem* icon = new QListWidgetItem(mPageIcons);
    ManagerPage* mp = static_cast<ManagerPage*>(mPageStack->widget(i));
    mp->setPageIcon(icon);
    mp->loadSettings();
    connect(mp, SIGNAL(message(const QString&, const bool)), this, SLOT(messageBox(const QString&, const bool)));
  }

  mRcFile->endGroup();

  connect(mPageIcons,
    SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
    this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));
}

void ManagerF::changePage(QListWidgetItem* current, QListWidgetItem* previous)
{
  if (!current) current = previous;

  mPageStack->setCurrentIndex(mPageIcons->row(current));
}

void ManagerF::messageBox(const QString& msg, const bool error/* = false*/)
{
  // FIXME: Work not as desired, see ManagerPage.h
  if(error)
    mMessage->setBackgroundRole(QPalette::Highlight);
  else
    mMessage->setBackgroundRole(QPalette::NoRole);

  mMessage->setText(msg);
  mLogBookPage->addToLog(msg, error);
}
