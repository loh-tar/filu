//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
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

#ifndef SCRIPT_HPP
#define SCRIPT_HPP

class QProcess;

#include "FObject.h"

class Script : public FObject
{
  Q_OBJECT

  public:
                  Script(FClass* parent);
    virtual      ~Script();

    QStringList   providerList();
    QStringList   functionList(const QString& provider);
    QHash<QString, QString> functionInfo(const QString& provider, const QString& function);

    QStringList*  execute(const QString& script, const QStringList& parameters);

    QStringList*  askProvider(const QString& provider
                            , const QString& function
                            , const QStringList& parameters);

    void          showWaitWindow(bool yes = true);
    void          stopRunning();

    signals:
    void          newData(QStringList*);
    void          finished();

  protected slots:
    void          readStdOut();
    void          returnResult();

  protected:
    QString       locateProviderScript(const QString& function, const QString& provider);

    QString       mProviderPath;
    QString       mWorkingDir;
    bool          mShowWaitWindow;
    QProcess*     mProc;
    bool          mRunning;
    QStringList*  mResult;
};

#endif
