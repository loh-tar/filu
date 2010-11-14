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

#include "PerformerF.h"

#include "SearchFiWidget.h"
#include "COType.h"
#include "IndiWidgetGroup.h"
#include "FiGroupWidget.h"
#include "LaunchPad.h"
#include "FToolBar.h"

PerformerF::PerformerF()
          : FMainWindow("PerformerF")
{
  mWindowTitel = tr("PerformerF - The Filu Chart Tool");
  setWindowTitle(mWindowTitel);

  // Some needs
  QPixmap       icon(10, 10);
  FToolBar*     tb;
  QDockWidget*  dw;
  QAction*      act;

  // Create the main tool bar
  tb = new FToolBar("Main Tool Bar", this);
  addToolBar(tb);
  tb->setObjectName("MainToolBar");
  tb->setToolButtonStyle(Qt::ToolButtonIconOnly);
  tb->setIconSize(QSize(10, 10));

  // Create the beef-widget withn all action-connections
  mIndiGroup = new IndiWidgetGroup((FWidget*)this);
  setCentralWidget(mIndiGroup);


  SearchFiWidget* searchFi = new SearchFiWidget(this);
  connect(searchFi, SIGNAL(selected(const QString &, const QString &))
        , this, SLOT(showWindowTitle(const QString &, const QString &)));
  connect(searchFi, SIGNAL(selected(int, int))
        , this, SLOT(loadData(int, int)));

  // Create the FI search dock
  dw = new QDockWidget(tr("Search FI"), this);
  dw->setObjectName("SearchFI");
  dw->setAllowedAreas(Qt::LeftDockWidgetArea |
                              Qt::RightDockWidgetArea);
  dw->setWidget(searchFi);
  act = dw->toggleViewAction();
  act->setObjectName("Act" + dw->objectName());
  icon.fill(Qt::red);
  act->setIcon(icon);
  tb->addAction(act);

  addDockWidget(Qt::RightDockWidgetArea, dw);

  // Create the FI group dock
  mGroupNavi = new FiGroupWidget((FWidget*)this);
  connect(mGroupNavi, SIGNAL(selected(const QString &, const QString &))
        , this, SLOT(showWindowTitle(const QString &, const QString &)));
  connect(mGroupNavi, SIGNAL(selected(int, int))
        , this, SLOT(loadData(int, int)));

  dw = new QDockWidget(tr("Group Navi"), this);
  dw->setObjectName("GroupNavi");
  dw->setAllowedAreas(Qt::LeftDockWidgetArea |
                              Qt::RightDockWidgetArea);
  dw->setWidget(mGroupNavi);
  act = dw->toggleViewAction();
  act->setObjectName("Act" + dw->objectName());
  icon.fill(Qt::blue);
  act->setIcon(icon);
  tb->addAction(act);

  addDockWidget(Qt::RightDockWidgetArea, dw);

  // Create the FI group dock 2
  mGroupNavi2 = new FiGroupWidget((FWidget*)this);
  connect(mGroupNavi2, SIGNAL(selected(const QString &, const QString &))
        , this, SLOT(showWindowTitle(const QString &, const QString &)));
  connect(mGroupNavi2, SIGNAL(selected(int, int))
        , this, SLOT(loadData(int, int)));

  dw = new QDockWidget(tr("Group Navi 2"), this);
  dw->setObjectName("GroupNavi2");
  dw->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  dw->setWidget(mGroupNavi2);
  act = dw->toggleViewAction();
  act->setObjectName("Act" + dw->objectName());
  icon.fill(Qt::darkBlue);
  act->setIcon(icon);
  tb->addAction(act);

  addDockWidget(Qt::RightDockWidgetArea, dw);

  // Create the LaunchPad with an own tool bar
  tb = new FToolBar("LaunchPad", this);
  addToolBar(tb);
  tb->setObjectName("LPToolBar");
  //tb->setToolButtonStyle(Qt::ToolButtonIconOnly);
  //tb->setIconSize(QSize(10, 10));

  mLaunchPad = new LaunchPad("LaunchPad", this);
  tb->addWidget(mLaunchPad);
  connect(tb, SIGNAL(orientationChanged(Qt::Orientation)),
          mLaunchPad, SLOT(orientationChanged(Qt::Orientation)));

  // Create the chart object buttons with an own tool bar
  tb = new FToolBar("Chart Objects", this);
  addToolBar(tb);
  tb->setObjectName("COToolBar");
  //tb->setToolButtonStyle(Qt::ToolButtonIconOnly);
  //tb->setIconSize(QSize(10, 10));

  QStringList coTypes;
  COType::getAllTypes(coTypes);
  QActionGroup* actGrp = new QActionGroup(tb);

  connect(actGrp, SIGNAL(triggered(QAction *)),
          mIndiGroup, SLOT(chartObjectChosen(QAction *)));

  act = new QAction(actGrp);
  act->setIconText("X");
  act->setObjectName("Act" + act->text());
  act->setData(""); // set to "no type"
  act->setCheckable(true);
  act->setChecked(true);
  QString type;
  foreach(type, coTypes)
  {
    act = new QAction(actGrp);
    act->setIconText(type);
    act->setObjectName("Act" + act->text());
    act->setData(type);
    act->setCheckable(true);
  }

  tb->addActions(actGrp->actions());

  // Restore all settings
  mRcFile->beginGroup("Performer");
  QSize size;
  size = mRcFile->getSZ("PerformerSize");
  resize(size);
  QPoint pos;
  pos = mRcFile->getPT("PerformerPosition");
  move(pos);
  QByteArray ba;
  ba = mRcFile->getBA("PerformerState");
  restoreState(ba);

  mRcFile->beginGroup("GroupNavi");
  mGroupNavi->loadSettings();
  mRcFile->endGroup();

  mRcFile->beginGroup("GroupNavi2");
  mGroupNavi2->loadSettings();
  mRcFile->endGroup();

  foreach(QObject* child, children())
  {
    if(child->inherits("FToolBar")) static_cast<FToolBar*>(child)->loadSettings();
  }

  mRcFile->endGroup(); // "Performer"

  mLaunchPad->loadSettings();

//   FIXME: Doesn't work, see doc/todo.txt
//   // Set a short cut to bring up the toolbars/dock widget menue
//   // which appears by right click on a toolbar/docwidget
//   act = createPopupMenu()->menuAction();
//   act->setShortcut(QKeySequence(QKeySequence::HelpContents));
//   act->setShortcutContext(Qt::ApplicationShortcut);
}

PerformerF::~PerformerF()
{
  mRcFile->beginGroup("Performer");
  mRcFile->set("PerformerSize", size());
  mRcFile->set("PerformerPosition", pos());
  mRcFile->set("PerformerState", saveState());

  mRcFile->beginGroup("GroupNavi");
  mGroupNavi->saveSettings();
  mRcFile->endGroup();

  mRcFile->beginGroup("GroupNavi2");
  mGroupNavi2->saveSettings();
  mRcFile->endGroup();

  foreach(QObject* child, children())
  {
    if(child->inherits("FToolBar")) static_cast<FToolBar*>(child)->saveSettings();
  }

  mRcFile->endGroup(); // "Performer"

  mLaunchPad->saveSettings();
}

void PerformerF::loadData(int fiId, int marketId)
{
  // FIXME: Add edit fields for dates to performers toolbar
  //        and read the dates out there

  //mFilu->setBarsToLoad(400);
  //mFilu->setFromDate("1900-01-01");
  //mFilu->setToDate(QDate::currentDate().toString(Qt::ISODate));

  mIndiGroup->showBarData(mFilu->getBars(fiId,
                                         marketId,
                                         "1900-01-01",
                                         QDate::currentDate().toString(Qt::ISODate)));

  mLaunchPad->newSelection(fiId, marketId);
}

void PerformerF::showWindowTitle(const QString& symbol, const QString& market)
{
  FiTuple* fi = mFilu->getFi(symbol);
  QString name;
  if(!fi)
  {
    name = tr("NOT FOUND!");
  }
  else if(fi->count() > 1)
  {
    name = tr("MORE THAN ONE MATCHES!");
  }
  else
  {
    fi->next();
    name = fi->name();
  }

  if(fi) delete fi;

  setWindowTitle(name + " (" + symbol + " " + market + ") - " + mWindowTitel);
}
