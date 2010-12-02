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

#include "IndicatorPicker.h"
#include "FWidget.h"

IndicatorPicker::IndicatorPicker(FClass* parent)
               : QTreeWidget(0)
               , FClass(parent)
{
  setColumnCount(1);
  readSettings();
  raiseTree();
  header()->hide();
  //setHeaderLabel("Indicators");
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  connect(this, SIGNAL(itemClicked (QTreeWidgetItem *, int))
        , this, SLOT(clicked(QTreeWidgetItem *, int)));
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
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      qDebug() << "IndicatorPicker::raiseTree: can't open indicator file:"
               << files.at(i);
      continue;
    }

    // Read the indicator
    QTextStream in(&file);
    while (!in.atEnd())
    {
      QString line = in.readLine();
      line.remove(" ");
      if(line.startsWith("*AddToGroup:"))
      {
        line = line.remove("*AddToGroup:");
        if(!groups.contains(line))
        {
          QTreeWidgetItem* group = new QTreeWidgetItem(this, QStringList(line));
          groups.insert(line, group);
        }

        new QTreeWidgetItem(groups.value(line), QStringList(files.at(i)));
      }
    }
    file.close();
  }
}
