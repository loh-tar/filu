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

#include "MyParser.h"

mu::value_type MyAndFunc(mu::value_type a, mu::value_type b)
{
  return (a and b);
}

static mu::value_type* MyAddVariable(const mu::char_type */*a_szName*/, void * /*a_pUserData*/)
{
  static double dummy = 1.0;

  //qDebug() << "MyAddVariable() added" << a_szName;

  return& dummy;
}

MyParser::MyParser()
{
  mParser.DefineOprt("&", &MyAndFunc, 0);

  // Define the dummy variable factory
  mParser.SetVarFactory(&MyAddVariable, &mParser);

  mVariables   = 0;
  mMyVariables = false;
  mUsedMData   = 0;
}

MyParser::~MyParser()
{
  if(mMyVariables) delete mVariables;
  if(mUsedMData) delete mUsedMData;
}

bool MyParser::setExp(const QString& expr)
{
  mParser.SetExpr(expr.toStdString());

  // call .Eval() to forces mu::Parser to parse the formula
  //  and create the needed variables, dummy variables
  // in our case
  try
  {
    mParser.Eval();
  }
  catch(mu::Parser::exception_type& e)
  {
    QString err;
    mErrorMessage << "MyParser::setExp: Error...";
    err = "  Message: " + QString(e.GetMsg().data());
    mErrorMessage << err;
    err = "  Formula: " + QString(e.GetExpr().data());
    mErrorMessage << err ;

    qDebug() << "Message:  " << e.GetMsg().data();
    qDebug() << "Formula:  " << e.GetExpr().data();
    qDebug() << "Token:    " << e.GetToken().data();
    qDebug() << "Position: " << e.GetPos();
    qDebug() << "Errc:     " << e.GetCode();
    return false;
  }

  // ask mu::Parser for the variables names
  QSet<QString> usedVariables;
  appendUsedVariables(usedVariables);

  // create mVariables if not set from outside
  if(!mVariables)
  {
    mVariables   = new QHash<const QString, double>;
    mMyVariables = true;
  }

  // and set the real needed variables...well, ist a crook again :-/
  QString name;
  foreach(name, usedVariables)
  {
    // 0.0 is only a dummy, replaced later
    QHash<const QString, double>::iterator it = mVariables->insert(name, 0.0);
    mParser.DefineVar(name.toStdString(), &it.value());
  }

  return true;
}

QString MyParser::getExp()
{
  return mParser.GetExpr().data();
}

void MyParser::useVariables(QHash<const QString, double> *variables)
{
  mVariables = variables;
}

void MyParser::useData(DataTupleSet* data)
{
  mData = data;

  if(mUsedMData) delete mUsedMData;

  mUsedMData = new QSet<QString>;
  QSet<QString> usedNames;
  QStringList   mdataNames;

  mData->getVariableNames(mdataNames);

  appendUsedVariables(usedNames);
  QString name;
  foreach(name, usedNames)
  {
    if(mdataNames.contains(name)) mUsedMData->insert(name);
  }
  //qDebug() << "MyParser::useData() mData variables" << *mUsedMData;
}

void MyParser::appendUsedVariables(QSet<QString> &list)
{
  // Get the map with the variables
  mu::varmap_type variables = mParser.GetVar();

  // Query the variables
  mu::varmap_type::const_iterator item = variables.begin();
  for (; item != variables.end(); ++item)
  {
    QString s = item->first.data();
    list.insert(s);
  }
}

int MyParser::calc(double& result)
{
  // returns
  //    0 if all is ok
  //    1 if no data from mData
  //    2 if error while mu::Parser.Eval()

  // fill mVariables with values from mData
  QString name;
  foreach(name, *mUsedMData)
  {
    double value;
    if(!mData->getValue(name, value)) return 1;

    mVariables->insert(name, value);
  }

  try
  {
    result = mParser.Eval();
    //qDebug() << "Formula:" << mParser.GetExpr().data() << "Result: " << result;
    return 0;
  }
  catch(mu::Parser::exception_type& e)
  {
    QString err;
    mErrorMessage << "MyParser::calc: Error...";
    err = "  Message: " + QString(e.GetMsg().data());
    mErrorMessage << err;
    err = "  Formula: " + QString(e.GetExpr().data());
    mErrorMessage << err ;

    qDebug() << "Message:  " << e.GetMsg().data();
    qDebug() << "Formula:  " << e.GetExpr().data();
    qDebug() << "Token:    " << e.GetToken().data();
    qDebug() << "Position: " << e.GetPos();
    qDebug() << "Errc:     " << e.GetCode();
    return 2;
  }

}

