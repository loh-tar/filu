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

#include "IndiSetPad.h"
#include "DialogButton.h"

IndiSetPad::IndiSetPad(const QString& name, FClass* parent)
          : ButtonPad(name, parent)
{}

IndiSetPad::~IndiSetPad()
{}

int IndiSetPad::loadSettings()
{
  int count = ButtonPad::loadSettings();

  QString indiSetsPath = mRcFile->getST("IndiSetsPath");

  for(int i = 0; i < count; ++i)
  {
    QSettings settings(indiSetsPath + mButtons.button(i)->text(),  QSettings::IniFormat);
    mIndiCount.append(settings.value("IndicatorCount", 1).toInt());
  }

  return count;
}

int IndiSetPad::saveSettings()
{
  int count = ButtonPad::saveSettings();

  QString indiSetsPath = mRcFile->getST("IndiSetsPath");

  for(int i = 0; i < count; ++i)
  {
    QSettings settings(indiSetsPath + mButtons.button(i)->text(),  QSettings::IniFormat);
    settings.setValue("IndicatorCount", mIndiCount.at(i));
  }

  return count;
}

void IndiSetPad::buttonClicked(int id)
{
  //qDebug() << "IndiSetPad::buttonClicked:" << id << mButtons.button(id)->text();
  emit setupChosen(mButtons.button(id)->text());
}

void  IndiSetPad::buttonContextMenu(const QPoint& /*pos*/)
{
  QAbstractButton* btn = static_cast<QAbstractButton*>(sender());
  int id = mButtons.id(btn);

  QDialog      dialog;
  QGridLayout  layout(&dialog);
  QLabel*      label;
  int          row = 0;            // Count layout rows

  label = new QLabel(tr("Name"));
  label->setAlignment(Qt::AlignRight);
  layout.addWidget(label, row, 0);
  QLineEdit name(btn->text());
  layout.addWidget(&name, row++, 1);
  layout.setColumnStretch(1, 1);

  label = new QLabel(tr("Tool Tip"));
  label->setAlignment(Qt::AlignRight);
  layout.addWidget(label, row, 0);
  QLineEdit tip(btn->toolTip());
  layout.addWidget(&tip, row++, 1);

  label = new QLabel(tr("Indicator Count"));
  label->setAlignment(Qt::AlignRight);
  layout.addWidget(label, row, 0);
  QSpinBox indiCount;
  indiCount.setMinimum(1);
  indiCount.setValue(mIndiCount.at(id));
  layout.addWidget(&indiCount, row++, 1);

  // Add an empty row to take unused space
  layout.addWidget(new QWidget, row, 1);
  layout.setRowStretch(row++, 2);

  // Build the button line
  QDialogButtonBox dlgBtns(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(&dlgBtns, SIGNAL(accepted()), &dialog, SLOT(accept()));
  connect(&dlgBtns, SIGNAL(rejected()), &dialog, SLOT(reject()));

  DialogButton* remove = new DialogButton(tr("&Remove"), -1);
  dlgBtns.addButton(remove, QDialogButtonBox::ActionRole);
  connect(remove, SIGNAL(clicked(int)), &dialog, SLOT(done(int)));

  DialogButton* add = new DialogButton(tr("&Add"), 2);
  dlgBtns.addButton(add, QDialogButtonBox::ActionRole);
  connect(add, SIGNAL(clicked(int)), &dialog, SLOT(done(int)));

  layout.addWidget(&dlgBtns, row, 1, 1, 2);

  dialog.setMinimumWidth(350);

  switch (dialog.exec())
  {
    case 0:     // Cancel
      return;
      break;
    case -1:    // Remove
      deleteButton(btn);
      mIndiCount.removeAt(id);
      saveSettings();
      break;
    case  1:    // OK
      setButtonName(btn, name.text());
      btn->setToolTip(tip.text());
      mIndiCount[id] = indiCount.value();
      saveSettings();
      buttonClicked(id);
      break;
    case  2:    // Add
      btn = newButton(name.text());
      btn->setToolTip(tip.text());
      mIndiCount.append(indiCount.value());
      mButtons.setId(btn, mIndiCount.size() - 1);
      saveSettings();
      buttonClicked(mButtons.id(btn));
      break;
  }
}
