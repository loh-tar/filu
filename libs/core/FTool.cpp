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

#include <QRegExp>
#include <QDir>
#include <QFile>
#include <QDirIterator>
// #include <QDebug>

#include "FTool.h"

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

  foreach(const QString& keyValueStr, keyValueList)
  {
    int idx = keyValueStr.indexOf("=");
    QString name  = keyValueStr.left(idx);
    QString value = keyValueStr.right(keyValueStr.size() - (++idx));

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

bool
FTool::removeDir(const QString &dirName)
{
  // Found at http://john.nachtimwald.com/2010/06/08/qt-remove-directory-and-its-contents/
  // Thanks to John Nachtimwald

  bool result = true;
  QDir dir(dirName);

  if(dir.exists(dirName))
  {
    foreach(const QFileInfo& info
          , dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden
                            | QDir::AllDirs        | QDir::Files, QDir::DirsFirst))
    {
      if(info.isDir())
      {
        result = removeDir(info.absoluteFilePath());
      }
      else
      {
        result = QFile::remove(info.absoluteFilePath());
      }

      if(!result)
      {
        return result;
      }
    }

    result = dir.rmdir(dirName);
  }

  return result;
}

QStringList
FTool::wrapText(const QString txt, int width)
{
  QStringList words = txt.split(" ");

  QString line;
  QStringList wraped;
  foreach(const QString& s, words)
  {
    if(line.size() + s.size() + 1 > width)
    {
      line.chop(1); // Remove added space
      wraped << line;
      line = s + " ";
    }
    else
    {
      line.append(s + " ");
    }
  }

  wraped << line;

  return wraped;
}

QStringList
FTool::breakUpText(const QString txt, bool /*ignoreQuotes = true*/)
{
  QRegExp openBrace("[\\(\\[\\{]");
  QRegExp closeBrace("[\\}\\]\\)]");
  int openBraceCount  = 0;
  int closeBraceCount = 0;

  QStringList rawWords = txt.split(" ", QString::SkipEmptyParts);
  QStringList fractions;
  QStringList parts;
  foreach(const QString& s,rawWords)
  {
    parts.append(s);

    // Sadly does QRegExp::captureCount() not work as I expect.
    // That's why we have to count braces manually
    int pos = 0;
    while((pos = openBrace.indexIn(s, pos)) != -1) { ++openBraceCount; ++pos; }
    pos = 0;
    while((pos = closeBrace.indexIn(s, pos)) != -1) { ++closeBraceCount; ++pos; }

    if(openBraceCount == closeBraceCount)
    {
      fractions.append(parts.join(" "));
      parts.clear();
      openBraceCount  = 0;
      closeBraceCount = 0;
    }
  }

  return fractions;
}

QStringList
FTool::formatToTable(const QStringList& data, int width/* = 100*/, int indent/* = 2*/)
{
  TableOptions opts;
  opts.insert(eWidth, width);
  opts.insert(eIndent, indent);

  return formatToTable(data, opts);
}

QStringList
FTool::formatToTable(const QStringList& data, const TableOptions& opts)
{
  if(!data.size()) return data;

  // Calc some facts, not all are currently used
  int minColWidth  = FTool::maxSizeOfStrings(data) + 1;
      minColWidth  = qMax(minColWidth, opts.value(eMinColWidth));
  int optiColWidth = minColWidth * 1.5;
  int width        = opts.value(eWidth, 80);
      width       -= opts.value(eIndent, 2);
  int maxColCount  = static_cast<double>(width) / static_cast<double>(minColWidth) + 1.0;
  //int minRowCount  = static_cast<double>(data.size()) / static_cast<double>(maxColCount) + 1.0;
  //int maxRowCount  = data.size();
  int optiColCount = static_cast<double>(width) / static_cast<double>(optiColWidth) + 1.0;

  int columns;
  int rows;
  if(opts.contains(eColumns))
  {
    columns = qBound(1, opts.value(eColumns), maxColCount); // Not qMin() to catch given zero
    rows    = static_cast<double>(data.size()) / static_cast<double>(columns) + 1.0;
  }
  else
  {
    // Reduce column count until we have not so much
    // more columns than rows
    columns = optiColCount;
    forever
    {
      rows = static_cast<double>(data.size()) / static_cast<double>(columns) + 1.0;
      if(columns <= 2 * rows + 1) break;
      --columns;
    }
  }

  int maxColWidth  = width / columns;
  int colWidth     = qBound(minColWidth, opts.value(eColWidth, optiColWidth), maxColWidth);

  QStringList table;
  QString lineMask = "%1";
  for(int i = 0; i < opts.value(eIndent, 2); ++i) lineMask.prepend(" ");

  if(opts.contains(eLeftRight))
  {
    // Sort data left->right and up->down
    for(int i = 0; i < data.size(); i += columns)
    {
      QString line;
      for(int j = 0; j < columns; ++j)
      {
        if(data.size() < i + j + 1) { break; }
        line.append(QString("%1").arg(data.at(i + j), -colWidth));
      }

      table.append(lineMask.arg(line));
    }
  }
  else
  {
    // Sort data up->down and left->right
    for(int i = 0; i < rows; ++i)
    {
      QString line;
      for(int j = 0; j < data.size(); j += rows)
      {
        if(data.size() < i + j + 1) { break; }
        line.append(QString("%1").arg(data.at(i + j), -colWidth));
      }

      table.append(lineMask.arg(line));
    }
  }

  for(int i = 0; i < opts.value(eAddEmptyLines); ++i) table.append("");

  return table;
}

int
FTool::maxSizeOfStrings(const QStringList& sl)
{
  int max = 0;
  foreach(const QString& s, sl) max = qMax(max, s.size());
  return max;
}

QString
FTool::makeValidWord(const QString& s)
{
  QString w = s;
  w.replace(QRegExp("\\W"), "_");                 // Replace all non "word" character
  while(w.contains("__")) w.replace("__", "_");   // Remove doubled underscores
  if(w.contains(QRegExp("^\\d"))) w.prepend("_"); // Don't start with a digit
  return w;
}

QString
FTool::number(const QString& s)
{
  if(s.isEmpty()) return "0";

  QString n = s;
  QRegExp rx("\\d+,\\d+$");
  if(rx.indexIn(n) != -1)
  {
    // s looks like "12.345,67"
    n.remove('.');
    n.replace(',', '.');
  }
  else
  {
    // Perhaps s looks like "12,345.67"
    n.remove(',');
  }

  return n;
}

void
FTool::sleep(int ms)
{
  // http://stackoverflow.com/questions/3752742/how-do-i-create-a-pause-wait-function-using-qt
  struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
  nanosleep(&ts, NULL);
}

/***********************************************************************
*   Ask User Function Stuff
*
*   Quick hacked. I'm sure it could be done more elegant e.g. by using
*   QFLags as function parameter so that there is no need to have
*   different functions for each use case but only one
*
************************************************************************/
#include <QTextStream>

const QString cYes  = QObject::tr("Yes");
const QString cNo   = QObject::tr("No");

QString
FTool::askUser(const QString& question)
{
  QTextStream out(stdout);
  out << question << flush;

  QTextStream in(stdin);
  QString answer;
  answer = in.readLine(100);

  return answer;
}

bool
FTool::askUserNoYes(const QString& question)
{
  QString text = QString("%1 %2/[%3] : ").arg(question).arg(cNo, cYes);

  QString answer = askUser(text);

  if(answer.isEmpty()) return true;
  if(cNo.startsWith(answer, Qt::CaseInsensitive)) return false;

  return true;
}

bool
FTool::askUserYesNo(const QString& question)
{
  QString text = QString("%1 %2/[%3] : ").arg(question).arg(cYes, cNo);

  QString answer = askUser(text);

  if(answer.isEmpty()) return true;
  if(cYes.startsWith(answer, Qt::CaseInsensitive)) return false;

  return true;
}

int
FTool::askUserInt(const QString& question)
{
  QString answer = askUser(question);

  bool ok;
  int num = answer.toInt(&ok);

//   if(!ok) return def;

  return num;
}

int
FTool::askUserInt(const QString& question, int def)
{
  QString text = QString("%1 [%2] : ").arg(question).arg(def);

  QString answer = askUser(text);
  if(answer.isEmpty()) return def;

  bool ok;
  int num = answer.toInt(&ok);

  if(!ok) return def;

  return num;
}

double
FTool::askUserDouble(const QString& question, double def)
{
  QString text = QString("%1 [%2] : ").arg(question).arg(def);

  QString answer = askUser(text);

  bool ok;
  double num = answer.toDouble(&ok);

  if(!ok) return def;

  return num;
}

QString
FTool::askUserStr(const QString& question, const QString& def)
{
  QString text = QString("%1 [%2] : ").arg(question, def);

  return askUser(text);
}
/***********************************************************************
*   End Of Ask User Function Stuff
************************************************************************/
