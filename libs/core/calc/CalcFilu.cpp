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

#include <QSqlQuery>
#include <QVariant>

#include "CalcFilu.h"

#include "DataTupleSet.h"
#include "FiluU.h"

CalcFilu::CalcFilu(Indicator* parent)
        : CalcType(parent, FUNC)
{
  mType = "Filu";
}

CalcFilu::~CalcFilu()
{}

bool CalcFilu::prepare(CalcParms& parms)
{
  init(parms);

  mPrepareError = true;

  if(mIns.size() < 1)
  {
    error(FUNC, tr("No functionName found."));
    return false;
  }

  KeyVal info;
  int retVal = mFilu->getIndicatorInfo(&info, mIns.at(0));
  if(retVal !=  Filu::eData)
  {
    if(retVal == Filu::eNoData)
    {
      error(FUNC, tr("Unknown indicator '%1'.").arg(mIns.at(0)));
      return false;
    }
    error(FUNC, tr("Error while getting info to indicator '%1'.").arg(mIns.at(0)));
    addErrors(mFilu->errors());
    return false;
  }

  if(mFilu->prepareIndicator(mIns.at(0), info.value("Call")) != Filu::eSuccess)
  {
    error(FUNC, tr("Can't prepare indicator '%1'.").arg(mIns.at(0)));
    addErrors(mFilu->errors());
    return false;
  }

  // Count output variables
  mOutputCount = 0;
  for(int i = 1; ; ++i)
  {
    QString out = info.value("Output" + QString::number(i));
    if(out.isEmpty()) break;
    ++mOutputCount;
  }
  checkOutputCount(mOutputCount);

  // Get input parameters
  int idx = 1;       // Index pointer to mIns, looks like: "CCP", "FI", "(", ")", "foo"
  QStringList ins;   // Collect cleaned mIns
  ins << mIns.at(0); // Save the function name
  mInput.clear();
  for(int i = 1; ; ++i)
  {
    QString in = info.value("Input" + QString::number(i));
    if(in.isEmpty()) break;
    in = in.section(',', 0, 0);
    mInput << in;

    if(idx > mIns.size() - 1)
    {
      error(FUNC, tr("Too less input parameters."));
      return false;
    }
    // Veryfy if the call is correct filled
    if(in.startsWith("FiRef"))
    {
      if(!(mIns.at(idx) == "FI"))
      {
        error(FUNC, tr("FI() expected but found '%1'.").arg(mIns.at(idx)));
        return false;
      }
      idx += 2;
      if(idx > mIns.size() - 1)
      {
        error(FUNC, tr("Something wrong with parameter list."));
        return false;
      }
      if(mIns.at(idx) == ")")
      {
        mFiRefAlias << "THIS";
      }
      else
      {
        mFiRefAlias << mIns.at(idx);
        ++idx;
      }
    }
    else
    {
      checkInputVariable(idx);
      ins << mIns.at(idx);
    }

    ++idx;
  }
  mIns = ins;

  // Extract error messages for later use
  mIndiErrorText.clear();
  QString errTxt;
  for(int i = 1; ; ++i)
  {
    errTxt = info.value("Error" + QString::number(i));
    if(errTxt.isEmpty()) break;
    mIndiErrorText.append(errTxt);
  }

  if(hasError()) return false;

  mPrepareError = false;

  return true;
}

bool CalcFilu::calc()
{
  if(mPrepareError) return false;

  clearErrors();
  getIndicatorVariables();

  // Create our output variables
  for(int i = 0; i < mOuts.size(); ++i)
  {
    addToDataSet(mOuts.at(i));
  }

  // Set all input parameter
  int j = 0; // Count FiRefs
  int k = 1; // Count all other input variables
  for(int i = 0; i < mInput.size(); ++i)
  {
    if(mInput.at(i).startsWith("FiRef"))
    {
      int fiId;
      int marketId;
      bool ok = mData->getIDs(mFiRefAlias.at(j), fiId, marketId);
      if(!ok)
      {
        error(FUNC, tr("Can't find FI '%1'.").arg(mFiRefAlias.at(j)));
        return false;
      }
      ++j;

      // FIXME: Here we set all 4 values of FiRef anyway if used or not.
      //        Furthermore, is there a need to allow differend date ranges
      // in case we have more than one FiRef?
      // And last but not least: When we have more than one FiRef, then we have
      // to append :fiid etc with a number starting from 2 (no number for the
      // first FiRef please)

      // Parameter names must all in lowercase, restriction by Postgres
      mFilu->setSqlParm(":fiid", fiId);
      mFilu->setSqlParm(":marketid", marketId);

      QDate fdate;
      QDate tdate;
      mData->getDateRange(fdate, tdate);

      mFilu->setSqlParm(":fdate", fdate);
      mFilu->setSqlParm(":tdate", tdate);
    }
    else
    {
      // Any non FiRef parameter
      mFilu->setSqlParm(":" + mInput.at(i), mIns.at(k));
      ++k;
    }
  }

  // Aaaand...ACTION!
  QSqlQuery* query = mFilu->callIndicator(mIns.at(0));

  if(!query)
  {
    error(FUNC, tr("Trouble while exec Filu indicator '%1'.").arg(mIns.at(0)));
    addErrors(mFilu->errors());
    return false;
  }

  // Check if indicator returns an error
  query->next();
  if(query->value(0).toDate() == QDate(1000,1,1))
  {
    int errNo = query->value(2).toInt();
    if((mIndiErrorText.size() < errNo) or (mIndiErrorText.size() == 0))
    {
      error(FUNC, tr("Unknown ErrorNo '%1' from Filu indicator '%2'.").arg(errNo).arg(mIns.at(0)));
    }
    else
    {
      error(FUNC, mIndiErrorText.at(errNo - 1));
    }

    return false;
  }
  query->previous();

  QDate mdataDate;
  QDate resultDate;
  QList<double> lastQueryData;
  for(int i = 0; i < mOuts.size(); ++i) lastQueryData << 0.0;

  mData->rewind();
  resultDate = QDate(1000,1,1);
  while(mData->next())
  {
    mData->getDate(mdataDate);
    while(resultDate < mdataDate)
    {
      if(!query->next())
      {
        if(verboseLevel(eInfo))
        { // Looks so expensive, that's why we check self if verbose or not
          QString txt = "NOTICE! Data not in sync: EODBarDate=%1, IndicatorDate=%2, Now adjusted.";
          verbose(FUNC, txt.arg(mdataDate.toString(Qt::ISODate)).arg(resultDate.toString(Qt::ISODate)));
        }
        break;
      }
      resultDate = query->value(0).toDate();
    }

    if(resultDate > mdataDate)
    {
      // Fill all output variables
      for(int i = 0; i < mOuts.size(); ++i)
      {
        lastQueryData[i] = query->value(2).toDouble();
        mData->setValue(mOuts.at(i), lastQueryData[i]);
      }

      if(verboseLevel(eInfo))
      { // Looks so expensive, that's why we check self if verbose or not
        QString txt = "NOTICE! Data not in sync: EODBarDate=%1, IndicatorDate=%2, Now adjusted.";
        verbose(FUNC, txt.arg(mdataDate.toString(Qt::ISODate)).arg(resultDate.toString(Qt::ISODate)));
      }
      continue;
    }

    // Fill all output variables
    for(int i = 0; i < mOuts.size(); ++i)
    {
      lastQueryData[i] = query->value(2).toDouble();
      mData->setValue(mOuts.at(i), lastQueryData[i]);
    }
  }

  //FIXME: detect/set the correct values
  for(int i = 0; i < mOuts.size(); ++i)
    mData->setValidRange(mOuts.at(i), 0, mData->dataTupleSize());

  return true;
}

int CalcFilu::barsNeeded(DataTupleSet* /*data*/)
{
  return 1;
}
