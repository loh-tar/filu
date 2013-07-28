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

#include <QListWidgetItem>

#include "ManagerPage.h"

ManagerPage::ManagerPage(FClass* parent, const QString& className)
           : FWidget(parent, className)
           , mFocusWidget(0)
{
  //createPage();
}

ManagerPage::~ManagerPage()
{}

/*
void  ManagerPage::createPage()
{

}
*/

void ManagerPage::loadSettings()
{
  //qDebug() << "ManagerPage::loadSettings()" << mIconText;
}

void ManagerPage::saveSettings()
{
  //qDebug() << "ManagerPage::saveSettings()" << mIconText;
}

void ManagerPage::setPageIcon(QListWidgetItem* button)
{
  //qDebug() << "ManagerPage::setPageIcon(QListWidgetItem* button)";
  button->setIcon(mIcon);
  button->setText(mIconText);
  button->setTextAlignment(Qt::AlignHCenter);
  button->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  button->setSizeHint(QSize(100, 70));
}

bool ManagerPage::sadFilu(const QString& func, const QString& txt/* = ""*/, const MsgType type/* = eError*/)
{
  if(check4FiluError(func, txt, type))
  {
    emitMessage(func, txt + "\n" + tr("For details see LogFile"), type);
    return true;
  }

  return false;
}

void ManagerPage::setFocusWidget(QWidget* fw)
{
  mFocusWidget = fw;
  if(mFocusWidget) mFocusWidget->setFocus();
}

void ManagerPage::hideEvent(QHideEvent* /*event*/)
{
  QWidget* fw = focusWidget();
  if(fw) mFocusWidget = fw;
}

void ManagerPage::showEvent(QShowEvent* /*event*/)
{
  if(mFocusWidget) mFocusWidget->setFocus();
}
