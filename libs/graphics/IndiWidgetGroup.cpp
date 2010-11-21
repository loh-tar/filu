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

IndiWidgetGroup::IndiWidgetGroup(FWidget* parent)
               : FWidget(parent)
               , mSetName("")
               , mBars(0)
{
  init();
}

IndiWidgetGroup::IndiWidgetGroup(const QString& name, FWidget* parent)
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

    connect(indi, SIGNAL(newSize(QList<int>*)), this, SLOT(childSplitterMoved(QList<int>*)));
    connect(indi, SIGNAL(mouse(MyMouseEvent*)), this, SLOT(mouseSlot(MyMouseEvent*)));

    mSplitter->addWidget(indi);
}

void IndiWidgetGroup::removeWindow()
{
  IndicatorWidget* indi = (IndicatorWidget*)mSplitter->widget(mSplitter->count() - 1);
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
    IndicatorWidget* indi = (IndicatorWidget*)mSplitter->widget(i);
    indi->showBarData(bars);
  }
}

void IndiWidgetGroup::saveSetup()
{
  if(mSetName.isEmpty()) return;

  QSettings settings(mFullIndiSetsPath + mSetName,  QSettings::IniFormat);
  settings.setValue("GroupSplitter", mSplitter->saveState());
  //settings.setValue("IndicatorCount", mSplitter->count());
}

/***********************************************************************
*
*
*
************************************************************************/
void IndiWidgetGroup::childSplitterMoved(QList<int> *size)
{
  for(int i = 0; i < mSplitter->count(); ++i)
  {
    IndicatorWidget* indi = (IndicatorWidget*)mSplitter->widget(i);
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
