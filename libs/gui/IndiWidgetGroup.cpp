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

#include "IndiWidgetGroup.h"

#include "IndicatorWidget.h"
#include "PlotSheet.h"

IndiWidgetGroup::IndiWidgetGroup(FClass* parent)
               : FWidget(parent)
               , mSetName("")
               , mBars(0)
{
  init();
}

IndiWidgetGroup::IndiWidgetGroup(const QString& name, FClass* parent)
               : FWidget(parent)
               , mSetName(name)
               , mBars(0)
{
  init();
}

IndiWidgetGroup::~IndiWidgetGroup()
{
  saveSetup();
  delete mSplitter;
  if(mBars) delete mBars;
}

void IndiWidgetGroup::init()
{
  // Create the context menu actions
  QMenu* menu = new QMenu(this);
  connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(changeWindowCount(QAction*)));
  QAction* act;
  act = menu->addAction("+1");
  act->setData(1);
  act = menu->addAction("+2");
  act->setData(2);
  act = menu->addAction("+3");
  act->setData(3);
  act = new QAction(tr("More Indicators..."), this);
  act->setMenu(menu);
  addAction(act);

  menu = new QMenu(this);
  connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(changeWindowCount(QAction*)));
  act = menu->addAction("-1");
  act->setData(-1);
  act = menu->addAction("-2");
  act->setData(-2);
  act = menu->addAction("-3");
  act->setData(-3);
  act = new QAction(tr("Less Indicators..."), this);
  act->setMenu(menu);
  addAction(act);


  mFullIndiSetsPath = mRcFile->getST("IndiSetsPath");
  mSplitter = new QSplitter(Qt::Vertical);

  loadSetup(mSetName);

  QVBoxLayout* lay = new QVBoxLayout;
  lay->addWidget(mSplitter);

  setLayout(lay);
}

void IndiWidgetGroup::addWindow()
{
    IndicatorWidget* indi = new IndicatorWidget(mSetName, mSplitter->count(), this);
    indi->showBarData(mBars);
    indi->insertActions(indi->actions().at(0), actions());

    connect(indi, SIGNAL(newSize(QList<int>*)), this, SLOT(childSplitterMoved(QList<int>*)));
    connect(indi, SIGNAL(mouse(MyMouseEvent*)), this, SLOT(mouseSlot(MyMouseEvent*)));

    mSplitter->addWidget(indi);

    indi->sync();
}

void IndiWidgetGroup::removeWindow()
{
  IndicatorWidget* indi = static_cast<IndicatorWidget*>(mSplitter->widget(mSplitter->count() - 1));
  delete indi;
}

void IndiWidgetGroup::loadSetup(const QString& setup)
{
  if(setup.isEmpty()) return;
  if(setup != mSetName) saveSetup();

  mSetName = setup;

  int rowCount;
  QSettings settings(mFullIndiSetsPath + mSetName,  QSettings::IniFormat);
  rowCount = settings.value("IndicatorCount", 1).toInt();

  for(int i = 0; (i < mSplitter->count()) and (i < rowCount); ++i)
  {
    static_cast<IndicatorWidget*>(mSplitter->widget(i))->loadSetup(mSetName, i);
  }

  while(mSplitter->count() < rowCount) addWindow();
  while(mSplitter->count() > rowCount) removeWindow();

  mSplitter->restoreState(settings.value("GroupSplitter").toByteArray());
}

void IndiWidgetGroup::showBarData(BarTuple* bars)
{
  if(mBars) delete mBars;

  mBars = bars;

  for(int i = 0; i < mSplitter->count(); ++i)
  {
    IndicatorWidget* indi = static_cast<IndicatorWidget*>(mSplitter->widget(i));
    indi->showBarData(bars);
  }
}

void IndiWidgetGroup::saveSetup()
{
  if(mSetName.isEmpty()) return;

  QSettings settings(mFullIndiSetsPath + mSetName,  QSettings::IniFormat);
  settings.setValue("GroupSplitter", mSplitter->saveState());
  settings.setValue("IndicatorCount", mSplitter->count());
}

/***********************************************************************
*
*
*
************************************************************************/
void IndiWidgetGroup::childSplitterMoved(QList<int>* size)
{
  for(int i = 0; i < mSplitter->count(); ++i)
  {
    IndicatorWidget* indi = static_cast<IndicatorWidget*>(mSplitter->widget(i));
    indi->setSize(*size);
  }
}

void IndiWidgetGroup::mouseSlot(MyMouseEvent* event)
{
  for(int i = 0; i < mSplitter->count(); ++i)
  {
    IndicatorWidget* indi = static_cast<IndicatorWidget*>(mSplitter->widget(i));
    indi->mouseSlot(event);
  }
}

void IndiWidgetGroup::chartObjectChosen(QAction* action)
{
  QString type = action->data().toString();

  for(int i = 0; i < mSplitter->count(); ++i)
  {
    IndicatorWidget* indi = static_cast<IndicatorWidget*>(mSplitter->widget(i));
    indi->chartObjectChosen(type);
  }
}

void IndiWidgetGroup::changeWindowCount(QAction* action) // Slot
{
  int count = action->data().toInt();

  if(count > 0)
  {
    for(int i = 0; i < count; ++i) addWindow();
  }
  else
  {
    for(int i = 0; i > count; --i)
    {
      if(1 == mSplitter->count()) break; // Don't remove the last IndicatorWindow
      removeWindow();
    }
  }
}
