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

#include "ManagerF.h"

//#include "ConfigPage.h"
#include "FiPage.h"
#include "IndicatorPage.h"
#include "AddFiPage.h"
#include "LogBookPage.h"

ManagerF::ManagerF(QApplication& app)
        : FMainApp("ManagerF", app)
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
  //mPageStack->addWidget(new ...);

  // Special treatment may to FIXME
  mLogBookPage = new LogBookPage(this);
  mPageStack->addWidget(mLogBookPage);

  QPushButton* closeButton = new QPushButton(tr("Close"));
  connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

  QHBoxLayout* horizontalLayout = new QHBoxLayout;
  horizontalLayout->addWidget(mPageIcons);
  horizontalLayout->addWidget(mPageStack, 1);

  mMsgLabel = new MsgLabel;
  mMsgLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

  createIcons();

  QHBoxLayout* buttonsLayout = new QHBoxLayout;
  buttonsLayout->addWidget(mMsgLabel);
  buttonsLayout->setStretch(0, 5);
  buttonsLayout->addWidget(closeButton);
  buttonsLayout->setStretch(1, 1);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addLayout(horizontalLayout);
  mainLayout->addLayout(buttonsLayout);

  QWidget* dummy = new QWidget;
  dummy->setLayout(mainLayout);
  setCentralWidget(dummy);

  setWindowTitle(tr("ManagerF - The Filu Manager"));

  //
  // Restore all settings
  mRcFile->beginGroup("Manager");

  resize(mRcFile->getSZ("ManagerSize"));
  move(mRcFile->getPT("ManagerPosition"));
  restoreState(mRcFile->getBA("ManagerState"));
  mPageIcons->setCurrentRow(mRcFile->getIT("CurrentPage"));

  for(int i = 0; i < mPageStack->count(); i++)
  {
    ManagerPage* mp = static_cast<ManagerPage*>(mPageStack->widget(i));
    mp->loadSettings();
  }
  mRcFile->endGroup(); // "Manager"
}

ManagerF::~ManagerF()
{
  mRcFile->beginGroup("Manager");
  mRcFile->set("ManagerSize", size());
  mRcFile->set("ManagerPosition", pos());
  mRcFile->set("ManagerState", saveState());
  mRcFile->set("CurrentPage", mPageIcons->currentRow());

  for(int i = 0; i < mPageStack->count(); i++)
  {
    ManagerPage* mp = static_cast<ManagerPage*>(mPageStack->widget(i));
    mp->saveSettings();
  }

  mRcFile->endGroup();
}

void ManagerF::createIcons()
{
  for(int i = 0; i < mPageStack->count(); i++)
  {
    QListWidgetItem* icon = new QListWidgetItem(mPageIcons);
    ManagerPage* mp = static_cast<ManagerPage*>(mPageStack->widget(i));
    mp->setPageIcon(icon);

    // Fetch errors happens while page construction
    QString msg = mp->formatMessages();
    if(!msg.isEmpty())
    {
      msg.prepend(mp->iconText() + " Page: ");
      mMsgLabel->setMessage(msg, eWarning);
      mLogBookPage->addToLog(msg, eWarning);
    }

    connect(mp, SIGNAL(message(const Message&))
          , this, SLOT(messageBox(const Message&)));
  }

  connect(mPageIcons, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *))
        , this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));
}

void ManagerF::changePage(QListWidgetItem* current, QListWidgetItem* previous)
{
  if(!current) current = previous;

  mPageStack->setCurrentIndex(mPageIcons->row(current));
}

void ManagerF::messageBox(const Message& message)
{
  QString msg = formatMessage(message, "%F %x");

  mMsgLabel->setMessage(msg, message.type);
  mLogBookPage->addToLog(msg, message.type);
}

MsgLabel::MsgLabel()
        : QLabel()
        , mLastMsgType(Newswire::eInfoMsg)
{
  connect(&mRolex, SIGNAL(timeout()), this, SLOT(resetMessage()));
}

void MsgLabel::setMessage(const QString& msg, const Newswire::MsgType type/* = eInfoMsg*/)
{
  // Don't overwirte an important message with a less important one
  if(type < mLastMsgType) return;
  mLastMsgType = type;

  if(type > Newswire::eInfoMsg) setStyleSheet("background: yellow");

  setText(msg);
  repaint();    // Force an update as soon as possible

  mRolex.start(9000);
}

void MsgLabel::resetMessage()
{
  mLastMsgType = Newswire::eInfoMsg;
  setStyleSheet("");
  setText("");
}
