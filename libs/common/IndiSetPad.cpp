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
{
  mSetSelector = new QComboBox;
  mSetSelector->setObjectName("SetSelector");
  //mSetSelector->setMinimumContentsLength(9);
  mSetSelector->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  QDir dir(mRcFile->getST("IndiSetsPath"));
  QStringList files = dir.entryList(QDir::Files, QDir::Name);
  mSetSelector->insertItems(0, files);
  connect(mSetSelector, SIGNAL(activated(int)), this, SLOT(setSelectorChanged()));

  mLayout->addWidget(mSetSelector);
}

IndiSetPad::~IndiSetPad()
{}

int IndiSetPad::loadSettings()
{
  int count = ButtonPad::loadSettings();

  if((1 == count) and ("Dummy" == mButtons.button(0)->text()))
  {
    setButtonName(mButtons.button(0), "DefaultSet");
  }

  // Remove buttons where the SetFile was deleted
  int  id = 0;
  bool deleted = false;
  forever
  {
    QAbstractButton* btn = mButtons.button(id);
    if(!btn) break;
    if(mSetSelector->findText(btn->text()) == -1)
    {
      deleteButton(btn);
      deleted = true;
    }
    else
    {
      ++id;
    }
  }

  if(deleted) return saveSettings();

  return count;
}

void IndiSetPad::addToToolBar(QToolBar* tb)
{
  QAction* act = tb->addWidget(mSetSelector);
  act->setObjectName("Act" + mSetSelector->objectName());

  ButtonPad::addToToolBar(tb);
}

void IndiSetPad::setCurrentSetup(const QString& setup)
{
  mSetSelector->setCurrentIndex(mSetSelector->findText(setup));
}

void IndiSetPad::buttonClicked(int id)
{
  //qDebug() << "IndiSetPad::buttonClicked:" << id << mButtons.button(id)->text();
  setCurrentSetup(mButtons.button(id)->text());
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
      saveSettings();
      break;
    case  1:    // OK
      setButtonName(btn, name.text());
      btn->setToolTip(tip.text());
      saveSettings();
      buttonClicked(id);
      break;
    case  2:    // Add
      btn = newButton(name.text());
      btn->setToolTip(tip.text());
      saveSettings();
      buttonClicked(mButtons.id(btn));
      break;
  }

  // Update the SetSelector
  int idx = mSetSelector->findText(name.text());
  if(-1 == idx)
  {
    for(idx = 0; idx < mSetSelector->count(); ++idx)
    {
      if(mSetSelector->itemText(idx) > name.text()) break;
    }
    mSetSelector->insertItem(idx, name.text());
  }

  idx = mSetSelector->findText(name.text());
  mSetSelector->setCurrentIndex(idx);
}

void IndiSetPad::setSelectorChanged()
{
  emit setupChosen(mSetSelector->currentText());
}
