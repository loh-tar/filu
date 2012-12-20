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

#include <QAction>
#include <QBoxLayout>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>

#include "IndiSetPad.h"

#include "DialogButton.h"
#include "RcFile.h"

IndiSetPad::IndiSetPad(const QString& name, FClass* parent)
          : ButtonPad(name, parent, FUNC)
{
  mSetSelector = new QComboBox;
  mSetSelector->setObjectName("SetSelector");
  mSetSelector->setToolTip(tr("Select an existing indicator set\n"
                              "To create a new set edit one of the buttons"));
  //mSetSelector->setMinimumContentsLength(9);
  mSetSelector->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  connect(mSetSelector, SIGNAL(activated(int)), this, SLOT(setSelectorChanged()));

  mLayout->addWidget(mSetSelector);
}

IndiSetPad::~IndiSetPad()
{}

void IndiSetPad::loadSettings()
{
  fillSetSelector();

  ButtonPad::loadSettings();

  QString isp = mRcFile->getST("IndiSetsPath");

  foreach(QAbstractButton* btn, mButtons.buttons())
  {
    if(!QFile::exists(isp + btn->text()))
    {
      // Remove buttons where the SetFile was deleted by any foreign action
      deleteButton(btn);
      continue;
    }

    QSettings settings(mRcFile->getST("IndiSetsPath") + btn->text(),  QSettings::IniFormat);
    btn->setToolTip(settings.value("Tip").toString());
  }
}

void IndiSetPad::saveSettings()
{
  // Don't call ButtonPad::saveSettings() to avoid saving of button tip
  // We want to save this in the IndiSetPad config file
  QSettings* settings = openSettings();
  settings->remove("");  // Delete all settings

  foreach(QAbstractButton* btn, mButtons.buttons())
  {
    int id = mButtons.id(btn);
    settings->beginGroup(QString::number(id));
    settings->setValue("Name",       btn->text());
    settings->endGroup();

    saveTip(btn->text(), btn->toolTip());
  }

  closeSettings();
}

void IndiSetPad::fillSetSelector()
{
  mSetSelector->blockSignals(true);

  QString current = mSetSelector->currentText();

  QString ip  = mRcFile->getST("InstallPath");
  QString isp = mRcFile->getST("IndiSetsPath");

  // Check if default sets exist and copy back from install dir
  if(!QFile::exists(isp + "Default")) QFile::copy(ip + "userfiles/IndicatorSets/Default"
                                               , isp + "Default");

  if(!QFile::exists(isp + "ZoomWidget")) QFile::copy(ip + "userfiles/IndicatorSets/ZoomWidget"
                                                  , isp + "ZoomWidget");

  QDir dir(isp);
  QStringList files = dir.entryList(QDir::Files, QDir::Name);

  mSetSelector->clear();
  mSetSelector->insertItems(0, files);

  int idx = mSetSelector->findText(current);
  if(idx > -1) mSetSelector->setCurrentIndex(idx);

  mSetSelector->blockSignals(false);
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
  QDir dir(mRcFile->getST("IndiSetsPath"));
  QStringList files = dir.entryList(QDir::Files, QDir::Name);
  QComboBox name;
  name.setToolTip(tr("Select an existing set, or\nenter a new name to create a new indicator set"));
  name.insertItems(0, files);
  name.setCurrentIndex(name.findText(btn->text()));
  name.setEditable(true);
  connect(&name, SIGNAL(activated(const QString&)), this, SLOT(getTip(const QString&)));

  layout.addWidget(&name, row++, 1);
  layout.setColumnStretch(1, 1);

  label = new QLabel(tr("Tip"));
  label->setAlignment(Qt::AlignRight);
  layout.addWidget(label, row, 0);
  QLineEdit tip(btn->toolTip());
  tip.setToolTip(tr("Button tool tip"));
  tip.setCursorPosition(0);
  mIndiSetTip = &tip;
  layout.addWidget(&tip, row++, 1, 1, 2);
  layout.setColumnStretch(2, 3);

  // Add an empty row to take unused space
  layout.addWidget(new QWidget, row, 1);
  layout.setRowStretch(row++, 2);

  // Build the button line
  QDialogButtonBox dlgBtns(QDialogButtonBox::Save | QDialogButtonBox::Discard);
  QPushButton* db = dlgBtns.button(QDialogButtonBox::Discard);
  dlgBtns.addButton(db, QDialogButtonBox::RejectRole);
  connect(&dlgBtns, SIGNAL(accepted()), &dialog, SLOT(accept()));
  connect(&dlgBtns, SIGNAL(rejected()), &dialog, SLOT(reject()));

  DialogButton* delSetBtn = new DialogButton(tr("&Delete"), -2);
  delSetBtn->setToolTip(tr("Remove button AND DELETE indicator set"));
  dlgBtns.addButton(delSetBtn, QDialogButtonBox::ActionRole);
  connect(delSetBtn, SIGNAL(clicked(int)), &dialog, SLOT(done(int)));

  DialogButton* remove = new DialogButton(tr("&Remove"), -1);
  remove->setToolTip(tr("Remove button but keep indicator set"));
  dlgBtns.addButton(remove, QDialogButtonBox::ActionRole);
  connect(remove, SIGNAL(clicked(int)), &dialog, SLOT(done(int)));

  DialogButton* add = new DialogButton(tr("&Add"), 2);
  add->setToolTip(tr("Copy to new indicator set and button"));
  dlgBtns.addButton(add, QDialogButtonBox::ActionRole);
  connect(add, SIGNAL(clicked(int)), &dialog, SLOT(done(int)));

  layout.addWidget(&dlgBtns, row, 1, 1, 2);

  dialog.setWindowTitle(tr("IndiSetPad - Edit button '%1'").arg(btn->text()));
  dialog.setMinimumWidth(350);

  int retVal = dialog.exec();
  QString newName = name.lineEdit()->text();
  switch(retVal)
  {
    case 0:     // Cancel
      return;
      break;
    case -2:    // Delete Set
    {
      int ret = QMessageBox::warning(&dialog
                  , tr("IndiSetPad - Last chance to keep your data")
                  , tr("Delete indicator set <b>'%1'</b> and not only the button<b>?</b>").arg(btn->text())
                  , QMessageBox::Yes | QMessageBox::No
                  , QMessageBox::No);

      if(ret == QMessageBox::No) return;

      deleteSet(newName);
      deleteButton(btn);
      break;
    }
    case -1:    // Remove
      deleteButton(btn);
      break;
    case  1:    // OK
      setButtonName(btn, newName);
      btn->setToolTip(tip.text());
      break;
    case  2:    // Add
      // Don't add button if name is in use
      foreach(QAbstractButton* btn, mButtons.buttons())
      {
        if(btn->text() == newName) return;
      }

      btn = newButton(newName);
      btn->setToolTip(tip.text());
      buttonClicked(mButtons.id(btn));
      break;
  }

  saveSettings();
}

void IndiSetPad::setSelectorChanged()
{
  emit setupChosen(mSetSelector->currentText());
}

QToolButton* IndiSetPad::addDummyButton()
{
  QToolButton* button = ButtonPad::addDummyButton();

  QString tt = tr("Most usual - Candle Stick Chart with Volume");
  tt.append("\n");
  tt.append(button->toolTip());
  button->setToolTip(tt);
  button->setText("Default");

  return button;
}

void IndiSetPad::deleteSet(const QString& name)
{
  if("Default" == name or "ZoomWidget" == name) return; // Don't delete

  QFile::remove(mRcFile->getST("IndiSetsPath") + name);
  fillSetSelector();
}

void IndiSetPad::saveTip(const QString& name, const QString& tip)
{
  if("Default" == name or "ZoomWidget" == name) return; // Don't change

  QSettings settings(mRcFile->getST("IndiSetsPath") + name,  QSettings::IniFormat);
  settings.setValue("Tip", tip);
}

void IndiSetPad::getTip(const QString& name)
{
  QSettings settings(mRcFile->getST("IndiSetsPath") + name,  QSettings::IniFormat);
  mIndiSetTip->setText(settings.value("Tip").toString());
}
