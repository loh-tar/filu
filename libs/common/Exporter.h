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

#ifndef EXPORTER_HPP
#define EXPORTER_HPP

#include <QtCore>

#include "FClass.h"

class FiluU;

/***********************************************************************
*
* File created at 2010-03-15
*
* Yes, he exports stuff out of the DB into a plain text file.
* He is almost used by "agentf -exp ..." command. Options are
*
************************************************************************/

class Exporter : public FClass
{
  public:
                Exporter(FClass* parent);
//                 Exporter(FiluU* filu);
    virtual    ~Exporter();

    void        reset();
    bool        exxport(QStringList& command);

  protected:
    int         getParameter(const QString& command, QStringList& parm);  // -1 cmd not given, 0..n parm count

    bool        expFiTypes();
    bool        expSymbolTypes();
    bool        expMarkets();
    bool        expFiNames();
    bool        expSymbols();
    bool        expEODRaw();
    bool        expSplits();
    //TODO: bool expUnderlyings();

    // user data exports
    bool        expCOs();
    bool        expGroups();

    QStringList    mCommandLine;  // yes, the command line with all options

    QTextStream*   mOutput;
    QTextStream    mConsole;
    QFile*         mOutFile;

  private:

};

#endif
