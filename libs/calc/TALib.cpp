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

#include "TALib.h"
#include "ta_abstract.h"

TALib::TALib()
{
  mErrorMessage.clear();
  readSettings();
  init();
}

TALib::~TALib()
{}

void TALib::getAllFunctionNames(QStringList& names)
{
   names = mFunctionNames;
}

bool TALib::functionIsUnknown(const QString& name)
{
  if(mFunctionNames.contains(name)) return false;
  return true;
}

void TALib::getFunctionInfo(const QString& function, FunctionInfo& info)
{
  const TA_FuncHandle* handle;
  const TA_FuncInfo* theInfo;
  TA_RetCode retCode;

  retCode = TA_GetFuncHandle(function.toAscii(), &handle);
  if(!retCode == TA_SUCCESS )
  {
    // error
    return;
  }

  retCode = TA_GetFuncInfo( handle, &theInfo );
  if(!retCode == TA_SUCCESS )
  {
    // error
    return;
  }

  info.insert("Name", theInfo->name);
  info.insert("Group", theInfo->group);
  info.insert("LongName", theInfo->hint);
  info.insert("CamelCaseName", theInfo->camelCaseName);
  info.insert("nbInput", theInfo->nbInput);
  info.insert("nbOptInput", theInfo->nbOptInput);
  info.insert("nbOutput", theInfo->nbOutput);
  info.insert("Flags", theInfo->flags);

  // get all input parameters
  QString key;
  QStringList inputParms;
  unsigned int i;
  const TA_InputParameterInfo* paramInfo;

  for( i=0; i < theInfo->nbInput; i++ )
  {
    TA_GetInputParameterInfo(theInfo->handle, i, &paramInfo);

    switch( paramInfo->type )
    {
    case TA_Input_Price:
    {
        key = "InputType" + QString::number(i + 1);
        info.insert(key , "EODBAR");
        key.append("-");
        int j = -1;
        if(paramInfo->flags & TA_IN_PRICE_OPEN)
        {
          inputParms.append("<open>");
          info.insert(key + QString::number(++j), "OPEN");
        }
        if(paramInfo->flags & TA_IN_PRICE_HIGH)
        {
          inputParms.append("<high>");
          info.insert(key + QString::number(++j), "HIGH");
        }
        if(paramInfo->flags & TA_IN_PRICE_LOW)
        {
          inputParms.append("<low>");
          info.insert(key + QString::number(++j), "LOW");
        }
        if(paramInfo->flags & TA_IN_PRICE_CLOSE)
        {
          inputParms.append("<close>");
          info.insert(key + QString::number(++j), "CLOSE");
        }
        if(paramInfo->flags & TA_IN_PRICE_VOLUME)
        {
          inputParms.append("<volume>");
          info.insert(key + QString::number(++j), "VOLUME");
        }
        if(paramInfo->flags & TA_IN_PRICE_OPENINTEREST)
        {
          inputParms.append("<opInt>");
          info.insert(key + QString::number(++j), "OPINT");
        }
        if(paramInfo->flags & TA_IN_PRICE_TIMESTAMP)
        {
          inputParms.append("TIMESTAMP");
          info.insert(key + QString::number(++j), "<timestamp>");
        }
        break;
    }
    case TA_Input_Real:
        info.insert("InputType" + QString::number(i + 1) , "DOUBLE");
        inputParms.append("<foo" + QString::number(i + 1) + ">");
        break;
    case TA_Input_Integer:
        info.insert("InputType" + QString::number(i + 1) , "INTEGER");
        inputParms.append("<foo" + QString::number(i + 1) + ">");
        break;
    }
  }
  info.insert("Input", inputParms.join(", "));

  // get all optional input parameters
  QStringList optInputParms;
  QString def; // def=default
  const TA_OptInputParameterInfo* optParmInfo;
  for( i=0; i < theInfo->nbOptInput; i++ )
  {
    TA_GetOptInputParameterInfo(theInfo->handle, i, &optParmInfo);

    key = "OptInputType" + QString::number(i);
    if(optParmInfo->type & (TA_OptInput_RealRange|TA_OptInput_RealList))
      info.insert(key, "DOUBLE");
    if(optParmInfo->type & (TA_OptInput_IntegerRange|TA_OptInput_IntegerList))
      info.insert(key, "INTEGER");

    def.setNum(optParmInfo->defaultValue);
    key = optParmInfo->paramName;
    key.remove("optIn");
    info.insert(key, def);
    info.insert("OptInput" + QString::number(i), key);
    key.append("=" + def);
    optInputParms.append(key);

    key = "OptInParm " + QString::number(i + 1);
    info.insert(key, optParmInfo->hint);
  }

  if(optInputParms.size() > 0)
    info.insert("OptInput", optInputParms.join(", "));

  // get all output names
  const TA_OutputParameterInfo* outInfo;
  for( i=0; i < theInfo->nbOutput; i++ )
  {
    TA_GetOutputParameterInfo(theInfo->handle, i, &outInfo);

    if(outInfo->type == TA_Output_Real)
      info.insert("OutType" + QString::number(i + 1), "DOUBLE");
    else info.insert("OutType" + QString::number(i + 1), "INTEGER");

    key = outInfo->paramName;
    key.remove("outReal");
    key.remove("outInteger");
    key.remove("out");
    if(key.isEmpty()) key = theInfo->name;
    info.insert("OutName" + QString::number(i + 1), key);

    key = "PlotType" + QString::number(i + 1);
    if(outInfo->flags & TA_OUT_LINE) info.insert(key, "LINE");
    else if(outInfo->flags & TA_OUT_DOT_LINE) info.insert(key, "DOTLINE");
    else if(outInfo->flags & TA_OUT_DASH_LINE) info.insert(key, "DASHLINE");
    else if(outInfo->flags & TA_OUT_DOT) info.insert(key, "DOT");
    else if(outInfo->flags & TA_OUT_HISTO) info.insert(key, "HISTOGRAM");
    else
    {
      info.insert(key, "???");
      //qDebug() << "TALib::getFunctionInfo: Unknown OutPlotType, outInfo->flags=" << outInfo->flags;
    }

    key = "OutHint" + QString::number(i + 1);
    if(outInfo->flags & TA_OUT_PATTERN_BOOL)
      info.insert(key, "Indicates if pattern exists (!=0) or not (0)");

    if(outInfo->flags & TA_OUT_PATTERN_BULL_BEAR)
      info.insert(key, "=0 no pattern, > 0 bullish, < 0 bearish");

    if(outInfo->flags & TA_OUT_PATTERN_STRENGTH)
      info.insert(key, "=0 neutral, ]0..100] getting bullish, ]100..200] bullish, [-100..0[ getting bearish, [-200..100[ bearish");

    if(outInfo->flags & TA_OUT_POSITIVE) info.insert(key, "Output can be positive");
    if(outInfo->flags & TA_OUT_NEGATIVE) info.insert(key, "Output can be negative");
    if(outInfo->flags & TA_OUT_ZERO) info.insert(key, "Output can be zero");

    getFunctionUsage(info);
  }
}

bool TALib::getIndicator(const QString& inclCmd, QStringList& indicator)
{
  // inclCmd looks like "INCLTALIB(<func>, <parm1>[,...<parmN>])"
  QStringList parms = inclCmd.split(",");
  parms[0] = parms[0].remove("INCLTALIB(");
  parms[parms.size() - 1].chop(1); // remove last brace

  if(!mFunctionNames.contains(parms.at(0)))
  {
    mErrorMessage= "TALib::prepare: unknown function: " + parms.at(0);
    return false;
  }

  FunctionInfo info;
  getFunctionInfo(parms.at(0), info);

  // build the output variables
  QStringList hl;  // help list, filled with output variables
  QString hs;      // help string
  int n = info.value("nbOutput").toInt();
  for(int i = 0; i < n; ++i)
  {
    hl.append(info.value("OutName" + QString::number(i + 1)).toString());
  }

  // build plot commands
  for(int i = 0; i < n; ++i)
  {
    QString pt = info.value("PlotType" + QString::number(i + 1)).toString();
    hs = "PLOT(" + pt + ", " + hl.at(i) + ")";
    if(pt == "HISTOGRAM") indicator.prepend(hs);
    else indicator.append(hs);
  }

  // build set color commands
  QString color[] = {"red", "green", "yellow", "blue", "white"};
  for(int i = 0; i < n; ++i)
  {
    hs = hl.at(i) + " = SETCOLOR(" + color[i % 5] + ")";
    indicator.prepend(hs);
  }

  // build the function call
  hs = hl.join(", ");
  hs.append(" = TALIB(");
  hs.append(parms.join(", ") + ")");
  indicator.prepend(hs);

  return true;
}

/***********************************************************************
*
*                              private stuff
*
************************************************************************/

void TALib::getFunctionUsage(FunctionInfo& info)
{
  // build all INCLTALIB(...) possibilities
  if(info.value("InputType1") == "EODBAR")
  {
    info.insert("Include1","INCLTALIB("
                          + info.value("Name").toString() + ", FI())");

    info.insert("Include2", "INCLTALIB(" + info.value("Name").toString()
                             + ", " + info.value("Input").toString() + ")");

    if(info.contains("OptInput"))
    {
     info.insert("Include3","INCLTALIB(" + info.value("Name").toString()
                + ", FI()"
                + ", " + info.value("OptInput").toString() + ")");

      info.insert("Include4","INCLTALIB(" + info.value("Name").toString()
                + ", " + info.value("Input").toString()
                + ", " + info.value("OptInput").toString() + ")");
    }
  }
  else
  {
    info.insert("Include1", "INCLTALIB(" + info.value("Name").toString()
                             + ", " + info.value("Input").toString() + ")");

    if(info.contains("OptInput"))
    {
      info.insert("Include2","INCLTALIB(" + info.value("Name").toString()
                + ", " + info.value("Input").toString()
                + ", " + info.value("OptInput").toString() + ")");
    }
  }

  // build all <foo> = TALIB(...) possibilities
  QStringList list;
  for(int i = 0; i < info.value("nbOutput").toInt(); ++i)
  {
    list.append("<out" + QString::number(i + 1) + ">");
  }
  QString out = list.join(", ");

  if(info.value("InputType1") == "EODBAR")
  {
    info.insert("Call1", out + " = TALIB("
                             + info.value("Name").toString() + ", FI())");

    info.insert("Call2", out + " = TALIB("
                             + info.value("Name").toString()
                             + ", " + info.value("Input").toString() + ")");

    if(info.contains("OptInput"))
    {
      info.insert("Call3", out + " = TALIB("
                              + info.value("Name").toString()
                              + ", FI()"
                              + ", " + info.value("OptInput").toString() + ")");

      info.insert("Call4", out + " = TALIB("
                              + info.value("Name").toString()
                              + ", " + info.value("Input").toString()
                              + ", " + info.value("OptInput").toString() + ")");
    }
  }
  else
  {
    info.insert("Call1", out + " = TALIB("
                             + info.value("Name").toString()
                             + ", " + info.value("Input").toString() + ")");

    if(info.contains("OptInput"))
    {
      info.insert("Call2", out + " = TALIB("
                              + info.value("Name").toString()
                              + ", " + info.value("Input").toString()
                              + ", " + info.value("OptInput").toString() + ")");
    }
  }
}

void  TALib::readSettings()
{

}

void  TALib::init()
{
  // the goal: get all available function names

  mFunctionNames.clear();

  // to got they we have to get all groups previous
  TA_StringTable* table;
  TA_RetCode retCode;

  retCode = TA_GroupTableAlloc(&table);

  if(retCode == TA_SUCCESS)
  {
    unsigned int i;
    for(i = 0; i < table->size; ++i)
    {
      // and now get each function name in the group
      TA_StringTable* table2;
      retCode = TA_FuncTableAlloc(table->string[i], &table2);

      if(retCode == TA_SUCCESS)
      {
        unsigned  int j;
        for(j = 0; j < table2->size; ++j)
        {
          mFunctionNames.append(table2->string[j]);
        }

        TA_FuncTableFree(table2);
      }
    }

    TA_GroupTableFree(table);
  }

  mFunctionNames.sort();
  //qDebug() << mFunctionNames;
}

