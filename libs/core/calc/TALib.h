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

#ifndef TALIB_HPP
#define TALIB_HPP

#include <QtCore>

/***********************************************************************
*
*   Support the full functionality of TA-Lib but don't calculate.
*
*   It is more like an interface. He fetch all infos about a function
*   and provide they in a user readable form.
*
*   One other task is to replace "INCLTALIB(...)" with "foo = TALIB(...)"
*   and add all "PLOT(...)" lines needed to got the desired indicator.
*
************************************************************************/

typedef QHash<QString, QVariant> FunctionInfo;
typedef QHashIterator<QString, QVariant> FunctionInfoIterator;

class TALib
{
  public:
                    TALib();
    virtual        ~TALib();

    void            getAllFunctionNames(QStringList& names);
    bool            functionIsUnknown(const QString& name);
    void            getFunctionInfo(const QString& function, FunctionInfo& info);

    bool            getIndicator(const QString& inclCmd, QStringList& indicator);
    const QString&  errorText() const { return mErrorMessage; } ;

  protected:
    void        getFunctionUsage(FunctionInfo& info);
    void        readSettings();
    void        init();

    QStringList mFunctionNames;
    QString     mErrorMessage;
};

#endif
