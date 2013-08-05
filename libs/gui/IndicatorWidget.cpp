//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
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

#include <QHBoxLayout>
#include <QSplitter>

#include "IndicatorWidget.h"

#include "IndicatorDataView.h"
#include "IndicatorPicker.h"
#include "PlotSheet.h"
#include "SettingsFile.h"

IndicatorWidget::IndicatorWidget(const QString& name, FWidget* parent)
               : IndiWidgetSimple(name, parent, FUNC)
{
  init();
}

IndicatorWidget::IndicatorWidget(const QString& name, int number, FWidget* parent)
               : IndiWidgetSimple(name, number, parent, FUNC)
{
  init();
}

IndicatorWidget::~IndicatorWidget()
{
  saveSettings();
}

void IndicatorWidget::init()
{
  QList<int> sizes;

  mPicker = new IndicatorPicker(this);
  connect(mPicker, SIGNAL(changed(QString))
        , this, SLOT(useIndicator(const QString &)));
  sizes.append(mPicker->minimumSizeHint().width());

  sizes.append(mSheet->sizeHint().width()*2);

  mDataView = new IndicatorDataView(mSheet);
  connect(mSheet, SIGNAL(mouse(MyMouseEvent*))
        , mDataView, SLOT(mouseSlot(MyMouseEvent*)));


  mSplitter = new QSplitter(Qt::Horizontal);
  connect(mSplitter, SIGNAL(splitterMoved(int, int)), this, SLOT(splitterMoved()));
  mSplitter->addWidget(mPicker);
  mSplitter->addWidget(mSheet);
  mSplitter->addWidget(mDataView);
  mSplitter->setSizes(sizes);

  readSettings();

  delete layout();
  QHBoxLayout* lay = new QHBoxLayout;
  lay->addWidget(mSplitter);
  lay->setMargin(0);
  setLayout(lay);
}

void IndicatorWidget::showBarData(BarTuple* bars)
{
  IndiWidgetSimple::showBarData(bars);
  mDataView->initView();
}

void IndicatorWidget::showFiIdMarketId(int fiId, int marketId)
{
  IndiWidgetSimple::showFiIdMarketId(fiId, marketId);
  mDataView->initView();
}

void IndicatorWidget::splitterMoved()
{
  QList<int> size = mSplitter->sizes();
  emit newSize(&size);
}

void IndicatorWidget::setSize(QList<int>& size)
{
  mSplitter->setSizes(size);
}

void IndicatorWidget::readSettings(const QString& setName, int number)
{
  if(setName == mSetName) return;

  IndiWidgetSimple::readSettings(setName, number);

  readSettings();
}

void IndicatorWidget::readSettings()
{
  SettingsFile sfile(mFullIndiSetsPath + mSetName);
  mSplitter->restoreState(sfile.getBA("IndicatorSplitter"));
  sfile.beginGroup(mName);
  mPicker->restoreState(sfile);
}

void IndicatorWidget::saveSettings()
{
  IndiWidgetSimple::saveSettings();

  SettingsFile sfile(mFullIndiSetsPath + mSetName);
  sfile.set("IndicatorSplitter", mSplitter->saveState());
  sfile.beginGroup(mName);
  mPicker->saveState(sfile);
}

void IndicatorWidget::mouseSlot(MyMouseEvent* event)
{
  IndiWidgetSimple::mouseSlot(event);
  mDataView->mouseSlot(event);
}
