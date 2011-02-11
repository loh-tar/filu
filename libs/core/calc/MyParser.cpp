//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011  loh.tar@googlemail.com
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

  return &dummy;
}

MyParser::MyParser(Newswire* parent)
        : Newswire(parent, FUNC)
{
  mParser.DefineOprt("&", &MyAndFunc, 0);

  // Define the dummy variable factory
  mParser.SetVarFactory(&MyAddVariable, &mParser);

  mVariables   = 0;
  mMyVariables = false;
}

MyParser::~MyParser()
{
  if(mMyVariables) delete mVariables;
}

bool MyParser::setExp(const QString& expr)
{
  mParser.SetExpr(expr.toStdString());

  // Call .Eval() to forces mu::Parser to parse the formula
  //  and create the needed variables, dummy variables
  // in our case
  try
  {
    mParser.Eval();
  }
  catch(mu::Parser::exception_type& e)
  {
    error(FUNC, tr("Message: %1").arg(e.GetMsg().data()));
    errInfo(FUNC, tr("Formula: %1").arg(e.GetExpr().data()));
    errInfo(FUNC, tr("Token: %1").arg(e.GetToken().data()));
    errInfo(FUNC, tr("Position: %1").arg(e.GetPos()));
    errInfo(FUNC, tr("ErrCode: %1").arg(e.GetCode()));

    return false;
  }

  // Ask mu::Parser for the variables names
  QSet<QString> usedVariables;
  appendUsedVariables(usedVariables);

  // Create mVariables if not set from outside
  if(!mVariables)
  {
    mVariables   = new QHash<QString, double>;
    mMyVariables = true;
  }

  // ...and set the real needed variables. Well, its a crook again :-/
  foreach(QString name, usedVariables)
  {
    // 0.0 is only a dummy, replaced later
    QHash<QString, double>::iterator it = mVariables->insert(name, 0.0);
    mParser.DefineVar(name.toStdString(), &it.value());
  }

  return true;
}

QString MyParser::getExp()
{
  return mParser.GetExpr().data();
}

void MyParser::useVariable(const QString& name, double& var)
{
  mParser.DefineVar(name.toStdString(), &var);
}

void MyParser::useVariables(QHash<QString, double>* variables)
{
  mVariables = variables;
}

void MyParser::useData(DataTupleSet* data)
{
  mData = data;

  mUsedMData.clear();
  QSet<QString> usedNames;
  QStringList   mdataNames;

  mData->getVariableNames(mdataNames);

  appendUsedVariables(usedNames);

  foreach(const QString& name, usedNames)
  {
    if(mdataNames.contains(name)) mUsedMData.insert(name);
  }
  //qDebug() << "MyParser::useData() mData variables" << mUsedMData;
}

void MyParser::appendUsedVariables(QSet<QString>& list)
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
  // Returns
  //    0 if all is ok
  //    1 if no data from mData
  //    2 if error while mu::Parser.Eval()

  // Fill mVariables with values from mData
  foreach(const QString& name, mUsedMData)
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
    error(FUNC, tr("Message: %1").arg(e.GetMsg().data()));
    errInfo(FUNC, tr("Formula: %1").arg(e.GetExpr().data()));
    errInfo(FUNC, tr("Token: %1").arg(e.GetToken().data()));
    errInfo(FUNC, tr("Position: %1").arg(e.GetPos()));
    errInfo(FUNC, tr("ErrCode: %1").arg(e.GetCode()));

    return 2;
  }

}
