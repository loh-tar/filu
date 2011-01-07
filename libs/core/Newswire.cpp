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

#include "Newswire.h"
#include "RcFile.h"

Newswire::Newswire(const QString& connectionName)
        : mRoot(true)
        , mVerboseLevel(eNoVerbose)
        , mHasError(false)
        , mErrConsole(new QTextStream(stderr))
        , mLogFileFile(0)
        , mLogFile(0)
        , mNoErrorLogging(false)
        , mRawFuncRegex("\\w+(?=::)")
        , mConnName(connectionName)

{}

Newswire::Newswire(Newswire* parent)
        : mRoot(false)
        , mVerboseLevel(eNoVerbose)  // Don't parent->mVerboseLevel, or should?
        , mHasError(false)
        , mErrConsole(parent->mErrConsole)
        , mLogFileFile(0)
        , mLogFile(0)
        , mNoErrorLogging(false)
        , mRawFuncRegex("\\w+::\\w+")
{}

Newswire::~Newswire()
{
  if(isRoot())
  {
    delete mErrConsole;
    if(mLogFileFile) delete mLogFileFile;
    if(mLogFile) delete mLogFile;
  }
}

void Newswire::setLogFile(const QString& path)
{
  if(!isRoot()) return;
  if(mLogFileFile) delete mLogFileFile;

  mLogFileFile = new QFile(path);
  if(!mLogFileFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
  {
    delete mLogFileFile;
    mLogFileFile = 0;
    fatal(FFI_, tr("Can't open log file '%1'").arg(path));
  }
  else
  {
    mLogFile = new QTextStream(mLogFileFile);
    verbose(FFI_, tr("Open log file '%1'").arg(path), eMax);
  }
}

void Newswire::setVerboseLevel(const VerboseLevel level)
{
  mVerboseLevel = level;
}

void Newswire::setVerboseLevel(const QString& func, const QString& level)
{
  bool ok;
  mVerboseLevel = VerboseLevel(level.toInt(&ok));

  if(ok and (mVerboseLevel >= eNoVerbose) and (mVerboseLevel <= eMax)) return;

  QString levelName = level.toUpper();
  if(levelName == "QUIET")     mVerboseLevel = eNoVerbose;
  else if(levelName == "INFO") mVerboseLevel = eInfo;
  else if(levelName == "MAX")  mVerboseLevel = eMax;
  else
  {
    error(func, tr("Verbose level unknown: %1").arg(level));
  }
}

void Newswire::setRawFuncRegex(const QRegExp& regex)
{
  mRawFuncRegex = regex;
}

void Newswire::setNoErrorLogging(bool noErrorLogging)
{
  mNoErrorLogging = noErrorLogging;
}

QString Newswire::formatErrors(const QString& format/* = "%f *** %t *** %x"*/)
{
  QString errors;
  foreach(Message error, mErrors)
  {
    errors.append(formatMessage(error.func, error.text, error.type, format) + "\n");
  }

  errors.chop(1); // Remove last newline
  return errors;
}

QString Newswire::formatMessage(const QString& func, const QString& txt, const MsgType type, const QString& format/* = "%f *** %t *** %x"*/)
{
  QString message = format;

  message.replace("%f", rawFunc(func));
  message.replace("%t", messageTypeName(type));
  message.replace("%c", mConnName);
  message.replace("%x", txt); // Replace at last, maybe is '%foo' in txt
//   message.replace("%d", QDate...);
//   message.replace("", );

  return message;
}

QString Newswire::messageTypeName(const MsgType type)
{
  switch(type)
  {
    case eInfoMsg: return tr("Info");    break;
    case eErrInfo: return tr("Info ");   break; // Um, yes one blank to became same lenght as "Error"
    case eError:   return tr("Error");   break;
    case eWarning: return tr("Warning"); break;
    case eFatal:   return tr("Fatal");   break;
    default:       return "UnknownMessageTypeName"; // Um, yes no tr(), should you never read
  }
}

void Newswire::addErrors(const MessageLst& errors)
{
  foreach(Message error, errors)
  {
    for(int i = 0; i < mErrors.size(); ++i)
    {
      if(mErrors.at(i).text != error.text) continue;
      return;
    }

    mErrors.append(error);
    mHasError = true;
  }
}

void Newswire::errInfo(const QString& func, const QString& txt)
{
  addError(func, txt, eErrInfo);
  logError(func, txt, eErrInfo);
}

void Newswire::warning(const QString& func, const QString& txt)
{
  addError(func, txt, eWarning);
  logError(func, txt, eWarning);
}

void Newswire::error(const QString& func, const QString& txt)
{
  addError(func, txt, eError);
  logError(func, txt, eError);
}

void Newswire::fatal(const QString& func, const QString& txt)
{
  addError(func, txt, eFatal);
  logError(func, txt, eFatal);
}

void Newswire::setMessage(const QString& func, const QString& txt, const MsgType type/* = eError*/)
{
  switch(type)
  {
    case eInfoMsg: verbose(func, txt, eInfo); break;
    case eErrInfo: errInfo(func, txt); break;
    case eError:   error(func, txt);   break;
    case eWarning: warning(func, txt); break;
    case eFatal:   fatal(func, txt);   break;
    default:
      fatal(func, QString("Oops, unknown MsgType: %1, ErrorText is: %2").arg(type).arg(txt));
  }
}

void Newswire::removeError(const QString& txt)
{
  int exist = -1;
  for(int i = 0; i < mErrors.size(); ++i)
  {
    if(mErrors.at(i).text != txt) continue;
    exist = i;
  }

  if(exist > -1) mErrors.removeAt(exist);

  if(mErrors.size() == 0) mHasError = false;
}

void Newswire::clearErrors()
{
  mErrors.clear();
  mHasError = false;
}

void Newswire::verboseP(const QString& func, const QString& txt, const VerboseLevel type/* = eInfo*/)
{
  *mErrConsole << rawFunc(func) << " " << txt << endl;
}

void Newswire::addError(const QString& func, const QString& txt, const MsgType type)
{
  for(int i = 0; i < mErrors.size(); ++i)
  {
    if(mErrors.at(i).text != txt) continue;
    return;
  }

  Message error = { rawFunc(func), txt, type };

  mErrors.append(error);
  mHasError = true;
}

void Newswire::logError(const QString& func, const QString& txt, const MsgType type)
{
  if(!mNoErrorLogging)
  {
    *mErrConsole << formatMessage(func, txt, type) << endl;
  }

  if(!mNoErrorLogging or (type == eFatal))
  {
    if(mLogFile) *mLogFile << formatMessage(func, txt, type, "%c *** %t *** %f %x") << endl;
  }
}
