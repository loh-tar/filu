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

#include "IndiWidgetSimple.h"

#include "PlotSheet.h"
#include "IndicatorPainter.h"

IndiWidgetSimple::IndiWidgetSimple(const QString& name, FWidget* parent)
                : FWidget(parent)
                , mName("1")
                , mSetName(name)
{
  init();
}

IndiWidgetSimple::IndiWidgetSimple(const QString& name, const int number, FWidget* parent)
               : FWidget(parent)
               , mSetName(name)
{
  mName = QString::number(number + 1);
  init();
}

IndiWidgetSimple::~IndiWidgetSimple()
{
  saveSettings();
}

void IndiWidgetSimple::init()
{
  mFullIndiSetsPath = mRcFile->getST("IndiSetsPath");

  mSheet = new PlotSheet(this);
  connect(mSheet, SIGNAL(mouse(MyMouseEvent*)), this, SIGNAL(mouse(MyMouseEvent*)));

  QHBoxLayout* lay = new QHBoxLayout;
  lay->addWidget(mSheet);
  lay->setMargin(0);
  setLayout(lay);

  readSettings();
}

void IndiWidgetSimple::setName(const QString& name)
{
  saveSettings();
  mName = name;
  readSettings();
}

void IndiWidgetSimple::useIndicator(const QString& file)
{
  mSheet->useIndicator(file);
  QSettings settings(mFullIndiSetsPath + mSetName,  QSettings::IniFormat);
  settings.beginGroup(mName);
  settings.setValue("Indicator", file);
}

void IndiWidgetSimple::showBarData(BarTuple* bars)
{
  mSheet->showBarData(bars);
}

void IndiWidgetSimple::showFiIdMarketId(int fiId, int marketId)
{
  mSheet->showFiIdMarketId(fiId, marketId);
}

void IndiWidgetSimple::readSettings()
{
  QSettings settings(mFullIndiSetsPath + mSetName,  QSettings::IniFormat);
  mSheet->mPainter->setDensity(settings.value("Density", 10).toDouble());

  settings.beginGroup(mName);
  mSheet->useIndicator(settings.value("Indicator", "Default").toString());
  mSheet->mPainter->mShowXScale = settings.value("ShowXScale", true).toBool();
  mSheet->mPainter->mScaleToScreen = settings.value("ScaleToScreen", 10).toInt();
}

void IndiWidgetSimple::saveSettings()
{
  QSettings settings(mFullIndiSetsPath + mSetName,  QSettings::IniFormat);
  settings.setValue("Density", mSheet->mPainter->mDensity);

  settings.beginGroup(mName);
  settings.setValue("ShowXScale", mSheet->mPainter->mShowXScale);
  settings.setValue("ScaleToScreen", mSheet->mPainter->mScaleToScreen);
}

void IndiWidgetSimple::mouseSlot(MyMouseEvent* event)
{
  mSheet->mouseSlot(event);
}

void IndiWidgetSimple::chartObjectChosen(const QString& type)
{
  mSheet->mNewCOType = type;
}