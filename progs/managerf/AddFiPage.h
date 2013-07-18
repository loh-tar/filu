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

#ifndef ADDFIPAGE_HPP
#define ADDFIPAGE_HPP

#include <QButtonGroup>
#include <QLabel>
class QComboBox;
class QPushButton;
class QTableWidget;
class QToolButton;

#include "ManagerPage.h"
class Importer;
class LineEdit;
class Script;
class SearchField;

class PSMGrp : public QObject // Provider-Symbol-Market-Group
{
  Q_OBJECT

  public:
                    PSMGrp();
    virtual        ~PSMGrp();

    int             addOne();
    int             size();

    QComboBox*      provider(int i);
    LineEdit*       symbol(int i);
    QComboBox*      market(int i);
    QAbstractButton* searchBtn(int i);

  signals:
    void            searchCompBtnClicked(int);

  protected:
    QList<QComboBox*>     mProvider;
    QList<LineEdit*>      mSymbol;
    QList<QComboBox*>     mMarket;
    QButtonGroup          mBtnGrp; // Search Components Buttons

    int                   mCount;
};

class HitCountLabel : public QLabel
{

  public:
          HitCountLabel(QWidget* p) : QLabel(p)
          {
            setPrefix(tr("Found"));
            setCount(0);
            /*setFrameStyle(QFrame::Panel | QFrame::Sunken);*/
          };

    void  setPrefix(const QString& pf) { mPrefix = pf + ": "; };
    void  setCount(const int c)
          {
            setText(mPrefix + QString::number(c));
            mCount = c;
          };

    int  count() const { return mCount; };

  protected:
    QString mPrefix;
    int     mCount;
};

class AddFiPage : public ManagerPage
{
  Q_OBJECT

  public:
                  AddFiPage(FClass* parent);
    virtual      ~AddFiPage();

    void          loadSettings();
    void          saveSettings();

  protected slots:
    void          providerChanged(const QString& provider);
    void          providerFuncChanged(const QString& func);
    void          showProviderFuncInfo();
    void          search();
    void          insertRow();
    void          removeRow();
    void          selectResultRow( int row, int column);
    void          fillResultTable(QStringList* data);
    void          searchOrCancel();
    void          scriptFinished();
    void          searchCompBtnClicked(int idx);
    void          addToDB();
    void          addMarketToDB();
    void          addAllToDB();
    void          addToDBbyTWIB(QString psm, int row);

  protected:
    void          createPage();
    void          showEvent(QShowEvent* /*event*/);

    void          searchFi();
    void          searchIdx();
    void          searchMarket();
    bool          importFails(const QString& func, const QString& data);

    QComboBox*    mProviderFuncSelector;
    QComboBox*    mProviderSelector;
    SearchField*  mSearchField;
    QTableWidget* mResultList;
    QPushButton*  mSearchCancelBtn;
    HitCountLabel mHitCounter;

    LineEdit*     mRefSymbol;
    LineEdit*     mName;
    QComboBox*    mType;
    PSMGrp        mPSMGrp;

    Script*       mScripter;
    Importer*     mImporter;
    QString       mProviderPath;
    QString       mProvider;
    QString       mDisplayType;
    bool          mNewQuery;

    QMultiHash<QString, int> mResultKeys; // Only used by TWIB
    QStringList              mPreparedHeader;
    QHash<QString, QString>  mPreparedHeaderData;
    QHash<QString, QStringList> mProviderFunctions;
    QHash<QString, QHash<QString, QString> > mProviderFuncInfo;
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
