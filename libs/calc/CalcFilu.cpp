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

#include "CalcFilu.h"

CalcFilu::CalcFilu(Indicator* parent)
        : CalcType(parent)
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
    addErrorText("CalcFilu::prepare: No functionName found");
    return false;
  }

  KeyVal info;
  int retVal = mFilu->getIndicatorInfo(&info, mIns.at(0));
  if(retVal !=  Filu::eData)
  {
    if(retVal == Filu::eNoData)
    {
      addErrorText("CalcFilu::prepare: Unknown indicator: " + mIns.at(0));
      return false;
    }
    addErrorText("CalcFilu::prepare: Error while getting info to indicator: " + mIns.at(0));
    addErrorText(mFilu->errorText());
    return false;
  }

  if(mFilu->prepareIndicator(mIns.at(0), info.value("Call")) != Filu::eSuccess)
  {
    addErrorText("CalcFilu::prepare: Can't prepare indicator: " + mIns.at(0));
    addErrorText(mFilu->errorText());
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
  int idx = 1;       // index pointer to mIns, looks like: "CCP", "FI", "(", ")", "foo"
  QStringList ins;   // collect cleaned mIns
  ins << mIns.at(0); // save the function name
  mInput.clear();
  for(int i = 1; ; ++i)
  {
    QString in = info.value("Input" + QString::number(i));
    if(in.isEmpty()) break;
    in = in.section(',', 0, 0);
    mInput << in;

    if(idx > mIns.size() - 1)
    {
      addErrorText("CalcFilu::prepare: Too less input parameters.");
      return false;
    }
    // veryfy if the call is correct filled
    if(in.startsWith("FiRef"))
    {
      if(!(mIns.at(idx) == "FI"))
      {
        addErrorText("CalcFilu::prepare: FI() expected but found: " + mIns.at(idx));
        return false;
      }
      idx += 2;
      if(idx > mIns.size() - 1)
      {
        addErrorText("CalcFilu::prepare: Something wrong with parameter list.");
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

  // create our output variables
  for(int i = 0; i < mOuts.size(); ++i)
  {
      addToDataSet(mOuts.at(i));
  }

  // Set all input parameter
  int j = 0; // count FiRefs
  int k = 1; // count all other input variables
  for(int i = 0; i < mInput.size(); ++i)
  {
    if(mInput.at(i).startsWith("FiRef"))
    {
      int fiId;
      int marketId;
      bool ok = mData->getIDs(mFiRefAlias.at(j), fiId, marketId);
      if(!ok)
      {
        addErrorText("CalcFilu::calc: Can't find FI: " + mFiRefAlias.at(j));
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
    addErrorText("CalcFilu::calc: Trouble while exec Filu indicator: " + mIns.at(0));
    addErrorText(mFilu->errorText());
    return false;
  }

  // check if indicator returns an error
  query->next();
  if(query->value(0).toDate() == QDate(1000,1,1))
  {
    int errNo = query->value(2).toInt();
    if((mIndiErrorText.size() < errNo) or (mIndiErrorText.size() == 0))
    {
      addErrorText("Unknown ErrorNo " + QString::number(errNo) + " from Filu indicator " + mIns.at(0));
    }
    else
    {
      addErrorText(mIndiErrorText.at(errNo - 1));
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
        if(mDebugLevel)
        {
          qDebug() << "CalcFilu::calc: NOTICE! Data not in sync: EODBarDate=" << mdataDate
                   << ", IndicatorDate=" << resultDate << ", Now adjusted";
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
      if(mDebugLevel)
      {
        qDebug() << "CalcFilu::calc:: NOTICE! Data not in sync: EODBarDate=" << mdataDate
                 << ", IndicatorDate=" << resultDate << ", Now adjusted";
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
