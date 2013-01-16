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

#ifndef CALCTYPE_HPP
#define CALCTYPE_HPP

#include <QSet>

#include "FClass.h"
class DataTupleSet;
class Indicator;

/***********************************************************************
*
*   This is the base class of all kind of calc objects
*
************************************************************************/

// Not as lucky that name, looks like a CalcType too but is only
// the parameter container a CalcType will get
class CalcParms : public QPair<QStringList, QStringList>
{
  public:
    void              setIns(const QStringList& i)  { first = i; };
    void              setOuts(const QStringList& o) { second = o; };

    QStringList&      ins()  { return first; };
    QStringList&      outs() { return second; };
};

class CalcType : public FClass
{
  public:
                      CalcType(Indicator* parent, const QString& className);
    virtual          ~CalcType();

    static  CalcType* createNew(Indicator* indi, const QString& type);
    virtual bool      prepare(CalcParms& parms);
    virtual bool      calc();
    virtual int       barsNeeded(DataTupleSet* data);
            QString   getType();

  protected:
    void              init(CalcParms& parms);
    bool              isUnknown(const QString& key);
    bool              checkOutputCount(int count);
    bool              checkInputCount(int count);
    bool              checkInputVariable(int i);
    bool              addToDataSet(const QString& key);
    void              getIndicatorVariables();

    QString           mType;
    QStringList       mOuts;
    QStringList       mIns;

    Indicator*        mIndicator;
    DataTupleSet*     mData;
    QSet<QString>*    mUsedVariables;
    //QString           mViewName;

  private:

};

#endif
