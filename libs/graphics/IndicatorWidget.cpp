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

#include "IndicatorWidget.h"

#include "IndicatorPainter.h"
#include "IndicatorPicker.h"
#include "IndicatorDataView.h"
#include "PlotSheet.h"

IndicatorWidget::IndicatorWidget(const QString& name, FWidget* parent)
               : FWidget(parent)
               , mName("1")
               , mSetName(name)
{
  init(parent);
}

IndicatorWidget::IndicatorWidget(const QString& name, const int number, FWidget* parent)
               : FWidget(parent)
               , mSetName(name)
{
  mName = QString::number(number + 1);
  init(parent);
}

IndicatorWidget::~IndicatorWidget()
{
  saveSettings();
}

void IndicatorWidget::init(FWidget* parent)
{
  mFullIndiSetsPath = mRcFile->getST("IndiSetsPath");

  QList<int> sizes;

  mPicker = new IndicatorPicker(parent);
  sizes.append(mPicker->minimumSizeHint().width());

  mSheet = new PlotSheet(parent);
  connect(mSheet, SIGNAL(mouse(MyMouseEvent*)), this, SIGNAL(mouse(MyMouseEvent*)));
  sizes.append(mSheet->sizeHint().width()*2);

  connect(mPicker, SIGNAL(changed(QString))
        , this, SLOT(useIndicator(const QString &)));

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

  QHBoxLayout* lay = new QHBoxLayout;
  lay->addWidget(mSplitter);
  lay->setMargin(0);
  setLayout(lay);
}

void IndicatorWidget::setName(const QString& name)
{
  saveSettings();
  mName = name;
  readSettings();
}

void IndicatorWidget::useIndicator(const QString& file)
{
  mSheet->useIndicator(file);
  QSettings settings(mFullIndiSetsPath + mSetName,  QSettings::IniFormat);
  settings.beginGroup(mName);
  settings.setValue("Indicator", file);
}

void IndicatorWidget::showBarData(BarTuple* bars)
{
  mSheet->showBarData(bars);
  mDataView->initView(); //FIXME:here only added for CalcWatchDogs.cpp
}

void IndicatorWidget::splitterMoved(/*int pos, int idx*/)
{
  QList<int> size = mSplitter->sizes();
  emit newSize(&size);
}

void IndicatorWidget::setSize(QList<int> &size)
{
  mSplitter->setSizes(size);
}

void IndicatorWidget::showFiIdMarketId(int fiId, int marketId)
{
  mSheet->showFiIdMarketId(fiId, marketId);
}

void IndicatorWidget::readSettings()
{
  QSettings settings(mFullIndiSetsPath + mSetName,  QSettings::IniFormat);
  mSplitter->restoreState(settings.value("IndicatorSplitter").toByteArray());
  mSheet->mPainter->setDensity(settings.value("Density", 10).toDouble());

  settings.beginGroup(mName);
  mSheet->useIndicator(settings.value("Indicator", "Default").toString());
  mSheet->mPainter->mShowXScale = settings.value("ShowXScale", true).toBool();
  mSheet->mPainter->mScaleToScreen = settings.value("ScaleToScreen", 10).toInt();
}

void IndicatorWidget::saveSettings()
{
  QSettings settings(mFullIndiSetsPath + mSetName,  QSettings::IniFormat);
  settings.setValue("IndicatorSplitter", mSplitter->saveState());
  settings.setValue("Density", mSheet->mPainter->mDensity);

  settings.beginGroup(mName);
  settings.setValue("ShowXScale", mSheet->mPainter->mShowXScale);
  settings.setValue("ScaleToScreen", mSheet->mPainter->mScaleToScreen);
}

void IndicatorWidget::mouseSlot(MyMouseEvent* event)
{
  mSheet->mouseSlot(event);
  mDataView->mouseSlot(event);
}

void IndicatorWidget::chartObjectChosen(const QString& type)
{
  mSheet->mNewCOType = type;
}