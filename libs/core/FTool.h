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

#ifndef FTOOL_HPP
#define FTOOL_HPP

#include <QHash>
#include <QString>
#include <QStringList>

/***********************************************************************
*
*   Here is a collection of more or less simple functions
*   which are to small to create an own class FIXME:or should?
*
************************************************************************/

namespace FTool
{
// A un/packer like pair of functions.
// Replace/Restore newline and ';' char in a string
// Used by Importer/Exporter
QString
lineToTxt(const QString& line);

QString
txtToLine(const QString& txt);

// Was native part of COType.
// Its moved in here because we need it also at CalcWatchDogs which
// is not part of Gui ==> remove the dependency between Core/Gui.
void
strToAttributes(const QString& str, QHash<QString, QString>& attr);

// A simple command line options handler.
// Places the parameter to switch cmd into parm.
// Retruns -1 if cmd not found or the number of cmd arguments
int
getParameter(const QStringList&  cmdLine, const QString&  cmd, QStringList&  parm);

// Convert a named time frame to a number or -1 if unknown.
// When trueDays = true is calculated with week=7 days, if false week=5 days
int
timeFrame(const QString& frame, bool trueDays = false);

// Yes, it copy a whole dir tree
void
copyDir(const QString& src, const QString& dest);

// Wrap a text to given width
QStringList
wrapText(const QString txt, int width);

// Break the text in connected parts. Mostly single words but respect any kind
// of braces and quotes
QStringList
breakUpText(const QString txt, bool ignoreQuotes = true);

// Format data into lines to build a table with max width
QStringList
formatToTable(const QStringList& data, int width
            , const QHash<QString, int>& options = QHash<QString, int>());

// Retruns the size of the longest string in given stringlist
int
maxSizeOfStrings(const QStringList& sl);

// Retruns a string where all not valid word characters are replaced
// by an underscore "_"
QString
makeValidWord(const QString& s);

}
#endif
