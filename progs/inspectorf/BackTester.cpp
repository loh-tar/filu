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

#include <QtCore>
#include <QDebug>

#include "BackTester.h"

BackTester::BackTester()
          : QThread()
          , FClass("BackTester")
{
  init();
}

BackTester::~BackTester()
{
//   qDebug() << "~BackTester() go die";
  mMutex.lock();
  mRun = false;
  mGoAndDie = true;
//   qDebug() << "~BackTester() call wakeOne";
  mWaitCondition.wakeOne();
  mMutex.unlock();
//    qDebug() << "~BackTester() wait for termination";
  wait();
  qDebug() << "I'm dead";
}

void BackTester::init()
{
  //qDebug() << "BackTester::init()";

  mTrader = new Trader(this);

  mConstMatcher.setPattern("(\\b\\d+\\.?\\d*)");
  mEditedMatcher.setPattern("\\{([\\d\\.\\-\\i\\;\\s]+)\\}");
  //mErrorMatcher.setPattern();
  mNewJob   = false;
  mRun      = false;
  mGoAndDie = false;
}

void BackTester::prepare(const QString& rule,
                         const QString& indicator,
                         const QDate& fdate, const QDate tdate)
{
  //qDebug() << "BackTester::prepare()";
  QMutexLocker locker(&mMutex);

  mOrigIndicator = indicator;
  mOrigRule = rule;
  mFromDate = fdate;
  mToDate   = tdate;
  mNewJob   = true;
  mRun      = false;

  if(!isRunning())
  {
//    qDebug() << "BackTester::prepare: not running";
    start(LowPriority);
  }
  else
  {
//    qDebug() << "BackTester::prepare: wake it up";
    mWaitCondition.wakeOne();
  }

}

void BackTester::calc()
{
  QMutexLocker locker(&mMutex);

  mRun = true;

  if(!isRunning())
  {
    start(LowPriority);
  }
  else
  {
    mWaitCondition.wakeOne();
  }
}

void BackTester::run()
{
//  qDebug() << "BackTester::run: I start";

  forever
  {
    mMutex.lock();
//    qDebug() << "BackTester::run: nach forever mutex";
    if(mNewJob)
    {
      qDebug() << "BackTester::run: aha, new job";
      mMutex.unlock();
      if(!detectConstants())
      {
        emit error();
      }
    }
    else if(mRun)
    {
      qDebug() << "BackTester::run: I go do my job";
      mMutex.unlock();
      if(!backtest())
      {
        emit error();
      }
    }
    else if(mGoAndDie)
    {
      mMutex.unlock();
      return;
    }
    else if(!mRun)
    {
     qDebug() << "BackTester::run: I go sleep";
      mWaitCondition.wait(&mMutex);
     qDebug() << "BackTester::run: wake up";
      mMutex.unlock();
    }

    //qDebug() << "BackTester::run() end of forever";

  }

}

bool BackTester::backtest()
{
  clearErrors();

  // init our control variables to start from first combination
  mIdx.clear();
  mOneMoreLoop.clear();
  for(int i = 0; i < mConst.size(); ++i)
  {
    mIdx.append(0);
    mOneMoreLoop.append(true);
  }

  int loop = 0;
  do
  {
    mMutex.lock();

    if(!mRun) break;

    buildFiles();
    buildStrategyId();

    mTsId = mFilu->addTradingStrategy( mStrategyId,
                                       mFromDate.toString(Qt::ISODate),
                                       mToDate.toString(Qt::ISODate),
                                       mTestRule.join("\n"),
                                       mTestIndicator.join("\n") );

    if(!mTrader->useRule(mTestRule))
    {
      addErrorText(mTrader->errorText());
      return false;
    }

    if(!mTrader->useIndicator(mTestIndicator))
    {
      addErrorText(mTrader->errorText());
      return false;
    }


    //
    // run the simulation for each Fi in the group used by rule file
    int fiCount = mTrader->prepare(mFromDate, mToDate);

    if(fiCount == -1)
    {
      addErrorText(mTrader->errorText());
      mRun = false;
      mMutex.unlock();
      return false;
    }

    mMutex.unlock();

     //int totalLoopsNeeded = mLoopsNeeded * fiCount;

    // here is the beef
    while(int ret = mTrader->simulateNext())
    {
      ++loop;
      if(ret == 2)
      {
        addErrorText(mTrader->errorText());
        continue; // any problem while simulation, but go ahead
      }

      QList<QStringList> report;
      mTrader->getReport(report);
      buildReport(report);
      emit loopDone(loop);
    }

    emit strategyDone();

    // move the pointers
    for(int i = 0; i < mConst.size(); ++i)
    {
      ++mIdx[i];
      if(mIdx.at(i) == mConst.at(i).size())
      {
        mIdx[i] = 0;
        mOneMoreLoop[i] = false;
      }
      else
      {
        break;
      }
    }

  }while(mOneMoreLoop.contains(true));

  mMutex.lock();
  mRun = false;
  mMutex.unlock();

  return true;
}


bool BackTester::detectConstants()
{
  mMutex.lock();
  mNewJob = false;
  mMutex.unlock();

  clearErrors();

  //
  // detect the changing constants

  mConst.clear();
  mCPos.clear();
  mCType.clear();

  // we start with the rule file
  int pos = 0;
  while ((pos = mEditedMatcher.indexIn(mOrigRule, pos)) != -1)
  {
    mCPos.append(pos);

    pos += mEditedMatcher.matchedLength();

    QStringList cl;
    buildConstants(mEditedMatcher.cap(1), cl);

    mConst.append(cl);
    mCType.append(eRule);
  }

  // all again for the indicator file
  pos = 0;
  while ((pos = mEditedMatcher.indexIn(mOrigIndicator, pos)) != -1)
  {
    mCPos.append(pos);
    pos += mEditedMatcher.matchedLength();
    QStringList cl;
    buildConstants(mEditedMatcher.cap(1), cl);
    mConst.append(cl);
    mCType.append(eIndicator);
  }
 qDebug() << "BackTester::detectConstants: constants to change" << mConst;

  mLoopsNeeded = 1;
  for(int i = 0; i < mConst.size(); ++i)
  {
    mLoopsNeeded *= mConst.at(i).size();
    //qDebug() << "Constant" << i << mLoopsNeeded << mConst.at(i).size();
  }

  mIdx.clear();
  for(int i = 0; i < mConst.size(); ++i)
  {
    mIdx.append(0);
  }

  buildFiles();

  if(!mTrader->useRule(mTestRule))
  {
    addErrorText(mTrader->errorText());
    return false;
  }

  if(!mTrader->useIndicator(mTestIndicator))
  {
    addErrorText(mTrader->errorText());
    return false;
  }

  int fiCount = mTrader->prepare(mFromDate, mToDate);

  if(fiCount == -1)
  {
    addErrorText(mTrader->errorText());
    return false;
  }

  emit loopsNeedet(mLoopsNeeded * fiCount);

//   mMutex.lock();
//   mNewJob = false;
//   mMutex.unlock();

  return true;
}

void BackTester::buildConstants(const QString& constExp, QStringList& constList)
{
  // constExp looks like
  // "10-100 i 10" or "10; 20; 30"
  if(constExp.contains("-") and constExp.contains("i"))
  {
    QStringList parts = constExp.split(QRegExp("[\\-\\i]"));
    parts.replaceInStrings(" ", "");
    if(parts.size() < 3)
    {
      constList.append("??? " + constExp);
      return;
    }

    double from = parts[0].toDouble();
    double to = parts[1].toDouble();
    double i = parts[2].toDouble();
    if(i == 0)
    {
      constList.append("??? " + constExp);
      return;
    }

    while(from <= to)
    {
      constList.append(QString::number(from));
      from += i;
    }

    return;
  }

  if(constExp.contains(";"))
  {
    constList = constExp.split(";");
    constList.replaceInStrings(" ", "");

    return;
  }

  constList.append("??? " + constExp);
}

void BackTester::buildFiles()
{
  // create the rule und indicator files with a new combination
  // of constant setting

  // take a fresh copy of the prepared files
  QString rule = mOrigRule;
  QString indi = mOrigIndicator;

  // because by the replacements the positions of the constants
  // are moving, we have have to pay attention of these
  int adjustRule = 0;
  int adjustIndi = 0;

  for(int i = 0; i < mConst.size(); ++i)
  {
    // replace in the files the constants
    if(mCType.at(i) == eRule)
    {
      int pos = mEditedMatcher.indexIn(rule, mCPos.at(i) - adjustRule);
      int length = mEditedMatcher.matchedLength();
      adjustRule += length - mConst.at(i).at(mIdx.at(i)).size();
      rule.replace(pos, length, mConst.at(i).at(mIdx.at(i)));
      //qDebug() << "rule i:j" << i << mIdx.at(i) << mConst.at(i).at(mIdx.at(i)) << mCPos.at(i);
    }
    else
    {
      int pos = mEditedMatcher.indexIn(indi, mCPos.at(i) - adjustIndi);
      int length = mEditedMatcher.matchedLength();
      adjustIndi += length - mConst.at(i).at(mIdx.at(i)).size();
      indi.replace(pos, length, mConst.at(i).at(mIdx.at(i)));
      //qDebug() << "indi i:j" << i << mIdx.at(i) << mConst.at(i).at(mIdx.at(i)) << mCPos.at(i);
    }
  }

  mTestRule = rule.split("\n");
  mTestIndicator = indi.split("\n");
}

void BackTester::buildStrategyId()
{
  QCryptographicHash idGenerator(QCryptographicHash::Md4);

  for(int i = 0; i < mTestRule.size(); ++i)
  {
    QString line = mTestRule.at(i);
    line.remove(" ");
    if(line.startsWith("*")) continue;
    idGenerator.addData(line.toUtf8());
  }

  for(int i = 0; i < mTestIndicator.size(); ++i)
  {
    QString line = mTestIndicator.at(i);
    line.remove(" ");
    if(line.startsWith("*")) continue;
    idGenerator.addData(line.toUtf8());
  }

  mStrategyId = idGenerator.result().toHex();
  //qDebug() << "BackTester::buildStrategyId() " << mStrategyId;
}

void BackTester::buildReport(QList<QStringList> &report)
{
  int i = 0;
  // fast forward to the score board, skip the order part
  for(; i < report.size(); ++i)
  {
    if(report.at(i).at(0) == ("[Score]")) break;
  }
  ++i; // skip the "[Score]" line

  // fetch the points who are interesting
  int fiId, marketId;
  double score, pWL, pLL, pAGL, pALL, pTP;
  for(; i < report.size(); ++i)
  {
    if(report.at(i).at(2) == ("FIID"))  fiId = report.at(i).at(1).toInt();
    if(report.at(i).at(2) == ("MI"))    marketId = report.at(i).at(1).toInt();
    if(report.at(i).at(2) == ("pWL"))   pWL   = report.at(i).at(1).toDouble();
    if(report.at(i).at(2) == ("pLL"))   pLL   = report.at(i).at(1).toDouble();
    if(report.at(i).at(2) == ("pAGL"))  pAGL  = report.at(i).at(1).toDouble();
    if(report.at(i).at(2) == ("pALL"))  pALL  = report.at(i).at(1).toDouble();
    if(report.at(i).at(2) == ("pTP"))   pTP   = report.at(i).at(1).toDouble();
    if(report.at(i).at(2) == ("SCORE")) score = report.at(i).at(1).toDouble();
    //if(report.at(i).at(2) == (""))  = report.at(i).at(1).toDouble();
  }

  // write to the DB
  mFilu->addTradingResult(mTsId, fiId, marketId, pWL, pLL, pAGL, pALL, pTP, score);
}
