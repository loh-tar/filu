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

#include <QContextMenuEvent>
#include <QFileSystemWatcher>
#include <QHBoxLayout>
#include <QMenu>

#include "IndiWidgetSimple.h"

#include "IndicatorPainter.h"
#include "PlotSheet.h"
#include "RcFile.h"

IndiWidgetSimple::IndiWidgetSimple(const QString& name, FClass* parent)
                : FWidget(parent, FUNC)
                , mName("1")
                , mSetName(name)
{
  init();
}

IndiWidgetSimple::IndiWidgetSimple(const QString& name, FClass* parent, const QString& className)
                : FWidget(parent, className)
                , mName("1")
                , mSetName(name)
{
  init();
}

IndiWidgetSimple::IndiWidgetSimple(const QString& name, int number, FClass* parent)
               : FWidget(parent, FUNC + QString::number(number + 1))
               , mName(QString::number(number + 1))
               , mSetName(name)
{
  init();
}

IndiWidgetSimple::IndiWidgetSimple(const QString& name, int number, FClass* parent, const QString& className)
               : FWidget(parent, className + QString::number(number + 1))
               , mName(QString::number(number + 1))
               , mSetName(name)
{
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

  mIndiWatcher = new QFileSystemWatcher(this);
  connect(mIndiWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(indiFileChanged()));

  QHBoxLayout* lay = new QHBoxLayout;
  lay->addWidget(mSheet);
  lay->setMargin(0);
  setLayout(lay);

  readSettings();

  addActions(mSheet->actions());
}

void IndiWidgetSimple::loadSetup(const QString& name, int number)
{
  if(name == mSetName) return;

  saveSettings();
  readSettings(name, number);
}

void IndiWidgetSimple::sync()
{
  if("1" != mName)  // Don't ask for a sync if you are the No.1
  {
    MyMouseEvent e;
    e.sender = mSheet;
    e.type   = eSyncReq;
    emit mouse(&e);
  }
}

void IndiWidgetSimple::useIndicator(const QString& file)
{
  mUsedIndiFile = file;
  mSheet->useIndicator(mUsedIndiFile);
  watchIndicator();

  QSettings settings(mFullIndiSetsPath + mSetName,  QSettings::IniFormat);
  settings.beginGroup(mName);
  settings.setValue("Indicator", mUsedIndiFile);
}

void IndiWidgetSimple::showBarData(BarTuple* bars)
{
  mSheet->showBarData(bars);
}

void IndiWidgetSimple::showFiIdMarketId(int fiId, int marketId)
{
  mSheet->showFiIdMarketId(fiId, marketId);
}

void IndiWidgetSimple::readSettings(const QString& setName, int number)
{
  if(setName == mSetName) return;

  mSetName = setName;
  mName    = QString::number(number + 1);

  readSettings();
}

void IndiWidgetSimple::readSettings()
{
  // FIXME: Hardcoded settings
  mSheet->useFont(QFont("DejaVu Sans", 9, 50, 0));
  mSheet->useScaleColor(QColor(Qt::white));
  mSheet->useGridColor(QColor(Qt::gray));
  mSheet->useSheetColor(QColor(Qt::black));
  mSheet->setPalette(QPalette(QColor(Qt::black)));
  mSheet->setDateRange(QDate(1900,1,1), QDate::currentDate());

  // Individual settings
  QSettings settings(mFullIndiSetsPath + mSetName,  QSettings::IniFormat);
  mSheet->setDensity(settings.value("Density", 10).toDouble());
  mSheet->showYScale(settings.value("ShowYScale", true).toBool());

  settings.beginGroup(mName);
  mUsedIndiFile = settings.value("Indicator", "Default").toString();
  mSheet->useIndicator(mUsedIndiFile);
  mSheet->showGrid(settings.value("ShowGrid", true).toBool());
  mSheet->showXScale(settings.value("ShowXScale", true).toBool());
  mSheet->mPainter->mScaleToScreen = settings.value("ScaleToScreen", 10).toInt();

  watchIndicator();
}

void IndiWidgetSimple::saveSettings()
{
  QSettings settings(mFullIndiSetsPath + mSetName,  QSettings::IniFormat);
  if("1" == mName)
  {
    settings.setValue("Density", mSheet->mPainter->mDensity);
    settings.setValue("ShowYScale", mSheet->mPainter->mShowYScale);
  }

  settings.beginGroup(mName);
  settings.setValue("ShowGrid", mSheet->mPainter->mShowGrid);
  settings.setValue("ShowXScale", mSheet->mPainter->mShowXScale);
  settings.setValue("ScaleToScreen", mSheet->mPainter->mScaleToScreen);
}

void IndiWidgetSimple::mouseSlot(MyMouseEvent* event)
{
  if((event->type == eSyncReq) and ("1" != mName)) return; // Block eSyncReq if you are not No.1
  mSheet->mouseSlot(event);
}

void IndiWidgetSimple::chartObjectChosen(const QString& type) // Slot
{
  mSheet->mNewCOType = type;
}

void IndiWidgetSimple::contextMenuEvent(QContextMenuEvent* event)
{
  QMenu::exec(actions(), event->globalPos());
}

void IndiWidgetSimple::indiFileChanged()
{
  mSheet->useIndicator(mUsedIndiFile);
}

void IndiWidgetSimple::watchIndicator()
{
  foreach(const QString indi, mIndiWatcher->files())
  {
    mIndiWatcher->removePath(indi);
  }

  mIndiWatcher->addPath(mRcFile->getST("IndicatorPath") + mUsedIndiFile);
}
