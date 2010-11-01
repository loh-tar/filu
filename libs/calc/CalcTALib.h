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

#ifndef CALCTALIB_HPP
#define CALCTALIB_HPP

#include "CalcType.h"
#include "TALib.h"
#include "ta_abstract.h"

/***********************************************************************
*
* Yes, calculate TA-Lib functions
*
************************************************************************/

class CalcTALib : public CalcType
{
  public:
                  CalcTALib(Indicator* parent);
    virtual      ~CalcTALib();

    virtual bool  prepare(CalcParms& parms);
    virtual bool  calc();
    virtual int   barsNeeded(DataTupleSet* data);

  protected:
    bool          initTALib(TA_ParamHolder **parmHolder);
    void          freesParmHolder(TA_ParamHolder *parmHolder);
    void          setFirstValid();
    TA_Real*      valueArray(const QString& key);

    FunctionInfo            mInfo;
    QHash<QString, QString> mInput;
    QHash<QString, QString> mOutput;
    QHash<QString, double>  mOptInput;
    int                     mFirstValid;
};

#endif
