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

#include "Script.h"

Script::Script(FClass* parent)
      : FObject(parent, FUNC)
      , mProviderPath(mRcFile->getST("ProviderPath"))
      , mShowWaitWindow(false)
      , mProc(0)
{}

Script::~Script()
{
  if(mProc) delete mProc;
}


QStringList * Script::execute(const QString& script, const QStringList& parameters)
{
  clearErrors();

  verbose(FUNC, script + " " + parameters.join(" "), eMax);

  if(!script.length())
  {
    error(FUNC, tr("No script name."));
    return 0;
  }

  if(mProc) delete mProc;

  if(mShowWaitWindow)
  {
    mProc = new QProcess;
    connect(mProc, SIGNAL(readyRead()), this, SLOT(readStdOut()));
    connect(mProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(returnResult()));

    mRunning = true;
    mProc->setWorkingDirectory(mWorkingDir);
    mProc->start(script, parameters);

    if(!mProc->waitForStarted())
    {
      fatal(FUNC, tr("Script not started: %1").arg(script));
      mRunning = false;
      mProc->kill();
    }

    return 0;
  }
  else
  {
    mProc = new QProcess;
    mProc->setWorkingDirectory(mWorkingDir);
    mProc->start(script, parameters);

    if(!mProc->waitForStarted())
    {
      fatal(FUNC, tr("Script '%1' not started.").arg(script));
      return 0;
    }

    if(!mProc->waitForFinished())
    {
      fatal(FUNC, tr("Script '%1' not finished.").arg(script));
      mProc->kill();
      return 0;
    }

    mResult = new QStringList;

    // Read the beef
    readStdOut();
    emit finished();
    return mResult;
  }
}

QStringList* Script::askProvider(const QString& provider
                                , const QString& function
                                , const QStringList& parameters)
{
  clearErrors();

  QString script = locateProviderScript(provider, function);

  if(script.isEmpty())
  {
    error(FUNC, tr("Provider '%1' has no script for function '%2'.").arg(provider, function));

    emit finished();

    return 0;
  }

  return execute(script, parameters);
}

void Script::stopRunning()
{
  if(!mProc) return;
  mProc->terminate();
  delete mProc;
  mProc = 0;
}

QString Script::locateProviderScript(const QString& provider, const QString& function)
{
  static QString oldProvider, oldFunction, oldFullPath;

  // Don't waste time
  if((provider == oldProvider) and (function == oldFunction)) return oldFullPath;

  // Build the fullpath to the provider script
  QString fullPath(mProviderPath + provider + "/");

  QDir dir(fullPath);
  dir.setNameFilters(QStringList(function + "*"));

  QStringList files = dir.entryList();
  if(files.isEmpty()) return QString();

  mWorkingDir = fullPath;

  fullPath.append(files.at(0));

  oldProvider = provider;
  oldFunction = function;
  oldFullPath = fullPath;

  return fullPath;
}

void Script::showWaitWindow(bool yes/* = true*/)
{
  mShowWaitWindow = yes;
}

void Script::readStdOut()
{
  if(mShowWaitWindow) mResult = new QStringList;

  while(mProc->canReadLine())
  {
    QString line = mProc->readLine();
    line.trimmed();
    if(line.startsWith("*")) continue;

    line.chop(1); // Remove newline char at the end
    if(line.isEmpty()) continue;

    mResult->append(line);
    //qDebug() << line;
  }

  if(mShowWaitWindow) emit newData(mResult);
}

void Script::returnResult()
{
  readStdOut();
  mRunning = false;
  mShowWaitWindow = false;
  emit finished();
}
