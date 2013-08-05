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

#include <QCheckBox>
#include <QHeaderView>

#include "IndicatorDataView.h"

#include "DataTupleSet.h"
#include "Indicator.h"
#include "IndicatorPainter.h"
#include "PlotSheet.h"
#include "RcFile.h"
#include "SettingsFile.h"

IndicatorDataView::IndicatorDataView(PlotSheet* parent)
                 : QTabWidget(parent)
                 , FClass(parent, FUNC)
{
  setTabPosition(QTabWidget::West);
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

  connect(parent, SIGNAL(newIndicator(const QString&)), this, SLOT(initView(const QString&)));

  mPainter = parent->mPainter;

  mDataView.setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  mDataView.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  mDataView.setSelectionBehavior(QAbstractItemView::SelectRows);
  mDataView.setEditTriggers(QAbstractItemView::NoEditTriggers);
  mDataView.horizontalHeader()->hide();
  mDataView.verticalHeader()->hide();
  mDataView.horizontalHeader()->setStretchLastSection(true);
  mDataView.setGridStyle(Qt::NoPen);

  mFilterView.setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  mFilterView.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  mFilterView.horizontalHeader()->hide();
  mFilterView.verticalHeader()->hide();
  mFilterView.horizontalHeader()->setStretchLastSection(true);
  mFilterView.setGridStyle(Qt::NoPen);

  addTab(&mDataView, tr("Data"));
  addTab(&mFilterView, tr("Filter"));

  connect(this, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

  readSettings();
}

IndicatorDataView::~IndicatorDataView()
{
  saveSettings();
}

void IndicatorDataView::mouseSlot(MyMouseEvent* me)
{
  if(me->type == eMouseMove)
  {
    if(!this->size().width() or !this->size().height())
    {
      return; // Not visible, no update needed
    }

    // Update data view
    DataTupleSet* data = mPainter->mData;

    if(!data) return;
    if(mDataView.rowCount() == 0) return;

    int mouseXPos = mPainter->mMouseXPos;
    int firstBar  = mPainter->mFirstBarToShow;

    data->rewind(firstBar + mouseXPos);

    int row = 0;
    if(mDataView.item(0, 0)->text() == "Date")
    {
      QDate date;
      data->getDate(date);
      mDataView.item(0, 1)->setText(date.toString(/*Qt::LocaleDate*/));
      ++row;
    }

    double value;
    for(; row < mDataView.rowCount(); ++row)
    {
      if(!data->getValue(mDataView.item(row, 0)->text(), value))
      {
        mDataView.item(row, 1)->setText("-");
      }
      else
      {
        int precision = 2;
        if(value < 1)    precision = 4;
        if(value > 1000) precision = 0;
        mDataView.item(row, 1)->setText(QString::number(value, 'f', precision));
      }
    }

    //mDataView.update();
    mDataView.resizeColumnsToContents();

  }//if(me->type == eMouseMove)

}

void IndicatorDataView::initView()
{
  initView(mIndicator);
}

void IndicatorDataView::initView(const QString& indicator)
{
  if(!mPainter->mIndicator) return;

  mIndicator = indicator;

  // Delete old entries

  // Insert new entries
  QStringList var;
  //mPainter->mData->getVariableNames(var);
  QSet<QString> varList;
  mPainter->mIndicator->getVariableNames(&varList);
  var = varList.values();
  //qDebug() << "IndicatorDataView::initView()" << var << varList.size();

  // Sort the variable list, but place the bardata as block in front
  var.removeAt(var.indexOf("OPEN"));
  var.removeAt(var.indexOf("HIGH"));
  var.removeAt(var.indexOf("LOW"));
  var.removeAt(var.indexOf("CLOSE"));
  var.removeAt(var.indexOf("VOLUME"));
  var.removeAt(var.indexOf("OPINT"));
  var.sort();
  var.prepend("OPINT");
  var.prepend("VOLUME");
  var.prepend("CLOSE");
  var.prepend("LOW");
  var.prepend("HIGH");
  var.prepend("OPEN");
  var.prepend("Date"); // Should not be a variable but is also interesting

  // And now insert entries
  int row = 0;
  QString name;
  QCheckBox* cb;

  mFilterView.setColumnCount(1);
  mFilterView.setRowCount(var.size());

  // Restore saved settings, if some
  QString filterPath;
  filterPath = mRcFile->getST("FiluHome");
  filterPath.append("IndicatorFilterSettings/");
  SettingsFile sfile(filterPath + mIndicator);

  foreach(name, var)
  {
    cb = new QCheckBox(name);
    if(sfile.getBL(name))
    {
      cb->setCheckState(Qt::Checked);
    }
    else
    {
      cb->setCheckState(Qt::Unchecked);
    }
    connect(cb, SIGNAL(stateChanged(int)), this, SLOT(filterChanged(int)));
    mFilterView.setCellWidget(row++, 0, cb);
  }

  mFilterView.resizeColumnsToContents();
  mFilterView.resizeRowsToContents();
  mFilterChanged = true;
  tabChanged(0);
}

void IndicatorDataView::filterChanged(int)
{
  mFilterChanged = true;
}

void IndicatorDataView::tabChanged(int index)
{
  //qDebug() << "IndicatorDataView::tabChanged()" << index << mFilterChanged;

  if(index == 0 and mFilterChanged)
  {
    mFilterChanged = false;

    // And save the settings
    QString filterPath;
    filterPath = mRcFile->getST("FiluHome");
    filterPath.append("IndicatorFilterSettings/");
    SettingsFile sfile(filterPath + mIndicator);

    // Delete old view
    mDataView.setRowCount(0);
    mDataView.setColumnCount(2);

    // Rebulid view
    QTableWidgetItem* item;
    QString varName;
    for(int row = 0; row < mFilterView.rowCount(); ++row)
    {
      varName = static_cast<QCheckBox*>(mFilterView.cellWidget(row, 0))->text();

      if(!static_cast<QCheckBox*>(mFilterView.cellWidget(row, 0))->isChecked())
      {
        sfile.set(varName, false);
        continue;
      }
      else
      {
        sfile.set(varName, true);
        mDataView.insertRow(mDataView.rowCount());
      }

      item = new QTableWidgetItem(varName);
      mDataView.setItem(mDataView.rowCount() - 1, 0, item);

      item = new QTableWidgetItem("");
      mDataView.setItem(mDataView.rowCount() - 1, 1, item);
    }

    mDataView.resizeColumnsToContents();
    mDataView.resizeRowsToContents();
  }
}

void IndicatorDataView::readSettings()
{
//   QSettings settings(mFullIndiSetsPath + mName,  QSettings::IniFormat);
//   mSheet->useIndicator(settings.value("Indicator", "Default").toString());
//   mSplitter->restoreState(settings.value("SplitterSize").toByteArray());

}

void IndicatorDataView::saveSettings()
{
/*  QString filuHome;

  RcFile rcFile;
  rcFile.get("IndiSetsPath", filuHome);

  QSettings settings(filuHome + mName,  QSettings::IniFormat);
  settings.setValue("SplitterSize", mSplitter->saveState());*/

}
