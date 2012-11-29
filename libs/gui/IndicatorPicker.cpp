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

#include <QDir>
#include <QFile>
#include <QFileSystemWatcher>
#include <QHeaderView>
#include <QTextStream>

#include "IndicatorPicker.h"

#include "RcFile.h"
#include "FWidget.h"

IndicatorPicker::IndicatorPicker(FClass* parent)
               : QTreeWidget(0)
               , FClass(parent, FUNC)
{
  setColumnCount(1);
  readSettings();
  raiseTree();
  header()->hide();
  //setHeaderLabel("Indicators");
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  connect(this, SIGNAL(itemClicked (QTreeWidgetItem *, int))
        , this, SLOT(clicked(QTreeWidgetItem *, int)));

  QFileSystemWatcher* watch = new QFileSystemWatcher(this);
  watch->addPath(mIndicatorPath);
  connect(watch, SIGNAL(directoryChanged(const QString&))
        , this, SLOT(indicatorsChanged(const QString&)));
}

IndicatorPicker::~IndicatorPicker()
{}

void IndicatorPicker::clicked(QTreeWidgetItem * item, int column)
{
  if(!item->parent()) return;
  emit changed(item->text(column));
}

void IndicatorPicker::readSettings()
{
  mIndicatorPath = mRcFile->getST("IndicatorPath");
}

void IndicatorPicker::saveState(QSettings& setting)
{
  QString group;

  if(currentItem())
  {
    if(currentItem()->parent()) group = currentItem()->parent()->text(0);
  }

  // Don't save the indicator, will saved by 'the beef' widget e.g. IndiWidgetSimple
  setting.setValue("PickerGroup", group);
}

void IndicatorPicker::restoreState(QSettings& setting)
{
  QString group;
  QString indi;

  group = setting.value("PickerGroup").toString();
  indi =  setting.value("Indicator").toString();

  climbOnTree(group, indi);
}

void IndicatorPicker::indicatorsChanged(const QString& path)
{
  QString group;
  QString indi;

  if(currentItem())
  {
    if(currentItem()->parent()) group = currentItem()->parent()->text(0);
    indi = currentItem()->text(0);
  }

  clear();
  raiseTree();
  climbOnTree(group, indi);
}

void IndicatorPicker::raiseTree()
{
  QDir dir(mIndicatorPath);
  QStringList files = dir.entryList(QDir::Files, QDir::Name);

  QTreeWidgetItem* top = new QTreeWidgetItem(this, QStringList("All"));

  QHash<QString, QTreeWidgetItem*> groups;

  for(int i = 0; i < files.size(); ++i)
  {
    if(files.at(i).endsWith("~")) continue;
    new QTreeWidgetItem(top, QStringList(files.at(i)));

    QFile file(mIndicatorPath + files.at(i));
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      fatal(FUNC, QString("Can't open indicator file '%1'.").arg(files.at(i)));
      continue;
    }

    // Read the indicator
    QTextStream in(&file);
    while(!in.atEnd())
    {
      QString line = in.readLine();
      line.remove(" ");
      if(line.startsWith("*AddToGroup"))
      {
        line.remove(0, 12); // Remove "*AddToGroup" plus the seperator char e.g. ":"
        foreach(QString group, line.split(","))
        {
          if(!groups.contains(group))
          {
            groups.insert(group, new QTreeWidgetItem(this, QStringList(group)));
          }

          new QTreeWidgetItem(groups.value(group), QStringList(files.at(i)));
        }
      }
    }
    file.close();
  }
}

void IndicatorPicker::climbOnTree(const QString& group, const QString& indi)
{
  // Restore old selection
  if(group.isEmpty()) return;

  // Collapse all items
  for(int i = 0; i < invisibleRootItem()->childCount(); ++i)
  {
    collapseItem(invisibleRootItem()->child(i));
  }

  QList<QTreeWidgetItem*> gLst = findItems(group, Qt::MatchExactly, 0);

  if(!gLst.size()) return;

  QTreeWidgetItem* item = gLst.at(0);
  expandItem(item);

  for(int i = 0; i < item->childCount(); ++i)
  {
    if(item->child(i)->text(0) != indi) continue;

    setCurrentItem(item->child(i));
    //scrollToItem(item->child(i)); FIXME Doesn't work
    break;
  }
}
