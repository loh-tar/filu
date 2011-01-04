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

#include "Newswire.h"
#include "RcFile.h"

Newswire::Newswire(const QString& connectionName)
        : mRoot(true)
        , mVerboseLevel(eNoVerbose)
        , mErrConsole(new QTextStream(stderr))
        , mLogFileFile(0)
        , mLogFile(0)
        , mRawFuncRegex("\\w+(?=::)")
        , mConnName(connectionName)

{}

Newswire::Newswire(const Newswire* parent)
        : mRoot(false)
        , mVerboseLevel(eNoVerbose)  // Don't parent->mVerboseLevel, or should?
        , mErrConsole(parent->mErrConsole)
        , mLogFileFile(0)
        , mLogFile(0)
        , mRawFuncRegex("\\w+::\\w+")
{}

Newswire::~Newswire()
{
  if(isRoot())
  {
    delete mErrConsole;
    if(mLogFileFile) delete mLogFileFile;
    if(mLogFile) delete mLogFile;
  }
}

void Newswire::setLogFile(const QString& path)
{
  if(!isRoot()) return;
  if(mLogFileFile) delete mLogFileFile;

  mLogFileFile = new QFile(path);
  if(!mLogFileFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
  {
    delete mLogFileFile;
    mLogFileFile = 0;
    fatal(FFI_, tr("Can't open log file '%1'").arg(path));
  }
  else
  {
    mLogFile = new QTextStream(mLogFileFile);
    verbose(FFI_, tr("Open log file '%1'").arg(path), eMax);
  }
}

void Newswire::setVerboseLevel(const VerboseLevel level)
{
  mVerboseLevel = level;
}

void Newswire::setVerboseLevel(const QString& func, const QString& level)
{
  bool ok;
  mVerboseLevel = VerboseLevel(level.toInt(&ok));

  if(ok and (mVerboseLevel >= eNoVerbose) and (mVerboseLevel <= eMax)) return;

  QString levelName = level.toUpper();
  if(levelName == "QUIET")     mVerboseLevel = eNoVerbose;
  else if(levelName == "INFO") mVerboseLevel = eInfo;
  else if(levelName == "MAX")  mVerboseLevel = eMax;
  else
  {
    error(func, tr("Verbose level unknown: %1").arg(level));
  }
}

void Newswire::errInfo(const QString& func, const QString& txt)
{
  addError(func, txt, eErrInfo);
  logError(func, txt, "Info ");
}

void Newswire::warning(const QString& func, const QString& txt)
{
  addError(func, txt, eWarning);
  logError(func, txt, "Warning");
}

void Newswire::error(const QString& func, const QString& txt)
{
  addError(func, txt, eError);
  logError(func, txt, "Error");
}

void Newswire::fatal(const QString& func, const QString& txt)
{
  addError(func, txt, eFatal);
  logError(func, txt, "Fatal");
}

void Newswire::setError(const QString& func, const QString& txt, const ErrorType type/* = eError*/)
{
  switch(type)
  {
    case eErrInfo: errInfo(func, txt); break;
    case eError:   error(func, txt);   break;
    case eWarning: warning(func, txt); break;
    case eFatal:   fatal(func, txt);   break;
    default:
      fatal(func, QString("Oops, unknown ErrorType: %1, ErrorText is: %2").arg(type).arg(txt));
  }
}

void Newswire::removeError(const QString& txt)
{
  int exist = -1;
  for(int i = 0; i < mErrors.size(); ++i)
  {
    if(mErrors.at(i).text != txt) continue;
    exist = i;
  }

  if(exist > -1) mErrors.removeAt(exist);

  if(mErrors.size() == 0) mHasError = false;
}
/*
bool Newswire::check4FiluError(const QString& errorMessage)
{
  if(mFilu->hadTrouble())
  {
    addErrorText(errorMessage, eCritical);
    addErrorText(mFilu->errorText(), eCritical);

    return true;
  }

  return false;
}

void Newswire::clearErrors()
{
  mErrors.clear();
  mHasError = false;
}
*/

void Newswire::verboseP(const QString& func, const QString& txt, const VerboseLevel type/* = eInfo*/)
{
  *mErrConsole << rawFunc(func) << " " << txt << endl;
}

void Newswire::addError(const QString& func, const QString& txt, const ErrorType type)
{
  mHasError = true;

  bool exist = false;
  for(int i = 0; i < mErrors.size(); ++i)
  {
    if(mErrors.at(i).text != txt) continue;
    exist = true;
  }

  Error error = { rawFunc(func), txt, type };

  mErrors.append(error);
}

void Newswire::logError(const QString& func, const QString& txt, const QString& type)
{
  *mErrConsole << QString("%1 *** %3 *** %2").arg(rawFunc(func), txt, type) << endl;

  if(mLogFile) *mLogFile << QString("%4 *** %3 *** %1 %2").arg(rawFunc(func), txt, type, mConnName) << endl;
}
