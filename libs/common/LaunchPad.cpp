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

#include "LaunchPad.h"

LaunchPad::LaunchPad(const QString& name, FClass* parent)
         : FWidget(parent)
{
  mName   = name;
  mLayout = new QBoxLayout(QBoxLayout::LeftToRight);

  setLayout(mLayout);

  connect(&mButtons, SIGNAL(buttonClicked(int)), this, SLOT(buttonClicked(int)));
}

LaunchPad::~LaunchPad()
{}

void LaunchPad::loadSettings()
{
  QString filuHome = mRcFile->getST("FiluHome");

  QSettings settings(filuHome + "LaunchPads/" + mName + ".ini",  QSettings::IniFormat);

  // determine how many buttons we have
  QStringList buttonGroups = settings.childGroups();
  int count = buttonGroups.size();

  // It's not a bug, it's a feature. We add an Dummy button if the file is empty
  for(int i = 0; i < count; ++i)
  {
    settings.beginGroup(QString::number(i));

    QString name = settings.value("Name", "Dummy").toString();
    QToolButton* button = new QToolButton;
    button->setText(name);
    mButtons.addButton(button);
    mButtons.setId(button, i);

    mLayout->addWidget(button);

    QString defaultCmd("echo Dummy button clicked: "
                       "SymbolType=[Provider] "
                       "Symbol=[Symbol] Market=[Market] "
                       "FiId=[FiId] MarketId=[MarketId]");

    mCommands.append(settings.value("Command", defaultCmd).toString());

    mSymbolTypes.append(settings.value("SymbolType", "").toString());
    mMultis.append(settings.value("AllMarkets", false).toBool());

    settings.endGroup();
  }
}

void LaunchPad::saveSettings()
{
  QString filuHome = mRcFile->getST("FiluHome");

  QSettings settings(filuHome + "LaunchPads/" + mName + ".ini",  QSettings::IniFormat);

  QAbstractButton* btn;
  foreach(btn, mButtons.buttons())
  {
    int id = mButtons.id(btn);
    settings.beginGroup(QString::number(id));

    settings.setValue("Name",       btn->text());
    settings.setValue("Command",    mCommands.at(id));
    settings.setValue("SymbolType", mSymbolTypes.at(id));
    settings.setValue("AllMarkets", mMultis.at(id));

    settings.endGroup();
  }
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
    st.setMarket("NoMarket");
    st.setOwner("Reuters");
    st.setCaption("Foo");
    st.setFiId(0);
    st.setMarketId(1);
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
      // exec cmd with each symbol to FI
      found = true;
      execCmd(mCommands.at(id), st);
    }
    else if(st->owner() == mSymbolTypes.at(id))
    {
      if(mMultis.at(id))
      {
        // exec cmd with each market to symbol type
        found = true;
        execCmd(mCommands.at(id), st);
      }
      else if(st->marketId() == mMarketId)
      {
        // exec cmd with unique symbol type and market
        found = true;
        execCmd(mCommands.at(id), st);
        break;
      }
    }
  }

  if(!found)
  {
    qDebug() << "LaunchPad::buttonClicked: No symbol type"
             << mSymbolTypes.at(id) << "found to FiId" << mFiId;
  }

  delete st;
}

void  LaunchPad::execCmd(const QString command, SymbolTuple* st)
{
  QString cmd = command;
  cmd.replace("[FiId]",     QString::number(st->fiId()));
  cmd.replace("[MarketId]", QString::number(st->marketId()));
  cmd.replace("[Provider]", st->owner());
  cmd.replace("[Symbol]",   st->caption());
  cmd.replace("[Market]",   st->market());

  QProcess::startDetached(cmd);
}
