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

#ifndef FCLASS_HPP
#define FCLASS_HPP

#include <QtCore>

#include "Newswire.h"
#include "FiluU.h"
#include "RcFile.h"

/***********************************************************************
*
*   The base of all classes which use Filu
*
************************************************************************/

class FClass : public Newswire
{
  public:
                   FClass(FClass* parent, const QString& className);
                   FClass(const QString& connectionName);
    virtual       ~FClass();

  protected:
    bool           check4FiluError(const QString& func, const QString& txt, const MsgType type = eError);  // True if error

    RcFile*        mRcFile;
    FiluU*         mFilu;
    int            mDebugLevel;

  private:
    bool           mRoot;
};

#endif
