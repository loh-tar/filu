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

#ifndef MYPARSER_HPP
#define MYPARSER_HPP

#include <QtCore>
#include "muParser.h"
#include "Newswire.h"

#include "DataTupleSet.h"

/***********************************************************************
*
*
*
************************************************************************/

class MyParser : public Newswire
{
  public:
                MyParser(Newswire* parent);
    virtual    ~MyParser();

    bool        setExp(const QString& expr);
    QString     getExp();
    void        useVariables(QHash<const QString, double>* variables);
    void        useData(DataTupleSet* data);
    void        appendUsedVariables(QSet<QString>& variables);
    int         calc(double& result);

  protected:
    QHash<const QString, double>* mVariables;

    bool            mMyVariables;
    DataTupleSet*   mData;
    QSet<QString>*  mUsedMData; // Holds name of variables to be read from mData
    mu::Parser      mParser;
};
#endif
