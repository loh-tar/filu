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

#include "PerformerF.h"

#include "SearchFiWidget.h"
#include "COType.h"
#include "IndiWidgetGroup.h"
#include "FiGroupWidget.h"
#include "LaunchPad.h"
#include "FToolBar.h"
#include "IndiWidgetSimple.h"
#include "IndiSetPad.h"

PerformerF::PerformerF(QApplication& app)
          : FMainApp("PerformerF", app)
{
  mWindowTitel = tr("PerformerF - The Filu Chart Tool");
  setWindowTitle(mWindowTitel);

  // Some needs
  QPixmap       icon(10, 10);
  FToolBar*     tb;
  QDockWidget*  dw;
  QAction*      act;

  //
  // Create the Main Tool Bar
  tb = new FToolBar("Main Tool Bar", this);
  addToolBar(tb);
  tb->setObjectName("MainToolBar");
  tb->setToolButtonStyle(Qt::ToolButtonIconOnly);
  tb->setIconSize(QSize(10, 10));

  //
  // Create the Beef-Widget withn all action-connections
  mIndiGroup = new IndiWidgetGroup(this);
  setCentralWidget(mIndiGroup);

  //
  // Create the FI Search Dock
  SearchFiWidget* searchFi = new SearchFiWidget(this);
  connect(searchFi, SIGNAL(selected(const QString &, const QString &))
        , this, SLOT(showWindowTitle(const QString &, const QString &)));
  connect(searchFi, SIGNAL(selected(int, int))
        , this, SLOT(loadData(int, int)));

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

  addDockWidget(Qt::LeftDockWidgetArea, dw);

  //
  // Create the FI Group Dock
  mGroupNavi = new FiGroupWidget(this);
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

  //
  // Create the FI Group Dock 2
  mGroupNavi2 = new FiGroupWidget(this);
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
  icon.fill(Qt::cyan);
  act->setIcon(icon);
  tb->addAction(act);

  addDockWidget(Qt::RightDockWidgetArea, dw);

  //
  // Create the Zoom Dock
  mZoomWidget = new IndiWidgetSimple("ZoomWidget", this);
  dw = new QDockWidget(tr("Zoom View"), this);
  dw->setObjectName("ZoomView");
  dw->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  dw->setWidget(mZoomWidget);
  act = dw->toggleViewAction();
  act->setObjectName("Act" + dw->objectName());
  icon.fill(Qt::black);
  act->setIcon(icon);
  tb->addAction(act);

  addDockWidget(Qt::LeftDockWidgetArea, dw);

  //
  // Create the LaunchPad with an own tool bar
  tb = new FToolBar("LaunchPad", this);
  addToolBar(tb);
  tb->setObjectName("LPToolBar");

  mLaunchPad = new LaunchPad("LaunchPad", this);
  mLaunchPad->loadSettings();
  mLaunchPad->addToToolBar(tb);

  //
  // Create the IndiSetPad with an own tool bar
  tb = new FToolBar("IndiSetPad", this);
  addToolBar(tb);
  tb->setObjectName("ISToolBar");

  IndiSetPad* isp = new IndiSetPad("PerformerIndiSetPad", this);
  isp->loadSettings();
  isp->addToToolBar(tb);

  connect(isp, SIGNAL(setupChosen(const QString&)),
          mIndiGroup, SLOT(loadSetup(const QString&)));

  //
  // Create the chart object buttons with an own tool bar
  tb = new FToolBar("Chart Objects", this);
  addToolBar(tb);
  tb->setObjectName("COToolBar");

  QStringList coTypes;
  COType::getAllTypes(coTypes);
  QActionGroup* actGrp = new QActionGroup(tb);

  connect(actGrp, SIGNAL(triggered(QAction *)),
          mIndiGroup, SLOT(chartObjectChosen(QAction *)));

  act = new QAction(actGrp);
  act->setIconText("X");
  act->setObjectName("Act" + act->text());
  act->setData(""); // Set to "no type"
  act->setCheckable(true);
  act->setChecked(true);
  foreach(QString type, coTypes)
  {
    act = new QAction(actGrp);
    act->setIconText(type);
    act->setObjectName("Act" + act->text());
    act->setData(type);
    act->setCheckable(true);
  }

  tb->addActions(actGrp->actions());

  //
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

  QString indiSet = mRcFile->getST("PerformerIndiSet");

  mRcFile->endGroup(); // "Performer"

  mIndiGroup->loadSetup(indiSet);  // Must done after mRcFile->endGroup(); // "Performer"
  isp->setCurrentSetup(indiSet);

//   FIXME: Doesn't work, see doc/todo.txt
//   // Set a short cut to bring up the toolbars/dock widget menu
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

  mRcFile->set("PerformerIndiSet", mIndiGroup->indiSetName());

  mRcFile->endGroup(); // "Performer"
}

void PerformerF::loadData(int fiId, int marketId)
{
  // FIXME: Add edit fields for dates to performers toolbar
  //        and read the dates out there

  //mFilu->setBarsToLoad(400);
  //mFilu->setFromDate("1900-01-01");
  //mFilu->setToDate(QDate::currentDate().toString(Qt::ISODate));

  BarTuple* bars = mFilu->getBars(fiId, marketId, "1900-01-01"
                                , QDate::currentDate().toString(Qt::ISODate));

  mIndiGroup->showBarData(bars);     // Will eaten "bars", no need to delete them here
  mZoomWidget->showBarData(bars);

  mLaunchPad->newSelection(fiId, marketId);
}

void PerformerF::showWindowTitle(const QString& symbol, const QString& market)
{
  FiTuple* fi = mFilu->getFiBySymbol(symbol);
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
    name = fi->name();
  }

  if(fi) delete fi;

  setWindowTitle(name + " (" + symbol + " " + market + ") - " + mWindowTitel);
}
