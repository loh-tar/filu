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

#include "LogBookPage.h"

LogBookPage::LogBookPage(FClass* parent)
           : ManagerPage(parent)
{
  createPage();
}

LogBookPage::~LogBookPage()
{}

void LogBookPage::createPage()
{
  mIcon = QIcon(":/icons/logbook.png");
  mIconText = tr("Log Book");

  mTab = new QTabWidget;
  mTab->setTabPosition(QTabWidget::South);
  connect(mTab, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

  mTab->addTab(&mLogBook, tr("LogBook"));
  mTab->addTab(&mLogFile, tr("LogFile"));

  QGroupBox* pageGroup = new QGroupBox(tr("Log Book Overview"));

  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(mTab);

  pageGroup->setLayout(layout);

  QHBoxLayout* dummyLayout = new QHBoxLayout;
  dummyLayout->addWidget(pageGroup);

  setLayout(dummyLayout);

  mLogBook.setReadOnly(true);
  //mLogBook.setFontFamily("Monospace");
  mLogBook.setLineWrapMode(QTextEdit::NoWrap);

  mLogFile.setReadOnly(true);
  //mLogFile.setFontFamily("Monospace");
  mLogFile.setLineWrapMode(QTextEdit::NoWrap);

  mActClearLogFile = new QAction(tr("Clear File"), &mLogFile);
  connect(mActClearLogFile, SIGNAL(triggered(bool)), this, SLOT(clearLogFile()));

  mLogFile.setContextMenuPolicy(Qt::CustomContextMenu);
  connect(&mLogFile, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(showLogFileContextMenu(const QPoint&)));
}

void LogBookPage::addToLog(const QString& msg, const MsgType type)
{
  mLogBook.moveCursor(QTextCursor::End);
  if(type == eNotice) mLogBook.setTextBackgroundColor(QColor("white"));
  else mLogBook.setTextBackgroundColor(QColor("yellow"));
//   if(type == eWarning) mLogBook.setTextBackgroundColor(QColor("yellow"));
//   if(type == eError) mLogBook.setTextBackgroundColor(QColor("red"));

  mLogBook.insertPlainText(msg + "\n");

  mLogBook.ensureCursorVisible();
}

void LogBookPage::tabChanged(int index)
{
  switch(index)
  {
    case 0:
      break;

    case 1:
      readLogFile();
      break;
  }
}

void LogBookPage::readLogFile()
{
  QFile file(mRcFile->getST("LogFile"));

  if(!file.open(QIODevice::ReadWrite | QIODevice::Text))
  {
    mLogFile.clear();
    mLogFile.insertPlainText(tr("Can't open log file ") + mRcFile->getST("LogFile"));
    return;
  }

  // Read the log file
  QStringList messages;
  QTextStream in(&file);
  while (!in.atEnd())
  {
    QString line = in.readLine();
    messages.append(line);
  }

  if(messages.isEmpty()) messages << tr("LogFile is empty.");
  if(file.size() > 10000)
  {
    QString msg = tr("File is big, have a little patience...");
    mLogFile.clear();
    mLogFile.insertPlainText(msg);
    emit message(Q_FUNC_INFO, msg);
  }

  mLogFile.clear();
  mLogFile.insertPlainText(messages.join("\n"));
  mLogFile.moveCursor(QTextCursor::End);

  if(file.size() > 10000) emit message(Q_FUNC_INFO, tr("...Done."));

  file.close();
}

void LogBookPage::showLogFileContextMenu(const QPoint &p)
{
  QMenu *menu = mLogFile.createStandardContextMenu();

  // Prepend the clear action before all others
  QList<QAction*> al = menu->actions();
  QAction* sep = new QAction(menu);
  sep->setSeparator(true);
  menu->insertAction(al.at(0), sep);
  menu->insertAction(sep, mActClearLogFile);

  menu->exec(mLogFile.mapToGlobal(p));
  delete menu;
}

void LogBookPage::clearLogFile()
{
  if(QFile::remove(mRcFile->getST("LogFile")))
  {
    readLogFile();
  }
  else
  {
    addErrorText("LogBookPage::clearLogFile: " + tr("Can't remove LogFile"), eCritical);
    emit message(Q_FUNC_INFO, tr("Can't remove LogFile"));
  }

  emit message(Q_FUNC_INFO, tr("LogFile cleared."));
}
