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

Newswire::Newswire()
        : mVerboseLevel(eNoVerbose)
{}

Newswire::~Newswire()
{}

void Newswire::setVerboseLevel(const NewsType level)
{
  mVerboseLevel = level;
}

void Newswire::setVerboseLevel(const QString& func, const QString& level)
{
  bool ok;
  mVerboseLevel = NewsType(level.toInt(&ok));

  if(ok and (mVerboseLevel >= eNoVerbose) and (mVerboseLevel <= eMax)) return;

  QString levelName = level.toUpper();
  if(levelName == "QUIET")     mVerboseLevel = eNoVerbose;
  else if(levelName == "INFO") mVerboseLevel = eInfo;
  else if(levelName == "MAX")  mVerboseLevel = eMax;
  else
  {
    setError(func, tr("Verbose level unknown: %1").arg(level), eError);
  }
}

void Newswire::verbose(const QString& func, const QString& txt, const NewsType type/* = eInfo*/)
{
  if(mVerboseLevel < type) return;

  qDebug() << buildFullText(func, txt);
}

void Newswire::setError(const QString& func, const QString& txt, const NewsType type/* = eError*/)
{
  mHasError = true;

  QString errMsg;
  if(mVerboseLevel == eMax) errMsg = txt;
  else errMsg = buildFullText(func, txt);

  if(!mErrorMessage.contains(errMsg))  mErrorMessage.append(errMsg);

  if(type == eCritical)
  {
    qDebug() << "*";
    qDebug() << "*** Critical ***" << errMsg;
    qDebug() << "*";
  }
  else if(type == eError)
  {
    qDebug() << "*";
    qDebug() << "*** Error ***" << errMsg;
    qDebug() << "*";
  }
  else if(mVerboseLevel == eMax)
  {
    qDebug() << errMsg;
  }
}

void Newswire::removeError(const QString& txt)
{
  int exist = mErrorMessage.indexOf(txt);
  if(exist > -1) mErrorMessage.removeAt(exist);

  if(mErrorMessage.size() == 0) mHasError = false;
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
  mErrorMessage.clear();
  mHasError = false;
}*/
