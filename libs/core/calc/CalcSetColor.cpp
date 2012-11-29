//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012  loh.tar@googlemail.com
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

#include <QColor>

#include "CalcSetColor.h"

#include "DataTupleSet.h"

CalcSetColor::CalcSetColor(Indicator* parent)
            : CalcType(parent, FUNC)
{
  mType = "SetColor";
}

CalcSetColor::~CalcSetColor()
{}

bool CalcSetColor::prepare(CalcParms& parms)
{
  init(parms);
  checkOutputCount(1);

  if(1 == mIns.size())
  {
    // Check format "<Foo> = SETCOLOR(<Color>)"
    // mIns look like: "red"
    QColor c;
    c.setNamedColor(mIns.at(0));
    if(!c.isValid())
    {
      error(FUNC, tr("Color '%1' not valid.").arg(mIns.at(0)));
    }
  }
  else if(3 == mIns.size())
  {
    // Check format "<Foo> = SETCOLOR(<Variable>, <TrueColor>, <FalseColor>)"
    // mIns look like: "winday", "green", "red"

    checkInputVariable(0);

    QColor c;
    c.setNamedColor(mIns.at(1));
    if(!c.isValid())
    {
      error(FUNC, tr("Color No.1 '%1' not valid.").arg(mIns.at(1)));
    }

    c.setNamedColor(mIns.at(2));
    if(!c.isValid())
    {
      error(FUNC, tr("Color No.2 '%1' not valid.").arg(mIns.at(2)));
    }
  }
  else
  {
    error(FUNC, tr("Wrong parameter count."));
  }

  if(hasError()) return false;

  return true;
}

bool CalcSetColor::calc()
{
  if(hasError()) return false;

  getIndicatorVariables();

  mData->rewind();
  if(1 == mIns.size())
  {
    // Format "<Foo> = SETCOLOR(<Color>)"
    while(mData->next())
    {
      mData->setColor(mOuts.at(0), mIns.at(0));
    }
  }
  else
  {
    // Format "<Foo> = SETCOLOR(<Variable>, <TrueColor>, <FalseColor>)"
    double var;
    while(mData->next())
    {
      if(!mData->getValue(mIns.at(0), var)) {/* error */}

      if(var > 0.0) mData->setColor(mOuts.at(0), mIns.at(1));
      else mData->setColor(mOuts.at(0), mIns.at(2));
    }
  }

  return true;
}
