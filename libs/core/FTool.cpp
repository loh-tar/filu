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

#include "FTool.h"

#include <QRegExp>
#include <QDir>
#include <QFile>
#include <QDirIterator>

QString
FTool::lineToTxt(const QString& line)
{
  QString txt = line;

  // We have to restore newline and simicolon
  // The RegExp says: Search for '/,' but may not be leaded by '/' => ([^/])
  // If found replace with ';' and keep the leading char => \\1
  txt.replace(QRegExp("([^/])\\/,"), "\\1;");

  // Newline char works as same.
  txt.replace(QRegExp("([^/])\\/n"), "\\1\n");

  // Finaly replace all double "//"
  txt.replace("//", "/");

  return txt;
}

QString
FTool::txtToLine(const QString& txt)
{
  QString line = txt;

  // We have to replace all newline and semicolon.
  // We do it by replacing them with a string sequence of a escape char
  // and a substitude char. => '/'

  // But because the substitude char may also include in the text,
  // we have to escape them too.
  line.replace("/", "//");

  // Semicolon
  line.replace(";", "/,");

  // new line char
  line.replace(QString('\n'), "/n");

  return line;
}

void
FTool::strToAttributes(const QString& str, QHash<QString, QString>& attr)
{
  QStringList keyValueList = str.split('\n');

  foreach(QString keyValueStr, keyValueList)
  {
    int idx = keyValueStr.indexOf("=");
    QString name  = keyValueStr.left(idx);
    QString value = keyValueStr.remove(0, ++idx);

    value.replace("/n", QString('\n'));
    value.replace("/=", "=");
    value.replace("//", "/");

    attr.insert(name, value);
  }
}

int
FTool::getParameter(const QStringList& cmdLine, const QString& cmd, QStringList& parm)
{
  // Place the parameter to the command switch "--foo" into parm
  int pos = cmdLine.indexOf(cmd);

  if(-1 == pos) return -1; // Command not found. was not given on command line
  parm.clear();            // Be on the save side

  for(int i = pos + 1; i <= cmdLine.size() - 1; ++i)
  {
    if(cmdLine.at(i).startsWith("--")) break;
    parm.append(cmdLine.at(i));
  }

  return parm.size();
}

int
FTool::timeFrame(const QString& frame, bool trueDays/* = false*/)
{
  bool ok;
  int timeFrame = frame.toInt(&ok);
  if(ok)
  {
    if(timeFrame > 0) return timeFrame;
    return -1;
  }

  int week = 5;
  if(trueDays) week = 7;

  QString Frame = frame.toUpper();

  if(Frame == "DAY")          timeFrame = 1;
  else if(Frame == "WEEK")    timeFrame = week;
  else if(Frame == "MONTH")   timeFrame = week * 4;
  else if(Frame == "QUARTER") timeFrame = week * 4 * 3;
  else timeFrame = -1; // Error

  return timeFrame;
}

void
FTool::copyDir(const QString& src, const QString& dest)
{
  // Found at http://agnit.blogspot.com/2009/03/directory-copy-code.html
  // Thanks to Agnit Sarkar

  //Check whether the dir directory exists
  if(!QDir(src).exists()) return;

  if(!QDir(dest).exists())
  {
    QDir().mkpath(dest);
  }

  //Construct an iterator to get the entries in the directory
  QDirIterator dirIterator(src);

  while (dirIterator.hasNext())
  {
    QString       item = dirIterator.next();
    QString   fileName = dirIterator.fileName();
    QFileInfo fileInfo = dirIterator.fileInfo();

    if(fileName != "." && fileName != "..")
    {
      if(fileInfo.isFile())
      { //If entry is a file copy it
        QFile::copy(item, dest + "/" + fileName);
      }
      else
      { //If entry is a directory, call the deltree function over it again to traverse it
        copyDir(item, dest+ "/"+ fileName);
      }
    }
  }
}

QStringList
FTool::wrapText(const QString txt, int width)
{
  QStringList words = txt.split(" ");

  QString line;
  QStringList wraped;
  foreach(QString s, words)
  {
    s.append(" ");
    if(line.size() + s.size() > width)
    {
      line.chop(1); // Remove above added space
      wraped << line;
      line = s;
    }
    else
    {
      line.append(s);
    }
  }

  wraped << line;

  return wraped;
}
