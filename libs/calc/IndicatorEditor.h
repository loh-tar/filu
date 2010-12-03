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

#ifndef INDICATOREDITOR_HPP
#define INDICATOREDITOR_HPP

#include "FWidget.h"

class IndicatorEditor : public FWidget
{
  Q_OBJECT

  public:
                  IndicatorEditor(FClass* parent);
   virtual       ~IndicatorEditor();

    void          includeText(const QString& txt);
    void          loadSettings();
    void          saveSettings();

  public slots:

  protected:
    QToolButton*  mButton;
    QComboBox*    mFileSelector;
    QTextEdit*    mEditor;

    QString       mIndicatorPath;
    QStringList   mAllFiles;

    void          readDir();
    void          readSettings();
    bool          eventFilter(QObject* pFilterObj, QEvent* pEvent);

  protected slots:
    void          loadFile();
    void          saveFile();
    void          editorLostFocus();
};

#endif
