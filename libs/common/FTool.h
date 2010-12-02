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

#ifndef FTOOL_HPP
#define FTOOL_HPP

#include <QtCore>

namespace FTool
{

//
// Here is a collection of more or less simple functions
// which are to small to create an own class FIXME:or should?
//

// A un/packer like pair of functions.
// Replace/Restore newline and ';' char in a string
// Used by Importer/Exporter
QString       lineToTxt(const QString& line);
QString       txtToLine(const QString& txt);

// A simple command line options handler.
// Places the parameter to switch cmd into parm.
// Retruns -1 if cmd not found or the number of cmd arguments
// FIXME: not used, create an own class with more features
int           getParameter( const QStringList& cmdLine
                          , const QString& cmd
                          , QStringList& parm );

// Yes, it copy a whole dir tree
void          copyDir(const QString& src, const QString& dest);

}
#endif
