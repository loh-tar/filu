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

#ifndef ADDFIPAGE_HPP
#define ADDFIPAGE_HPP

#include "ManagerPage.h"

class SearchField;
class Script;

class AddFiPage : public ManagerPage
{
  Q_OBJECT

  public:
                  AddFiPage(ManagerF* parent);
                  AddFiPage(FWidget* parent);
    virtual      ~AddFiPage();

  protected slots:
    void          search();
    void          selectResultRow( int row, int column);
    void          fillResultTable(QStringList* data);
    void          searchOrCancel();
    void          scriptFinished();
    void          addToDB();
    void          addToDBbyTWIB(QString psm, int row);

  protected:
    void          createPage();
    void          showEvent(QShowEvent * /*event*/);
    void          readSettings();

    void          searchFi();
    void          searchIdx();

    QComboBox*    mTypeSelector;
    QComboBox*    mProviderSelector;
    SearchField*  mSearchField;
    QTableWidget* mResultList;
    QPushButton*  mSearchCancelBtn;

    QPushButton*  mAddBtn;
    QLineEdit*    mAnySymbol;
    QLineEdit*    mName;
    QLineEdit*    mType;
    QLineEdit*    mSymbol;
    QComboBox*    mMarket;
    QComboBox*    mSymbolType;

    Script*       mScripter;

    QString       mProviderPath;
    QString       mProvider;
    QString       mDisplayType;
    bool          mNewQuery;

    QMultiHash<QString, int> mResultKeys;
};

class TWIB : public QWidget // Table-Widget-Install-Button
{
  Q_OBJECT

  public:
                  TWIB(const QString& txt, int row, QWidget* parent = 0);
    virtual      ~TWIB();

    QString       text();

    signals:
    void          requested(QString, int);

  protected:
    QLabel*       mField;
    QToolButton*  mButton;
    int           mRow;

  protected slots:
    void          clicked();
};

#endif
