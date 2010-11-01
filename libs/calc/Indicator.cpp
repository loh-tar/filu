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

#include "Indicator.h"

#include "DataTupleSet.h"
#include "TALib.h"
#include "CalcType.h"

Indicator::Indicator(FClass* parent)
         : FClass(parent)
         , mTALib(0)
         , mData(0)
         , mIgnorePlot(true)
         , mAlreadyIncluded(new QSet<QString>)
         , mUsedVariables(new QSet<QString>)
         , mBuildInVariables(new QSet<QString>)

{
  *mBuildInVariables << "DAY" << "WEEK" << "MONTH" << "YEAR";

  readSettings();
}

Indicator::~Indicator()
{
  if(mData)  delete mData;
  if(mTALib) delete mTALib;

  for(int i = 0; i < mCalcCommands.size(); ++i)
  {
    delete mCalcCommands[i];
  }

  delete mAlreadyIncluded;
  delete mUsedVariables;
  delete mBuildInVariables;
}

QStringList* Indicator::useFile(const QString& fileName)
{
  clearErrors();
  mOrigIndicator.clear();
  mAlreadyIncluded->clear();

  mViewName = fileName; // if no VIEWNAME(...) in indicator file

  if(!readIndicator(fileName, mIndicator)) return 0;

// for(int i=0; i < mOrigIndicator.size(); i++)
//   qDebug() << "mOrigIndicator:" << i << ": " << mOrigIndicator.at(i);

  return& mIndicator;
}

QString Indicator::viewName()
{
  return mViewName;
}

bool Indicator::useIndicator(const QStringList& indicator)
{
  clearErrors();
  mOrigIndicator.clear();
  mAlreadyIncluded->clear();

  mIndicator = indicator;

  return parse(mIndicator);
}

void Indicator::getIndicator(QStringList& indicator)
{
  indicator = mOrigIndicator;
}

DataTupleSet* Indicator::calculate(BarTuple* bars)
{
  //qDebug() << "Indicator::calculate(BarTuple* bars)";
  if(mData) delete mData;
  mData = new DataTupleSet;

  if(!mData->appendBarTuple(bars)) //FIXME: add also extra FIs
  {
    addErrorText("Indicator::calculate: Fail to create mData");
    delete mData;
    mData = 0;
    return 0;
  }

  return calculate(mData);
}

DataTupleSet* Indicator::calculate(DataTupleSet* data)
{
  if(!data) return 0; // be on the save side

  // No check here if(hasError()) , we want ask all CalcType what's wrong.
  // That's important because of CalcWatchdogs, depend on found dog or not
  // cause an error.
  // if(hasError()) return 0;

  if(mParseError) return 0; // That's the solution :-/

  clearErrors();

  // All CalcType needs access to mData,
  // so we have to copy here if direct called from outside (e.g. Trader.cpp)
  DataTupleSet* saveOwnMData = mData;
  mData = data;

  data->setRange();
  data->rewind();

  //
  // check for build-in date "functions"
  if(mUsedVariables->contains("DAY"))
  {
    data->append("DAY");
    while(data->next())
    {
      QDate date;
      data->getDate(date);
      data->setValue("DAY", date.dayOfWeek());
    }

    data->rewind();
  }

  if(mUsedVariables->contains("WEEK"))
  {
    data->append("WEEK");
    while(data->next())
    {
      QDate date;
      data->getDate(date);
      data->setValue("WEEK", date.weekNumber());
    }

    data->rewind();
  }

  if(mUsedVariables->contains("MONTH"))
  {
    data->append("MONTH");
    while(data->next())
    {
      QDate date;
      data->getDate(date);
      data->setValue("MONTH", date.month());
    }

    data->rewind();
  }

  // almost useless..but anyway
  if(mUsedVariables->contains("YEAR"))
  {
    data->append("YEAR");
    while(data->next())
    {
      QDate date;
      data->getDate(date);
      data->setValue("YEAR", date.year());
    }

    data->rewind();
  }

  //
  // here is the beef, calc ...
  data->setRange();
  for(int i = 0; i < mCalcCommands.size(); ++i)
  {
    //qDebug() << "Indicator::calculate()" << mCalcCommands.at(i)->getType();
    if(!mCalcCommands.at(i)->calc())
    {
      addErrorText(mCalcCommands.at(i)->errorText());
    }
  }

  // Restore
  mData = saveOwnMData;

  if(hasError()) return 0;

/*
QStringList list;
data->getVariableNames(list);
qDebug() << "Indicator::calculate: data->getVariableNames():" << list;
*/

  return data;
}

void Indicator::ignorePlot(bool ignore /* = true*/)
{
  mIgnorePlot = ignore;
}

int Indicator::barsNeeded()
{
  DataTupleSet data;
  BarTuple bars(1);

  if(!data.appendBarTuple(&bars))
  {
    addErrorText("Indicator::barsNeeded: Fail to create data");
    return -1;
  }

  int MaxNeeded = -1;
  for(int i = 0; i < mCalcCommands.size(); ++i)
  {
    int needed = mCalcCommands.at(i)->barsNeeded(&data);
    if(needed > MaxNeeded) MaxNeeded = needed;
  }

  return MaxNeeded;
}

void Indicator::getVariableNames(QSet<QString> *list)
{
  *list = *mUsedVariables;
}

/***********************************************************************
*
*                              private stuff
*
************************************************************************/

bool Indicator::readIndicator(const QString& fileName, QStringList& indicator)
{
  // make sure we have no garbage in the indicator
  indicator.clear();

  if (fileName.isEmpty())
  {
    addErrorText("Indicator::readIndicator: No indicator file name given");
    return false;
  }

  QFile file(mIndicatorPath + fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    addErrorText("Indicator::readIndicator: Can't load indicator: " + fileName);
    return false;
  }

  // read/fill the Indicator
  QTextStream in(&file);
  while (!in.atEnd())
  {
    QString line = in.readLine();
    indicator.append(line);
  }

  file.close();

  return parse(indicator);
}

bool Indicator::parse(QStringList& indicator)
{
  bool rootFile = mOrigIndicator.isEmpty();

  int i = 0;
  //
  // remove all unwanted stuff
  while( i < indicator.size())
  {
    if(rootFile) mOrigIndicator.append(indicator.at(i));

    bool remove = false;

    if(indicator[i].startsWith("*")) remove = true; // ignore remarks
    else if(indicator[i].isEmpty())  remove = true;
    else if(indicator[i].contains("PLOT(") and mIgnorePlot)     remove = true;
    else if(indicator[i].contains("SETCOLOR(") and mIgnorePlot) remove = true;

    if(remove)
    {
      indicator.removeAt(i);
      continue;
    }

    ++i;
  }

  //
  // extract view name, if some
  QStringList viewName = indicator.filter("VIEWNAME(");
  if(viewName.size())
  {
    mViewName = viewName.at(0);
    mViewName.remove("VIEWNAME(");
    mViewName.remove(")");
    mViewName.remove(" ");
  }

  //
  // load included indicator files
  i = 0;
  while((i = indicator.indexOf(QRegExp("INCLUDE\\(.*"), i)) >= 0)
  {
    // extract the file name and optional IGNOREPLOT
    QStringList parms = indicator.at(i).split(",");
    parms.replaceInStrings(QRegExp(".*\\(|\\).*"), "");
    parms.replaceInStrings(" ", "");
    if(parms.size() == 0)
    {
      addErrorText("Indicator::parse: No parameter at INCLUDE()");
      ++i;
      continue;
    }

    bool ignorePlot = mIgnorePlot; // save global setting

    if(parms.size() == 2)
    {
      if(parms.at(1) == "IGNOREPLOT")
      {
        mIgnorePlot = true;
      }
      else
      {
        addErrorText("Indicator::parse: 2nd INCLUDE() parameter unknown: " + parms.at(1));
        ++i;
        continue;
      }
    }

    if(mAlreadyIncluded->contains(parms.at(0)))
    {
      ++i;
      continue;
    }

    mAlreadyIncluded->insert(parms.at(0));

    // read the file...
    QStringList list;
    if(!readIndicator(parms.at(0), list))
    {
      addErrorText("Indicator::parse: Fail to INCLUDE() file");
      mIgnorePlot = ignorePlot;
      ++i;
      continue;
    }

    mIgnorePlot = ignorePlot;

    // ...and include each line
    for(int j = 0; j < list.size(); j++)
    {
      indicator.insert(i, list.at(j));
      ++i;
    }

    indicator.removeAt(i); // the old INCLUDE(...) line
  }

  //
  // include ta-lib indicators
  i = 0;
  while((i = indicator.indexOf(QRegExp("INCLTALIB\\(.*"), i)) >= 0)
  {
    if(!mTALib) mTALib = new TALib();

    // prepare for battle...
    QStringList list;
    if(!mTALib->getIndicator(indicator.at(i), list))
    {
      addErrorText(mTALib->errorText());
      ++i;
      continue;
    }

    // ...and include each line
    for(int j = 0; j < list.size(); j++)
    {
      indicator.insert(i, list[j].remove(" "));
      ++i;
    }
    indicator.removeAt(i); // the old INCLTALIB(...) line
  }

// for(int i=0;i<indicator.size();i++)
//   qDebug() << "Indicator::parse: That's it:" << i << ": " << indicator.at(i);

  if(hasError())
  {
    mParseError = true;
    return false;
  }

  mParseError = false;

  //
  //  puh, that's all
  return prepare(indicator);
}

bool Indicator::prepare(QStringList& indicator)
{
  mUsedVariables->clear();
  mUsedVariables->insert("OPEN");
  mUsedVariables->insert("HIGH");
  mUsedVariables->insert("LOW");
  mUsedVariables->insert("CLOSE");
  mUsedVariables->insert("VOLUME");
  mUsedVariables->insert("OPINT");

  for(int i = 0; i < mCalcCommands.size(); ++i)
  {
    delete mCalcCommands[i];
  }
  mCalcCommands.clear();

  //
  // check for SIMTRADE
  int i = 0;
  while((i = indicator.indexOf(QRegExp("SIMTRADE\\(.*"), i)) >= 0)
  {
    // extract parameter list
    QString tradeRule = indicator.at(i);
    tradeRule.remove(QRegExp(".*\\(|\\).*"));
    //qDebug() << "Indicator::prepare:check for SIMTRADE" << tradeRule;

    QStringList ins(tradeRule);
    QStringList outs;
    CalcParms parms;     // for the beef
    parms.setIns(ins);
    parms.setOuts(outs);

    CalcType calcType(this);
    CalcType* newCalcType = calcType.createNew("SIMTRADE");
    if(!newCalcType)
    {
      addErrorText(calcType.errorText());
      return false;
    }

    mCalcCommands.append(newCalcType);

    if(!newCalcType->prepare(parms))
    {
      addErrorText(newCalcType->errorText());
      return false;
    }
    ++i;
  }

  //
  // load extra FIs
  i = 0;
  while((i = indicator.indexOf(QRegExp("LOADFI\\(.*"), i)) > 0)
  {
    // extract parameter list
    QString parameters = indicator.at(i);
    parameters.remove(QRegExp(".*\\(|\\).*"));
qDebug() << "Indicator::prepare:load extra FIs" << parameters;
    // load the bars to the FI
    //FIXME: has to be implemented
    addErrorText("Indicator::prepare: Not yet supported: " + indicator.at(i));
    return false;

    BarTuple bars(1); // = mFilu->getBars();
    if(!mData->appendBarTuple(&bars))
    {
      addErrorText("Indicator::prepare: Fail to create mData");
      return 0;
    }

    ++i;
  }

  //
  // check for equations/function calls
  QStringList equations = indicator.filter("=");

  if(equations.isEmpty()) return true;

  // some preparations for constant search&replace
  QStringList days;
  days << "MON" << "TUE" << "WED" << "THU" << "FRI" << "SAT" << "SUN";
  QStringList months;
  months << "JAN" << "FEB" << "MAR" << "APR" << "MAY" << "JUN" << "JUL"
         << "AUG" << "SEP" << "OCT" << "NOV" << "DEC";

  for(i = 0; i < equations.size(); i++)
  {
    CalcParms parms;      // for the beef

    // a function call looks like
    // sma50 = TALIB(SMA, CLOSE, TimePeriod=50)
    // sma50 = SETCOLOR(red)
    // midPrice = MUP((HIGH + LOW ) * 0.5)

    // split the line into parts
    QString line = equations.at(i);
    QStringList parts;
    QString part;
    for(int i = 0; i - line.size(); ++i)
    {
      QChar ch = line.at(i);
      if(ch == ' ') continue;

      uint u = uint(ch.unicode());
      // matches [a-zA-Z0-9_.]
      if((u - 'a' < 26 || u - 'A' < 26 || u - '0' < 10 || u == '_' || u == '.'))
      {
        part += ch;
        continue;
      }

      if(!part.isEmpty()) parts << part;
      part.clear();

      if( (ch == '-') or (ch == '#') ) part += ch;
      else if( (ch == '=') and (parts.size() > 0) )
      {
        // build operators: <=, >=, !=, ==
        if(parts.last().contains(QRegExp("[\\<\\>\\!\\=]"))) parts.last() += ch;
        else parts << ch;
      }
      else if(ch != ',') parts << ch;
    }

    //qDebug() << "\nparts: " << parts;

    // get the output
    QStringList outs = parts.mid(0, parts.indexOf("="));
    parms.setOuts(outs);
    //qDebug() << "outs: " << outs;

    // get the input
    QStringList ins  = parts.mid(parts.indexOf("=") + 1);
    //qDebug() << "ins1: " << ins;

    if(ins.size() < 3)
    {
      addErrorText("Indicator::prepare: Something wrong with equation");
      continue;
    }

    // get the function name
    QString func = ins.at(0);
    //qDebug() << "func:" << func;
    ins.removeFirst(); // remove function name

    // remove the function parentheses
    ins.removeFirst();
    ins.removeLast();

    // replace days and month constants
    for(int i = 0; i < ins.size(); ++i)
    {
      int day = days.indexOf(ins.at(i));
      if(day > -1) ins.replace(i, QString::number(day + 1));

      int month = months.indexOf(ins.at(i));
      if(month > -1) ins.replace(i, QString::number(month + 1));
    }
    //qDebug() << "ins2: " << ins;
    parms.setIns(ins);

    CalcType calcType(this);
    CalcType* newCalcType = calcType.createNew(func);
    if(!newCalcType)
    {
      addErrorText(calcType.errorText());
      continue;
    }

    mCalcCommands.append(newCalcType);

    if(!newCalcType->prepare(parms)) addErrorText(newCalcType->errorText());
   }

// for(int i = 0; i < mCalcCommands.size(); i++)
// {
//   qDebug() << "Indicator::prepare: mCalcCommands=" << i << ": "
//           << mCalcCommands.at(i)->getType();
// }
// qDebug() << "Indicator::prepare: mUsedVariables=" << mUsedVariables->values();
// qDebug() << "barsNeeded(): " << barsNeeded();

  if(hasError()) return false;

  return true;
}

void  Indicator::readSettings()
{
  mIndicatorPath = mRcFile->getST("IndicatorPath");
}
