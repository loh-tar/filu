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

#ifndef CALCFIMI_HPP
#define CALCFIMI_HPP

#include <QtSql>

#include "CalcType.h"

//
//  Yes, calculate the Filu Power Indicators
//

class CalcFilu : public CalcType
{
  public:
                  CalcFilu(Indicator* parent);
    virtual      ~CalcFilu();

    virtual bool  prepare(CalcParms& parms);
    virtual bool  calc();
    virtual int   barsNeeded(DataTupleSet* data);

  protected:
    QStringList   mInput;          // Holds the input variable names used by the function
    QStringList   mFiRefAlias;
    int           mOutputCount;
    QStringList   mIndiErrorText;
    bool          mPrepareError;
};

#endif