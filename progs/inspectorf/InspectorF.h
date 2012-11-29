//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012  loh.tar@googlemail.com
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

#ifndef INSPECTORF_HPP
#define INSPECTORF_HPP

#include <QComboBox>
#include <QDateEdit>
#include <QSqlQueryModel>
class QProgressBar;
class QPushButton;

#include "FWidget.h"

#include "SqlTableView.h"
#include "StrategyEditor.h"
class Trader;
class BackTester;

class InspectorF : public FMainApp
{
  Q_OBJECT

  enum SomeEnums
  {
    eRule,
    eIndicator
  };

  public:
                    InspectorF(QApplication& app);
    virtual        ~InspectorF();

  protected slots:
    void            runTest();
    void            loadRule(const QString& fileName);
    void            tabChanged(int = 0);
    void            edited();
    void            loopsNeedet(int);
    void            loopDone(int);
    void            newData();
    void            resultSelected(const QModelIndex& index);
    void            backTestError();

  protected:
    void            init();
    void            readSettings();
    void            saveSettings();

    BackTester*     mBackTester;

    QTabWidget*     mTabWidget;
    QProgressBar*   mProgessBar;
    QPushButton*    mTestButton;

    QComboBox       mTradingRuleName;

    Trader*         mTrader;
    QStringList     mOrigRule;
    QStringList     mOrigIndicator;
    QStringList     mTestRule;
    QStringList     mTestIndicator;

    StrategyEditor  mEditor;
    StrategyEditor  mDisplay;
    StrategyEditor  mDetailDisplay;
    bool            mEdited;

    QRegExp         mConstMatcher;
    QRegExp         mEditedMatcher;
    QRegExp         mErrorMatcher;

    QDateEdit       mFromDate;
    QDateEdit       mToDate;

    QTextEdit       mReport;

    SqlTableView    mResultsView;
    SqlTableView    mPerformanceView;
    SqlTableView    mScoreView;
    SqlTableView    mDetailView;

    QSqlQueryModel  mResults;
    QSqlQueryModel  mPerformance;
    QSqlQueryModel  mScore;
    QSqlQueryModel  mDetail;

    QString         mSelectedStrategyId;
    QString         mShowingStrategyId;

    QHash<const QString, bool> mNewData; // Saves for each result tab if he have to load new data

    QString         mTradingRulePath;
    QString         mIndicatorPath;
};

#endif
