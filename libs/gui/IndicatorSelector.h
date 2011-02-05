//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011  loh.tar@googlemail.com
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

#ifndef INDICATORSELECTOR_HPP
#define INDICATORSELECTOR_HPP

#include "FWidget.h"

#include "TALib.h"

class SearchField;

class IndicatorSelector : public FWidget
{
  Q_OBJECT

  public:
                    IndicatorSelector(FClass* parent);
                   ~IndicatorSelector();

    void            loadSettings();
    void            saveSettings();

  signals:
    void            addText(const QString* txt);

  protected slots:
    void            refreshFunctionList();
    void            getFunctionInfo(QListWidgetItem* item);
    void            checkInfoSelection(int r);

    void            addToIndicator();
    void            inclToIndicator();

  protected:
    void            createPage();
    void            showEvent(QShowEvent* /*event*/);

    SearchField*    mSF;
    QListWidget*    mInfoList;
    QListWidget*    mFunctionList;
    QPushButton*    mAddBtn;
    QPushButton*    mInclBtn;
    QSplitter*      mSplitter;

    QStringList     mAllFunctions;
    QStringList     mTheIndicator;

    TALib           mTALib;
};

#endif
