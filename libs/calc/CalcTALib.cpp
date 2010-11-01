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

#include "CalcTALib.h"

CalcTALib::CalcTALib(Indicator* parent) : CalcType(parent)
{
  mType = "TALib";
}

CalcTALib::~CalcTALib()
{}

bool CalcTALib::prepare(CalcParms& parms)
{
  init(parms);

  if(mIns.size() < 1)
  {
    addErrorText("CalcTALib::prepare: No functionName found");
    return false;
  }

  TALib talib;

  if(talib.functionIsUnknown(mIns.at(0)))
  {
    addErrorText("CalcTALib::prepare: Unknown TA function: "
                          + mIns.at(0));
    return false;
  }

  // analyse all parameters, 1st build input list...
  mInput.clear();

  // get the info
  talib.getFunctionInfo(mIns.at(0), mInfo);

  if(mOuts.size() < mInfo.value("nbOutput").toInt())
  {
    addErrorText("CalcTALib::prepare: Too less output variables, function '"
                        + mIns.at(0) + "' expect " + mInfo.value("nbOutput").toString());
    return false;
  }

  int firstOptInput = 1;
  int nbInput = mInfo.value("nbInput").toInt();

  if(!checkInputCount(nbInput + 1)) return false;

  for(int i = 0; i < nbInput; ++i)
  {
    QString key = "InputType" + QString::number(i + 1);

    if(mInfo.value(key) == "EODBAR")
    {
      QStringList input = mInfo.value("Input").toString().split(", ");
      key.append("-");
      if(mIns.at(1) == "FI")
      {
        // mIns looks like: "AD", "FI" "(", "alias", ")"
        if(mIns.size() < 4)
        {
          addErrorText("CalcTALib::prepare: Missing ) at FI()");
          return false;
        }

        QString alias = mIns.at(3);
        if(alias == ")") alias = "";
        else
        {
          alias.append(".");
          ++firstOptInput;
        }

        for(int j = 0; j < input.size(); ++j)
        {
          QString key2 = key + QString::number(j);
          mInput.insert(key2, alias + mInfo.value(key2).toString());
        }

        firstOptInput += 3;
      }
      else
      {
        // mIns looks like: "AD", "HIGH", "LOW", "CLOSE", "VOLUME"
        if(!checkInputCount(input.size() + 1)) return false;

        for(int j = 0; j < input.size(); ++j)
        {
          bool isNumber;
          mIns.at(1 + j).toDouble(&isNumber);
          if((mIns.at(1 + j) == "=") or isNumber)
          {
            addErrorText("CalcTALib::prepare: Not enough input variables:\n\t"
                         "Found to early an optional input.");
            return false;
          }

          mInput.insert(key + QString::number(j), mIns.at(1 + j));
          ++firstOptInput;
        }
      }
    }
    else // no EODBAR InputType
    {
      mInput.insert(key, mIns.at(1 + i));
      ++firstOptInput;
    }
  }

  // check if all input variables are known
  QHashIterator<QString, QString> hit(mInput);
  while(hit.hasNext())
  {
    QString var =  hit.next().value();
    if(isUnknown(var))
    {
      addErrorText("CalcTALib::prepare: Unknown input variable: " + var);
      return false;
    }
  }

  // ...2nd build optional input list
  // mIns looks like: "ADOSC", "FI", "(", ")", "FastPeriod", "=", "3", "SlowPeriod", "=", "10"
  mOptInput.clear();
  for(int i = firstOptInput; i < mIns.size(); i += 3)
  {
    if((i + 3) > mIns.size())
    {
      addErrorText("CalcTALib::prepare: Something wrong with optional parameter list" );
      return false;
    }

    if(!mInfo.contains(mIns.at(i)))
    {
      addErrorText("CalcTALib::prepare: Unknown optional parameter: " + mIns.at(i));
      return false;
    }

    bool isNumber;
    double value = mIns.at(i + 2).toDouble(&isNumber);
    if(!isNumber)
    {
      addErrorText("CalcTALib::prepare: Optional parameter "
                           + mIns.at(i) + " is not a number: " + mIns.at(i + 2));
      return false;
    }

    mOptInput.insert(mIns.at(i), value);
  }

  // add the output to the...
  for(int i = 0; i < mInfo.value("nbOutput").toInt(); ++i)
  {
    mUsedVariables->insert(mOuts.at(i));
    mOutput.insert("OutType" + QString::number(i + 1), mOuts.at(i));
  }

  if(hasError()) return false;

  return true;
}

bool CalcTALib::calc()
{
  if(hasError()) return false;

  getIndicatorVariables();

  TA_ParamHolder* parmHolder;
  TA_RetCode retCode;

  if(!initTALib(&parmHolder)) return false;

  // call the beast...
  TA_Integer startIdx = 0;
  TA_Integer endIdx = mData->dataTupleSize() - mFirstValid - 1;
  TA_Integer outBegIdx;
  TA_Integer outNbElement;

  retCode = TA_CallFunc(parmHolder, startIdx, endIdx, &outBegIdx, &outNbElement);
  if(!retCode == TA_SUCCESS )
  {
    // error
    qDebug() << "CalcTALib::calc: TALib returned error code" << retCode;
    freesParmHolder(parmHolder);
    return false;
  }

  // set the valid ranges
  QHashIterator<QString, QString> hit(mOutput);
  while(hit.hasNext())
  {
    QString key = hit.next().value();
    mData->setValidRange(key, outBegIdx + mFirstValid, outNbElement);
  }

  freesParmHolder(parmHolder);

  return true;
}

int CalcTALib::barsNeeded(DataTupleSet* data)
{
  DataTupleSet* save = mData;

  mData = data;

  TA_ParamHolder* parmHolder;
  TA_RetCode retCode;

  if(!initTALib(&parmHolder))
  {
    mData = save;
    return -1;
  }

  TA_Integer lookback;
  retCode = TA_GetLookback(parmHolder, &lookback);

  if(!retCode == TA_SUCCESS )
  {
    // error
    qDebug() << "error 7";
    mData = save;
    freesParmHolder(parmHolder);
    return -1;
  }

  ++lookback;

  QHashIterator<QString, QString> hit(mOutput);
  while(hit.hasNext())
  {
    QString key = hit.next().value();
    mData->setNeededBars(key, lookback);
  }

  mData = save;
  freesParmHolder(parmHolder);

  return lookback;
}

bool CalcTALib::initTALib(TA_ParamHolder **parmHolder)
{
  setFirstValid();

  const TA_FuncHandle* handle;
  TA_RetCode retCode;

  retCode = TA_GetFuncHandle(mIns.at(0).toAscii(), &handle);
  if(!retCode == TA_SUCCESS )
  {
    // error
    qDebug() << "error 1";
    return false;
  }

  retCode = TA_ParamHolderAlloc(handle, parmHolder);
  if(!retCode == TA_SUCCESS )
  {
    // error
    qDebug() << "error 2";
    return false;
  }

  // set input variables
  int nbInput = mInfo.value("nbInput").toInt();
  for(int i = 0; i < nbInput; ++i)
  {
    QString key = "InputType" + QString::number(i + 1);

    if(mInfo.value(key) == "EODBAR")
    {
      const TA_Real* open;
      const TA_Real* high;
      const TA_Real* low;
      const TA_Real* close;
      const TA_Real* volume;
      const TA_Real* openInterest;

      QStringList input = mInfo.value("Input").toString().split(", ");
      key.append("-");
      for(int j = 0; j < mInput.size(); ++j)
      {
        QString part = mInfo.value(key + QString::number(j)).toString();
        const TA_Real* val = valueArray(mInput.value(key + QString::number(j)));

        if(part == "OPEN") open = val;
        if(part == "HIGH") high = val;
        if(part == "LOW") low = val;
        if(part == "CLOSE") close = val;
        if(part == "VOLUME") volume = val;
        if(part == "OPINT") openInterest = val;
        //if(part == "TIMESTAMP") open = val;
      }

      retCode = TA_SetInputParamPricePtr(*parmHolder, i, open, high, low, close
                                          , volume, openInterest);
      if(!retCode == TA_SUCCESS )
      {
        // error
        qDebug() << "error 3";
        return false;
      }
    }
    else if(mInfo.value(key) == "DOUBLE")
    {
      const TA_Real* value = valueArray(mInput.value(key));

      retCode = TA_SetInputParamRealPtr(*parmHolder, i, value);
      if(!retCode == TA_SUCCESS )
      {
        // error
        qDebug() << "error 4";
        return false;
      }
    }
    else if(mInfo.value(key) == "INTEGER")
    {
        qDebug() << "CalcTALib::calc: oops...'integer' as input is not implemented";
        return false;
    }
  }

  // set optional inputs
  nbInput = mInfo.value("nbOptInput").toInt();
  for(int i = 0; i < nbInput; ++i)
  {
    QString key = "OptInput" + QString::number(i);
    QString parm = mInfo.value(key).toString();
    if(!mOptInput.contains(parm)) continue;

    key = "OptInputType" + QString::number(i + 1);
    QString type = mInfo.value(key).toString();

    if(type == "DOUBLE")
    {
      TA_Real optInValue = mOptInput.value(parm);
      retCode = TA_SetOptInputParamReal(*parmHolder, i, optInValue);
    }
    else // sould be "INTEGER"
    {
      TA_Integer optInValue = (TA_Integer)mOptInput.value(parm);
      retCode = TA_SetOptInputParamInteger(*parmHolder, i, optInValue);
    }
  }

  // create our output variable
  QHashIterator<QString, QString> hit(mOutput);
  while(hit.hasNext())
    addToDataSet(hit.next().value());

  // set output
  int nbOutput = mInfo.value("nbOutput").toInt();
  for(int i = 0; i < nbOutput; ++i)
  {
    QString key = "OutType" + QString::number(i + 1);
    if(mInfo.value(key) == "DOUBLE")
    {
      TA_Real* out = mData->valueArray(mOutput.value(key));
      retCode = TA_SetOutputParamRealPtr(*parmHolder, i, out);
      if(!retCode == TA_SUCCESS )
      {
        // error
        qDebug() << "error 5";
        return false;
      }
    }
    else // sould be "INTEGER"
    {
      //TA_Integer* out = mData->valueArray(mOutput.value(key));
      //retCode = TA_SetOutputParamIntegerPtr(*parmHolder, i, out);
      qDebug() << "CalcTALib::calc: oops...'integer' as output is not implemented";
      return false;
    }
  }

 return true;
}

void CalcTALib::freesParmHolder(TA_ParamHolder *parmHolder)
{
  TA_RetCode retCode;

  retCode = TA_ParamHolderFree(parmHolder);

  if(!retCode == TA_SUCCESS)
  {
    // error
    qDebug() << "CalcTALib::freesParmHolder: TALib returned error code: " << retCode;
  }
}

void CalcTALib::setFirstValid()
{
  mFirstValid = 0;

  QHashIterator<QString, QString> hit(mInput);
  while(hit.hasNext())
  {
    int outBegIdx;
    int outNbElement;
    mData->getValidRange(hit.next().value(), outBegIdx, outNbElement);

    if(mFirstValid < outBegIdx) mFirstValid = outBegIdx;
  }
}

TA_Real* CalcTALib::valueArray(const QString& key)
{
  int outBegIdx;
  int outNbElement;

  mData->getValidRange(key, outBegIdx, outNbElement);
  return mData->valueArray(key) + (mFirstValid - outBegIdx);
}
