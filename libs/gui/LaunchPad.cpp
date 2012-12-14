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

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QSettings>
#include <QToolButton>

#include "LaunchPad.h"

#include "DialogButton.h"
#include "FiluU.h"
#include "SymbolTuple.h"

LaunchPad::LaunchPad(const QString& name, FClass* parent)
         : ButtonPad(name, parent, FUNC)
{}

LaunchPad::~LaunchPad()
{}

int LaunchPad::loadSettings()
{
  QSettings* settings = openSettings();

  int count = ButtonPad::loadSettings();

  // It's not a bug, it's a feature. We add an Dummy button if the file is empty
  for(int i = 0; i < count; ++i)
  {
    settings->beginGroup(QString::number(i));

    QString defaultCmd("echo Dummy button clicked: "
                       "SymbolType=[Provider] "
                       "Symbol=[Symbol] Market=[Market] "
                       "FiId=[FiId] MarketId=[MarketId]");

    mCommands.append(settings->value("Command", defaultCmd).toString());

    mSymbolTypes.append(settings->value("SymbolType", "").toString());
    mMultis.append(settings->value("AllMarkets", false).toBool());

    settings->endGroup();
  }

  closeSettings();

  return count;
}

int LaunchPad::saveSettings()
{
  QSettings* settings = openSettings();

  int count = ButtonPad::saveSettings();

  for(int i = 0; i < count; ++i)
  {
    settings->beginGroup(QString::number(i));

    settings->setValue("Command",    mCommands.at(i));
    settings->setValue("SymbolType", mSymbolTypes.at(i));
    settings->setValue("AllMarkets", mMultis.at(i));

    settings->endGroup();
  }

  closeSettings();

  return count;
}

void LaunchPad::newSelection(int fiId, int marketId)
{
  //qDebug() << "LaunchPad::newSelection:" << fiId << marketId;
  mFiId     = fiId;
  mMarketId = marketId;
}

void LaunchPad::buttonClicked(int id)
{
  //qDebug() << "LaunchPad::buttonClicked:" << mFiId << id << mCommands.at(id) << mSymbolTypes.at(id);

  if(!mFiId)
  {
    // Set some dummy values that we can call
    // the program anyway a FI was not selected
    SymbolTuple st(1);
    st.next();
    st.set("Foo", "NoMarket", "Reuters");
    execCmd(mCommands.at(id), &st);
    return;
  }

  SymbolTuple* st = mFilu->getSymbols(mFiId);

  if(!st) return;

  bool found = false;
  while(st->next())
  {
    if(mSymbolTypes.at(id).isEmpty())
    {
      // Exec cmd with each symbol to FI
      found = true;
      execCmd(mCommands.at(id), st);
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

//   if(!found)
//   {
//     qDebug() << "LaunchPad::buttonClicked: No symbol type"
//              << mSymbolTypes.at(id) << "found to FiId" << mFiId;
//   }

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
  layout.addWidget(&name, row++, 1);
  layout.setColumnStretch(1, 1);

  label = new QLabel(tr("Tool Tip"));
  label->setAlignment(Qt::AlignRight);
  layout.addWidget(label, row, 0);
  QLineEdit tip(btn->toolTip());
  layout.addWidget(&tip, row++, 1);

  label = new QLabel(tr("Command"));
  label->setAlignment(Qt::AlignRight);
  layout.addWidget(label, row, 0);
  QLineEdit command(mCommands.at(id));
  //QTextEdit command(mCommands.at(id));
  layout.addWidget(&command, row++, 1, 1, 2); // Spawn over two colums...
  layout.setColumnStretch(2, 2);              // ...and take more space

  label = new QLabel(tr("Symbol Type"));
  label->setAlignment(Qt::AlignRight);
  layout.addWidget(label, row, 0);
  QLineEdit symbolType(mSymbolTypes.at(id));
  layout.addWidget(&symbolType, row, 1);

  QCheckBox allMarkets(tr("All Markets"));
  allMarkets.setChecked(mMultis.at(id));
  layout.addWidget(&allMarkets, row++, 2);

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

      mCommands.removeAt(id);
      mSymbolTypes.removeAt(id);
      mMultis.removeAt(id);
      break;
    case  1:    // OK
      setButtonName(btn, name.text());
      btn->setToolTip(tip.text());

      mCommands[id] = command.text();
      //mCommands[id] = command.toPlainText();
      mSymbolTypes[id] = symbolType.text();
      mMultis[id] = allMarkets.isChecked();
      break;
    case  2:    // Add
      btn = newButton(name.text());
      btn->setToolTip(tip.text());

      mCommands.append(command.text());
      //mCommands.append(command.toPlainText());
      mSymbolTypes.append(symbolType.text());
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
