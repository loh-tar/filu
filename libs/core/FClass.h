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
  Q_DECLARE_TR_FUNCTIONS(FClass)

  public:
                   FClass(const FClass* parent);
                   FClass(const QString& connectionName);
    virtual       ~FClass();

    enum MsgType
    {
      eNotice,
      eWarning,
      eError,
      eCritical
    };

    const QStringList& errorText() const { return mErrorMessage; };
    bool               hasError() const { return mHasError; };

  protected:
    void           addErrorText(const QStringList& errorMessage, MsgType type = eNotice);
    void           addErrorText(const QString& errorMessage, MsgType type = eNotice);
    void           removeErrorText(const QString& errorMessage);
    bool           check4FiluError(const QString& errMessage);  // True if error
    void           clearErrors();

    RcFile*        mRcFile;
    FiluU*         mFilu;
    int            mDebugLevel;

  private:
    QStringList    mErrorMessage;
    bool           mHasError;
    bool           mRoot;
};

#endif
