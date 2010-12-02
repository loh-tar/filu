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

#include "IndicatorEditor.h"

IndicatorEditor::IndicatorEditor(FClass* parent)
                : FWidget(parent)
{
  mButton = new QToolButton;
  mButton->setAutoRaise(true);
  mButton->setArrowType(Qt::DownArrow);
  connect(mButton, SIGNAL(clicked()), this, SLOT(saveFile()));

  mFileSelector = new QComboBox;
  mFileSelector->setEditable(true);
  connect(mFileSelector, SIGNAL(activated(int)), this, SLOT(loadFile()));

  QHBoxLayout* layout1 = new QHBoxLayout;
  layout1->addWidget(mButton);
  layout1->addWidget(mFileSelector);

  mEditor = new QTextEdit;
  mEditor->setLineWrapMode(QTextEdit::NoWrap);
  mEditor->installEventFilter(this);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setMargin(0);
  mainLayout->addLayout(layout1);
  mainLayout->addWidget(mEditor);

  setLayout(mainLayout);

  readSettings();
  readDir();
}

IndicatorEditor::~IndicatorEditor()
{
}

void IndicatorEditor::includeText(const QString& txt)
{
  QTextCursor cur = mEditor->textCursor();
  cur.clearSelection();
  cur.movePosition(QTextCursor::StartOfLine);
  cur.insertText(txt);
}

/***********************************************************************
*
*                           Private  Stuff
*
************************************************************************/
void IndicatorEditor::loadFile()
{
  QString fileName = mFileSelector->currentText();

  QFile file(mIndicatorPath + fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QMessageBox::critical(this, tr("Indicator Editor"),
                         tr("\nCan't load file\t"),
                         QMessageBox::Close);
    return;
  }

  // Read the indicator
  QStringList indicator;
  QTextStream in(&file);
  while (!in.atEnd())
  {
    QString line = in.readLine();
    indicator.append(line);
  }

  file.close();

  mEditor->clear();
  mEditor->insertPlainText(indicator.join("\n"));
}

void IndicatorEditor::saveFile()
{
  QString fileName = mFileSelector->currentText();

  if(mAllFiles.contains(fileName))
  {
     int ret = QMessageBox::question(this, tr("Indicator Editor"),
                   tr("\nOverwrite existing file?\t"),
                   QMessageBox::Save | QMessageBox::Cancel);

    if(ret != QMessageBox::Save)
    {
      mFileSelector->setFocus();
      return;
    }
  }

  QFile file(mIndicatorPath + fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    QMessageBox::critical(this, tr("Indicator Editor"),
                         tr("\nCan't save file\t"),
                         QMessageBox::Close);
    return;
  }

  file.write(mEditor->toPlainText().toAscii() + '\n');

  file.close();
}

void IndicatorEditor::readDir()
{
  QDir dir(mIndicatorPath);
  mAllFiles = dir.entryList(QDir::Files, QDir::Name);

  mFileSelector->insertItems(0, mAllFiles);
}

void IndicatorEditor::readSettings()
{
  mIndicatorPath = mRcFile->getST("IndicatorPath");
}

/***********************************************************************
*
*                         event handler
*
************************************************************************/
void IndicatorEditor::editorLostFocus()
{
  QTextCursor cur = mEditor->textCursor();
  cur.select(QTextCursor::LineUnderCursor);

  mEditor->setTextCursor(cur);
}

// Code found at qtforum.de, thanks guys
bool IndicatorEditor::eventFilter(QObject* pFilterObj, QEvent* pEvent)
{
  if((pFilterObj == mEditor) && (pEvent->type() == QEvent::FocusOut))
  {
    editorLostFocus();
  }

  return QWidget::eventFilter(pFilterObj, pEvent);
}
