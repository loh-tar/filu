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

#include <QAction>
#include <QBoxLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QToolBar>
#include <QToolButton>

#include "ButtonPad.h"
#include "DialogButton.h"
#include "RcFile.h"
#include "SettingsFile.h"

ButtonPad::ButtonPad(const QString& name, FClass* parent)
         : FWidget(parent, FUNC)
         , mSettings(0)
{
  setObjectName(name);
  mLayout = new QBoxLayout(QBoxLayout::LeftToRight);

  setLayout(mLayout);

  connect(&mButtons, SIGNAL(buttonClicked(int)), this, SLOT(buttonClicked(int)));
}

ButtonPad::ButtonPad(const QString& name, FClass* parent, const QString& className)
         : FWidget(parent, className)
         , mSettings(0)
{
  setObjectName(name);
  mLayout = new QBoxLayout(QBoxLayout::LeftToRight);

  setLayout(mLayout);

  connect(&mButtons, SIGNAL(buttonClicked(int)), this, SLOT(buttonClicked(int)));
}

ButtonPad::~ButtonPad()
{
    if(mSettings) delete mSettings;
}

void ButtonPad::addToToolBar(QToolBar* tb)
{
  if(!tb) return;

  foreach(QAbstractButton* btn, mButtons.buttons())
  {
    QAction* act = tb->addWidget(btn);
    act->setObjectName("Act" + btn->objectName());
  }

  setParent(tb);
}

SettingsFile* ButtonPad::openSettings()
{
  if(!mSettings)
  {
    QString filuHome = mRcFile->getPath("FiluHome");
    mSettings = new SettingsFile(filuHome + "ButtonPads/" + objectName() + ".ini");
  }

  return mSettings;
}

void ButtonPad::closeSettings()
{
  if(mSettings)
  {
    delete mSettings;
    mSettings = 0;
  }
}

void ButtonPad::loadSettings()
{
  bool alone = false;

  if(!mSettings)
  {
    alone = true;
    openSettings();
  }

  // Determine how many buttons we have
  QStringList buttonGroups = mSettings->childGroups();
  int count = buttonGroups.size();

  if(!count)
  {
    addDummyButton();
    if(alone) closeSettings();
    return;
  }

  for(int i = 0; i < count; ++i)
  {
    mSettings->beginGroup(QString::number(i));

    QString txt = mSettings->getST("Name");
    QToolButton* button = newButton(txt, i);

    txt = mSettings->getST("Tip");
    if(txt.isEmpty()) button->setToolTip(button->text());
    else button->setToolTip(txt);

    mSettings->endGroup();
  }

  if(alone) closeSettings();
}

void ButtonPad::saveSettings()
{
  bool alone = false;

  if(!mSettings)
  {
    alone = true;
    openSettings();
  }

  mSettings->remove("");  // Delete all settings

  foreach(QAbstractButton* btn, mButtons.buttons())
  {
    int id = mButtons.id(btn);
    mSettings->beginGroup(QString::number(id));

    mSettings->set("Name", btn->text());
    mSettings->set("Tip",  btn->toolTip());
    mSettings->endGroup();
  }

  if(alone) closeSettings();
}

void ButtonPad::orientationChanged(Qt::Orientation o) // Slot
{
  if(Qt::Horizontal == o)
  {
    mLayout->setDirection(QBoxLayout::LeftToRight);
  }
  else
  {
    mLayout->setDirection(QBoxLayout::TopToBottom);
  }

  parentWidget()->adjustSize();
}

void ButtonPad::buttonClicked(int id)
{
  //qDebug() << "ButtonPad::buttonClicked:" << id;
}

void  ButtonPad::buttonContextMenu(const QPoint& /*pos*/)
{
  //qDebug() << "ButtonPad::buttonContextMenu";
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
      break;
    case  1:    // OK
      setButtonName(btn, name.text());
      btn->setToolTip(tip.text());
      break;
    case  2:    // Add
      btn = newButton(name.text());
      btn->setToolTip(tip.text());
      break;
  }

  saveSettings();
}

QToolButton* ButtonPad::newButton(const QString& name, int id/* = -1*/)
{
  QToolButton* btn = new QToolButton;
  btn->setContextMenuPolicy(Qt::CustomContextMenu);
  btn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  btn->setText(name);
  btn->setObjectName("Btn" + name);

  connect(btn, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(buttonContextMenu(const QPoint&)));

  mLayout->addWidget(btn);

  if(id < 0) id = mButtons.buttons().size();
  mButtons.addButton(btn, id);

  if(!parent()) return btn;

  if(parent()->inherits("QToolBar"))
  {
    QAction* act = static_cast<QToolBar*>(parent())->addWidget(btn);
    act->setObjectName("Act" + btn->objectName());
  }

  return btn;
}

void ButtonPad::deleteButton(QAbstractButton* btn)
{
  QList<QAbstractButton *> buttonLst = mButtons.buttons();

  foreach(QAbstractButton* thisBtn, buttonLst)
  {
    int thisId = mButtons.id(thisBtn);
    if(thisId > mButtons.id(btn)) mButtons.setId(thisBtn, thisId - 1);
  }

  mButtons.removeButton(btn);

  if(parent())
  {
    if(parent()->inherits("QToolBar"))
    {
      QToolBar* tb = static_cast<QToolBar*>(parent());
      foreach(QAction* act, tb->actions())
      {
        if(tb->widgetForAction(act) != btn) continue;
        tb->removeAction(act);
      }
    }
  }

  delete btn;

  if(buttonLst.size() == 1) addDummyButton();
}

void ButtonPad::setButtonName(QAbstractButton* btn, const QString& name)
{
  btn->setText(name);
  btn->setObjectName("Btn" + name);

  if(!parent()) return;

  if(parent()->inherits("QToolBar"))
  {
    QToolBar* tb = static_cast<QToolBar*>(parent());
    foreach(QAction* act, tb->actions())
    {
      if(tb->widgetForAction(act) != btn) continue;
      act->setObjectName("Act" + btn->objectName());
      break;
    }
  }
}

QToolButton* ButtonPad::addDummyButton()
{
  QToolButton* button = newButton("Dummy", 0);
  button->setToolTip(tr("I'm the dummy button. I'll be back...if no one else is present"));

  mButtons.setId(button, 0);

  return button;
}
