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

#include "CalcMuParser.h"

CalcMuParser::CalcMuParser(Indicator* parent) : CalcType(parent)
{
  mType = "muParser";

  mParser = new MyParser;
}

CalcMuParser::~CalcMuParser()
{
  delete mParser;
}

bool CalcMuParser::prepare(CalcParms& parms)
{
  init(parms);

  checkOutputCount(1);

  QString exp = mIns.join(" ");

  if(!mParser->setExp(exp))
  {
    addErrorText(mParser->errorText());
    return false;
  }

  mParser->appendUsedVariables(mParserVariables);
  mParser->appendUsedVariables(*mUsedVariables);
  //qDebug() << mUsedVariables->toList();

  if(hasError()) return false;

  return true;
}

bool CalcMuParser::calc()
{
  if(hasError()) return false;

  getIndicatorVariables();

  // create our output variable
  addToDataSet(mOuts.at(0));

  int outBegIdx = 0;
  int outNbElement = 0;
  int firstValid = 0;

  QStringList list;
  mData->getVariableNames(list);

  QString var;
  foreach(var, mParserVariables)
  {
    if(!list.contains(var))
    {
      addErrorText("CalcMuParser::calc: Variable not found: " + var);
      continue;
    }
    mData->getValidRange(var, outBegIdx, outNbElement);
    if(firstValid < outBegIdx) firstValid = outBegIdx;
  }

  if(hasError()) return false;

  mData->setValidRange(mOuts.at(0), firstValid, mData->dataTupleSize() - firstValid);
  mData->setRange(firstValid, mData->dataTupleSize());

  mParser->useData(mData);

  //...and rock 'n' roll...
  while(mData->next())
  {
    double result;
    int ret = mParser->calc(result);
    if(ret == 1)
    {
      qDebug()   << "CalcMuParser::calc: Oops?!? No valid value in mData, should never happens!";
      continue;
    }
    else if(ret == 2)
    {
      qDebug()   << "CalcMuParser::calc: Bad value from mu::Parser";
      continue;
    }

    mData->setValue(mOuts.at(0), result);
  }

  mData->setRange();
  return true;
}

