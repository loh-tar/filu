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

#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QTextEdit>
#include <QTextStream>
#include <QToolButton>
#include <QVBoxLayout>

#include "IndicatorEditor.h"

#include "RcFile.h"

IndicatorEditor::IndicatorEditor(FClass* parent)
               : FWidget(parent, FUNC)
               , mFileNameChanged(false)
{
  QToolButton*  saveBtn;
  saveBtn = new QToolButton;
  saveBtn->setToolTip(tr("Save File"));
  saveBtn->setAutoRaise(true);
  saveBtn->setIcon(QIcon::fromTheme("document-save"));
  saveBtn->setShortcut(QKeySequence(QKeySequence::Save));
  connect(saveBtn, SIGNAL(clicked()), this, SLOT(saveFile()));

  QToolButton*  delBtn;
  delBtn = new QToolButton;
  delBtn->setToolTip(tr("Delete File"));
  delBtn->setAutoRaise(true);
  delBtn->setIcon(QIcon::fromTheme("edit-delete"));
  delBtn->setShortcut(QKeySequence("Ctrl+Del"));
  connect(delBtn, SIGNAL(clicked()), this, SLOT(deleFile()));


  mFileSelector = new QComboBox;
  mFileSelector->setEditable(true);
  connect(mFileSelector, SIGNAL(activated(const QString&)), this, SLOT(loadFile(const QString&)));
  connect(mFileSelector, SIGNAL(editTextChanged(const QString&)), this, SLOT(fileNameChanged(const QString&)));

  QHBoxLayout* layout1 = new QHBoxLayout;
  layout1->addWidget(saveBtn);
  layout1->addWidget(mFileSelector);
  layout1->addWidget(delBtn);
  layout1->addSpacing(20);

  mEditor = new QTextEdit;
  mEditor->setFontFamily("Monospace");
  mEditor->setLineWrapMode(QTextEdit::NoWrap);
  mEditor->setCursorWidth(2);
  mEditor->installEventFilter(this);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->setMargin(0);
  mainLayout->addLayout(layout1);
  mainLayout->addWidget(mEditor);

  setLayout(mainLayout);
  setFocusProxy(mFileSelector);

  mIndicatorPath = mRcFile->getST("IndicatorPath");
  readDir();
}

IndicatorEditor::~IndicatorEditor()
{
}

void IndicatorEditor::loadSettings()
{
  mRcFile->beginGroup("IndicatorEditor");

  if(loadFile(mRcFile->getST("LastEditFile")))
  {
    QTextCursor cur = mEditor->textCursor();
    cur.setPosition(mRcFile->getIT("CursorPos"));
    mEditor->setTextCursor(cur);
    mEditor->ensureCursorVisible();
//     mEditor->setFocus(); // FIXME:Does not work...
    editorLostFocus();   // ...as workaround
  }

  mRcFile->endGroup();
}

void IndicatorEditor::saveSettings()
{
  mRcFile->beginGroup("IndicatorEditor");
  mRcFile->setValue("LastEditFile", mFileSelector->lineEdit()->text());
  mRcFile->setValue("CursorPos", mEditor->textCursor().position());
  mRcFile->endGroup();
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
bool IndicatorEditor::loadFile(const QString& fileName)
{
  if(mEditor->document()->isModified())
  {
    QString question;
    if(mFileNameChanged)
    {
      mFileSelector->blockSignals(true);
      mFileSelector->setEditText(mCurrentFile);
      question = QString(tr("\nSave new file '%1'?\t").arg(mCurrentFile));
    }
    else
    {
      // Don't confuse the user, show the still valid file name
      mFileSelector->setCurrentIndex(mFileSelector->findText(mCurrentFile));
      question = QString(tr("\nFile '%1' was changed.\t").arg(mCurrentFile));
    }

    int ret = QMessageBox::warning(this, tr("Indicator Editor"), question
                , QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
                , QMessageBox::Cancel);

    mFileSelector->blockSignals(false);

    if(ret == QMessageBox::Save)
    {
      if(!saveFile()) return false;
    }
    else if(ret == QMessageBox::Cancel)
    {
      return false;
    }
  }

  mFileSelector->setCurrentIndex(mFileSelector->findText(fileName));

  QFile file(mIndicatorPath + fileName);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QMessageBox::critical(this, tr("Indicator Editor")
                   , tr("\nCan't load file '%1'.\t").arg(file.fileName())
                   , QMessageBox::Close);
    return false;
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

  mEditor->setPlainText(indicator.join("\n"));
  mEditor->document()->setModified(false);
  mCurrentFile = fileName;
  mFileNameChanged = false;

  return true;
}

bool IndicatorEditor::saveFile()
{
  if(mCurrentFile.isEmpty()) return false;

  QFile file(mIndicatorPath + mCurrentFile);

  bool newFile = false;

  if(!file.exists()) newFile = true;

  if(mFileNameChanged and !newFile)
  {
     mFileSelector->blockSignals(true); // Don't try to load new file because was changed

     int ret = QMessageBox::question(this, tr("Indicator Editor")
                   , tr("\nOverwrite existing file '%1'?\t").arg(mCurrentFile)
                   , QMessageBox::Save | QMessageBox::Cancel);

    mFileSelector->blockSignals(false);

    if(ret != QMessageBox::Save)
    {
      mFileSelector->setFocus();
      return false;
    }
  }

  if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    QMessageBox::critical(this, tr("Indicator Editor")
                         , tr("\nCan't save file '%1.'\t").arg(mCurrentFile)
                         , QMessageBox::Close);
    return false;
  }

  file.write(mEditor->toPlainText().toUtf8() + '\n');
  file.close();

  mEditor->document()->setModified(false);
  mFileNameChanged = false;

  if(newFile)
  {
    readDir();
    mFileSelector->setCurrentIndex(mFileSelector->findText(mCurrentFile));
  }

  return true;
}

bool IndicatorEditor::deleFile()
{
  // FIXME Disable delButton if fileName is empty or edited
  if(mCurrentFile.isEmpty()) return false;

  QFile file(mIndicatorPath + mCurrentFile);

  if(!file.exists())  return false;

  int ret = QMessageBox::warning(this, tr("Indicator Editor")
              , tr("\nDelete file '%1'?\t").arg(mCurrentFile)
              , QMessageBox::Yes | QMessageBox::Cancel
              , QMessageBox::Cancel);

  if(ret != QMessageBox::Yes) return false;

  if(file.remove())
  {
    int idx = mFileSelector->currentIndex();
    readDir();
    if(idx > mFileSelector->count() - 1 and idx > 0) --idx;
    mFileSelector->setCurrentIndex(idx);
    mEditor->document()->setModified(false);
    loadFile(mFileSelector->currentText());

    return true;
  }

  QMessageBox::critical(this, tr("Indicator Editor")
                    , tr("\nCan't delete file '%1'.\t").arg(mCurrentFile)
                    , QMessageBox::Close);
  return false;
}

void IndicatorEditor::fileNameChanged(const QString& newName)
{
  // Set when name was edited but not when only a different was selected.
  if(mFileSelector->lineEdit()->isModified())
  {
    mFileNameChanged = true;
    mCurrentFile = newName;
  }
}

void IndicatorEditor::readDir()
{
  QDir dir(mIndicatorPath);
  mFileSelector->clear();
  mFileSelector->insertItems(0, dir.entryList(QDir::Files, QDir::Name));
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
