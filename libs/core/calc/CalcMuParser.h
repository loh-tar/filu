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

#ifndef CALCMUPARSER_HPP
#define CALCMUPARSER_HPP

#include "CalcType.h"
#include "MyParser.h"

/***********************************************************************
*
*   Yes, calculate expressions using muParser
*
************************************************************************/

class CalcMuParser : public CalcType
{
  public:
                  CalcMuParser(Indicator* parent);
    virtual      ~CalcMuParser();

    virtual bool  prepare(CalcParms& parms);
    virtual bool  calc();

  protected:
    MyParser*     mParser;
    QSet<QString> mParserVariables;
};

#endif
