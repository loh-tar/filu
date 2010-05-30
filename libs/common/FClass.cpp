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

#include "FClass.h"

FClass::FClass(FClass* parent)
      : mRcFile(parent->getRcFile())
      , mFilu(parent->getFilu())
      , mHasError(false)
      , mRoot(false)
{
  mDebugLevel = mRcFile->getIT("DebugLevel");
}

FClass::FClass(const QString& connectionName)
      : mRcFile(new RcFile())
      , mFilu(new FiluU(connectionName, mRcFile))
      , mHasError(false)
      , mRoot(true)
{
  mDebugLevel = mRcFile->getIT("DebugLevel");
}

FClass::~FClass()
{
  if(mRoot)
  {
    delete mFilu;
    delete mRcFile;
  }
}

FiluU* FClass::getFilu()
{
  return mFilu;
}

RcFile* FClass::getRcFile()
{
  return mRcFile;
}

void FClass::addErrorText(const QStringList& errorMessage, int type/* = eNotice*/)
{
  for(int i = 0; i < errorMessage.size(); i++)
  {
    addErrorText(errorMessage.at(i), type);
  }
}

void FClass::addErrorText(const QString& errorMessage, int type/* = eNotice*/)
{
  mHasError = true;

  QString errMsg = errorMessage;
  if(mDebugLevel < 1) errMsg.remove(QRegExp("^\\w+::\\w+: "));

  if(!mErrorMessage.contains(errMsg))
    mErrorMessage.append(errMsg);

  if((type == eCritical) or (mDebugLevel == 2)) qDebug() << errorMessage;
}

void FClass::removeErrorText(const QString& errorMessage)
{
  QString errMsg = errorMessage;
  if(mDebugLevel < 1) errMsg.remove(QRegExp("^\\w+::\\w+: "));

  int exist = mErrorMessage.indexOf(errMsg);
  if(exist > -1) mErrorMessage.removeAt(exist);

  if(mErrorMessage.size() == 0) mHasError = false;
}

bool FClass::check4FiluError(const QString& errorMessage)
{
  if(mFilu->hadTrouble())
  {
    addErrorText(errorMessage, eCritical);
    addErrorText(mFilu->errorText(), eCritical);

    return true;
  }

  return false;
}

void FClass::clearErrors()
{
  mErrorMessage.clear();
  mHasError = false;
}
