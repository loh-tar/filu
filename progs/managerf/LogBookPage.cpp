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

//#include <QtGui>

#include "LogBookPage.h"

LogBookPage::LogBookPage(ManagerF* parent) : ManagerPage(parent)
{
  createPage();
}

LogBookPage::LogBookPage(FWidget* parent) : ManagerPage(parent)
{
  createPage();
}

LogBookPage::~LogBookPage()
{}

void LogBookPage::createPage()
{
  mIcon = QIcon(":/icons/logbook.png");
  mIconText = tr("Log Book");

  QGroupBox* pageGroup = new QGroupBox(tr("Log Book"));

  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(&mLogBook);

  pageGroup->setLayout(layout);

  QHBoxLayout* dummyLayout = new QHBoxLayout;
  dummyLayout->addWidget(pageGroup);

  setLayout(dummyLayout);

  mLogBook.setReadOnly(true);
}

void LogBookPage::addToLog(const QString& msg, const bool error)
{
  mLogBook.moveCursor(QTextCursor::End);
  mLogBook.insertPlainText(msg + "\n");

  if(error) mLogBook.insertPlainText("\n");

  mLogBook.ensureCursorVisible();
}
