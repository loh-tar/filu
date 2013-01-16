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

#include <QString>

#include "FClass.h"

#include "FiluU.h"
#include "RcFile.h"

FClass::FClass(FClass* parent, const QString& className)
      : Newswire(parent, className)
      , mRcFile(parent->mRcFile)
      , mFilu(parent->mFilu)
{}

FClass::FClass(const QString& connectionName)
      : Newswire(connectionName)
      , mRcFile(new RcFile(this))
      , mFilu(0)
{
  mFilu = new FiluU(connectionName, mRcFile);
}

FClass::~FClass()
{
  if(isRoot())
  {
    delete mFilu;
    delete mRcFile;
  }
}

bool FClass::check4FiluError(const QString& func, const QString& txt/* = ""*/, const MsgType type/* = eError*/)
{
  if(mFilu->hasMessage())
  {
    if(txt.size()) setMessage(func, txt, type);
    errInfo(func, tr("Filu says..."));
    addErrors(mFilu->errors());
    return true;
  }

  return false;
}
