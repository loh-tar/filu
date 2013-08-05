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
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QToolButton>

#include "LaunchPad.h"

#include "DialogButton.h"
#include "FiluU.h"
#include "SettingsFile.h"
#include "SymbolTuple.h"
#include "SymbolTypeTuple.h"

static const QString cDummyTip = QObject::tr("Print data to stdout - Must run from terminal to see these");

LaunchPad::LaunchPad(const QString& name, FClass* parent)
         : ButtonPad(name, parent, FUNC)
{}

LaunchPad::~LaunchPad()
{}

void LaunchPad::loadSettings()
{
  SettingsFile* sfile = openSettings();

  ButtonPad::loadSettings();

  for(int i = 0; i < mButtons.buttons().size(); ++i)
  {
    sfile->beginGroup(QString::number(i));

    mCommands.append(sfile->getST("Command"));
    mSymbolTypes.append(sfile->getST("SymbolType"));
    mMultis.append(sfile->getBL("AllMarkets", false));

    sfile->endGroup();
  }

  closeSettings();

  return;
}

void LaunchPad::saveSettings()
{
  SettingsFile* sfile = openSettings();

  ButtonPad::saveSettings();

  int btnCount = mButtons.buttons().size();

  // Don't save the 'I'll be back' info
  if(btnCount < 2 and mButtons.button(0)->text() == "Echo")
  {
    sfile->beginGroup(QString::number(0));
    sfile->set("Tip", cDummyTip);
    sfile->endGroup();
  }

  for(int i = 0; i < btnCount; ++i)
  {
    sfile->beginGroup(QString::number(i));

    sfile->set("Command",    mCommands.at(i));
    sfile->set("SymbolType", mSymbolTypes.at(i));
    sfile->set("AllMarkets", mMultis.at(i));

    sfile->endGroup();
  }

  closeSettings();
}

void LaunchPad::newSelection(int fiId, int marketId)
{
  //qDebug() << "LaunchPad::newSelection:" << fiId << marketId;
  mFiId     = fiId;
  mMarketId = marketId;
}

void LaunchPad::buttonClicked(int id)
{
  SymbolTuple* st = mFilu->getSymbols(mFiId);

  if(!st)
  {
    // Set some dummy values that we can call
    // the program anyway a FI was not selected
    SymbolTuple st(1);
    st.next();
    st.set("Foo", "NoMarket", "Reuters");
    execCmd(mCommands.at(id), &st);
    return;
  }

  bool found = false;
  while(st->next())
  {
    if(mSymbolTypes.at(id).isEmpty())
    {
      // Exec cmd only with first symbol to FI
      // The symbol is anyway not used
      found = true;
      execCmd(mCommands.at(id), st);
      break;
    }
    else if(st->owner() == mSymbolTypes.at(id))
    {
      if(mMultis.at(id))
      {
        // Exec cmd with each market to symbol type
        found = true;
        execCmd(mCommands.at(id), st);
      }
      else if(st->marketId() == mMarketId)
      {
        // Exec cmd with unique symbol type and market
        found = true;
        execCmd(mCommands.at(id), st);
        break;
      }
    }
  }

  if(!found)
  {
    verbose(FUNC, tr("No symbol type '%1' found to FiId %2").arg(mSymbolTypes.at(id))
                                                            .arg(mFiId));
  }

  delete st;
}

void  LaunchPad::buttonContextMenu(const QPoint& /*pos*/)
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
  name.setToolTip(tr("Button caption"));
  layout.addWidget(&name, row++, 1);
  layout.setColumnStretch(1, 1);

  label = new QLabel(tr("Tip"));
  label->setAlignment(Qt::AlignRight);
  layout.addWidget(label, row, 0);
  QLineEdit tip(btn->toolTip());
  tip.setToolTip(tr("Button tool tip"));
  tip.setCursorPosition(0);
  layout.addWidget(&tip, row++, 1, 1, 2);
  layout.setColumnStretch(2, 3);

  label = new QLabel(tr("Command"));
  label->setAlignment(Qt::AlignRight);
  layout.addWidget(label, row, 0);
  QLineEdit command(mCommands.at(id));
  //QTextEdit command(mCommands.at(id));
  command.setCursorPosition(0);
  command.setToolTip(tr("Available Tags are: %1").arg("[Provider] [Symbol] [Market] "
                                                      "[FiId] [MarketId]"));
  layout.addWidget(&command, row++, 1, 1, 2); // Spawn over two colums...
//   layout.setColumnStretch(2, 2);              // ...and take more space

  label = new QLabel(tr("Symbol Type"));
  label->setAlignment(Qt::AlignRight);
  layout.addWidget(label, row, 0);
//   QLineEdit symbolType(mSymbolTypes.at(id));
  QComboBox symbolType;
  symbolType.setToolTip(tr("Witch type has to be [Symbol]. When empty is called once with any symbol\n"
                           "(You should not use [Symbol] in this case at the command)"));
  SymbolTypeTuple* st = mFilu->getSymbolTypes(Filu::eAllTypes);
  if(st)
  {
    while(st->next()) symbolType.addItem(st->caption());
  }

  symbolType.addItem("");
  symbolType.setCurrentIndex(symbolType.findText(mSymbolTypes.at(id)));

  layout.addWidget(&symbolType, row, 1);

  QCheckBox allMarkets(tr("All Markets"));
  allMarkets.setToolTip(tr("Call multiple times with all markets by 'Symbol Type'"));
  allMarkets.setChecked(mMultis.at(id));
  layout.addWidget(&allMarkets, row++, 2);

  // Add an empty row to take unused space
  layout.addWidget(new QWidget, row, 1);
  layout.setRowStretch(row++, 2);

  // Build the button line
  QDialogButtonBox dlgBtns(QDialogButtonBox::Save | QDialogButtonBox::Discard);
  QPushButton* db = dlgBtns.button(QDialogButtonBox::Discard);
  dlgBtns.addButton(db, QDialogButtonBox::RejectRole);
  connect(&dlgBtns, SIGNAL(accepted()), &dialog, SLOT(accept()));
  connect(&dlgBtns, SIGNAL(rejected()), &dialog, SLOT(reject()));

  DialogButton* remove = new DialogButton(tr("&Remove"), -1);
  remove->setToolTip(tr("Remove button"));
  dlgBtns.addButton(remove, QDialogButtonBox::ActionRole);
  connect(remove, SIGNAL(clicked(int)), &dialog, SLOT(done(int)));

  DialogButton* add = new DialogButton(tr("&Add"), 2);
  add->setToolTip(tr("Copy to new button"));
  dlgBtns.addButton(add, QDialogButtonBox::ActionRole);
  connect(add, SIGNAL(clicked(int)), &dialog, SLOT(done(int)));

  layout.addWidget(&dlgBtns, row, 1, 1, 2);

  dialog.setWindowTitle(tr("LaunchPad - Edit button '%1'").arg(btn->text()));
  dialog.setMinimumWidth(350);

  switch (dialog.exec())
  {
    case 0:     // Discard
      return;
      break;
    case -1:    // Remove
    {
      int ret = QMessageBox::warning(&dialog
                  , tr("LaunchPad - Last chance to keep your data")
                  , tr("Are you sure to delete button <b>'%1'</b> with all your work<b>?</b>")
                      .arg(btn->text())
                  , QMessageBox::Yes | QMessageBox::No
                  , QMessageBox::No);

      if(ret == QMessageBox::No) return;

      deleteButton(btn);

      mCommands.removeAt(id);
      mSymbolTypes.removeAt(id);
      mMultis.removeAt(id);
      break;
    }
    case  1:    // Save
      setButtonName(btn, name.text());
      btn->setToolTip(tip.text());

      mCommands[id] = command.text();
      //mCommands[id] = command.toPlainText();
//       mSymbolTypes[id] = symbolType.text();
      mSymbolTypes[id] = symbolType.currentText();
      mMultis[id] = allMarkets.isChecked();
      break;
    case  2:    // Add
      btn = newButton(name.text());
      btn->setToolTip(tip.text());

      mCommands.append(command.text());
      //mCommands.append(command.toPlainText());
//       mSymbolTypes.append(symbolType.text());
      mSymbolTypes.append(symbolType.currentText());
      mMultis.append(allMarkets.isChecked());
      mButtons.setId(btn, mCommands.size() - 1);
      break;
  }

  saveSettings();
}

void LaunchPad::execCmd(const QString command, SymbolTuple* st)
{
  QString cmd = command;
  cmd.replace("[FiId]",     QString::number(st->fiId()));
  cmd.replace("[MarketId]", QString::number(st->marketId()));
  cmd.replace("[Provider]", st->owner());
  cmd.replace("[Symbol]",   st->caption());
  cmd.replace("[Market]",   st->market());

  QProcess::startDetached(cmd);
}

QToolButton* LaunchPad::addDummyButton()
{
  QToolButton* button = ButtonPad::addDummyButton();

  QString tt = cDummyTip;
  tt.append("\n");
  tt.append(button->toolTip());
  button->setToolTip(tt);
  button->setText("Echo");

  QString cmd("echo Dummy button clicked: "
              "SymbolType=[Provider] "
              "Symbol=[Symbol] Market=[Market] "
              "FiId=[FiId] MarketId=[MarketId]");

  mCommands.append(cmd);
  mSymbolTypes.append("");
  mMultis.append(true);

  return button;
}
