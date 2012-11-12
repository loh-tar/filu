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

#include "Newswire.h"

#include "RcFile.h"
#include "FTool.h"

Newswire::Newswire(const QString& connectionName)
        : mRoot(true)
        , mConnName(connectionName)
        , mClass(connectionName)
        , mVerboseLevel(eNoVerbose)
        , mHasError(false)
        , mHasFatal(false)
        , mErrConsole(new QTextStream(stderr))
        , mLogFileFile(0)
        , mLogFile(0)
        , mNoErrorLogging(false)
{
  init();
}

Newswire::Newswire(Newswire* parent, const QString& className)
        : mRoot(false)
        , mConnName(parent->mConnName)
        , mClass(className)
        , mVerboseLevel(parent->mVerboseLevel)
        , mHasError(false)
        , mHasFatal(false)
        , mErrConsole(parent->mErrConsole)
        , mLogFileFile(0)
        , mLogFile(0)
        , mNoErrorLogging(false)
{
  init();
}

Newswire::~Newswire()
{
  if(isRoot())
  {
    delete mErrConsole;
    if(mLogFileFile) delete mLogFileFile;
    if(mLogFile) delete mLogFile;
  }
}

void Newswire::init()
{
  mFormat.insert(eVerbose, "%F %x");
  mFormat.insert(eConsLog, "%F *** %t *** %x");
  mFormat.insert(eFileLog, "%T %C *** %t *** %F %x");
  mFormat.insert(eErrFunc, "%f *** %t *** %x");
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
    fatal(FUNC, QString("Can't open log file '%1'.").arg(path));
  }
  else
  {
    mLogFile = new QTextStream(mLogFileFile);
    verbose(FUNC, tr("Open log file '%1'.").arg(path), eMax);
  }
}

void Newswire::setMsgTargetFormat(MsgTarget target, const QString& format)
{
  mFormat.insert(target, format);
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
  if(levelName == "QUIET")      mVerboseLevel = eNoVerbose;
  else if(levelName == "INFO")  mVerboseLevel = eInfo;
  else if(levelName == "AMPLE") mVerboseLevel = eAmple;
  else if(levelName == "MAX")   mVerboseLevel = eMax;
  else
  {
    error(func, tr("Verbose level '%1' unknown.").arg(level));
  }
}

void Newswire::setVerboseLevel(const QString& func, const QStringList& parm)
{
  QStringList level;
  if(FTool::getParameter(parm, "--verbose", level) < 1) return; // We ignore that fault

  setVerboseLevel(func, level.at(0));
}

void Newswire::setNoErrorLogging(bool noErrorLogging)
{
  mNoErrorLogging = noErrorLogging;
}

QString Newswire::formatMessages(const QString& format/* = ""*/)
{
  QString useFormat = format;
  if(useFormat.isEmpty()) useFormat = mFormat.value(eErrFunc);

  QString msgs;
  foreach(Message msg, mMessages)
  {
    msgs.append(formatMessage(msg, useFormat) + "\n");
  }

  msgs.chop(1); // Remove last newline
  return msgs;
}

QString Newswire::formatMessage(const Message& msg, const QString& format/* = ""*/)
{
  QString message = format;
  if(message.isEmpty()) message = mFormat.value(eErrFunc);

  message.replace("%F", QString("%1::%2:").arg(msg.clas, msg.func));
  message.replace("%c", msg.clas);
  message.replace("%f", msg.func);
  message.replace("%t", messageTypeName(msg.type));
  message.replace("%C", mConnName);
  message.replace("%x", msg.text); // Replace at last, maybe is '%foo' in txt
  message.replace("%D", QDate::currentDate().toString(Qt::ISODate));
  message.replace("%T", QTime::currentTime().toString(Qt::ISODate));
//   message.replace("", );

  return message;
}

Newswire::Message Newswire::makeMessage(const QString& func, const QString& txt, const MsgType type)
{
  Message msg = { mClass, func, txt, type };
  return msg;
};

QString Newswire::messageTypeName(const MsgType type)
{
  switch(type)
  {
    case eInfoMsg: return tr("Info");    break;
    case eErrInfo: return tr("Info ");   break;    // Um, yes one blank to became same lenght as "Error"
    case eError:   return tr("Error");   break;
    case eWarning: return tr("Warning"); break;
    case eFatal:   return tr("Fatal");   break;
    default:       return "UnknownMessageTypeName"; // No tr(), should you never read
  }
}

void Newswire::addMessages(const MessageLst& msgl)
{
  foreach(Message msg, msgl) addMessage(msg);
}

void Newswire::errInfo(const QString& func, const QString& txt)
{
  addMessage(makeMessage(func, txt, eErrInfo));
}

void Newswire::warning(const QString& func, const QString& txt)
{
  addMessage(makeMessage(func, txt, eWarning));
}

void Newswire::error(const QString& func, const QString& txt)
{
  addMessage(makeMessage(func, txt, eError));
}

void Newswire::fatal(const QString& func, const QString& txt)
{
  addMessage(makeMessage(func, txt, eFatal));
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

void Newswire::removeMessage(const QString& txt)
{
  mHasError = false;
  mHasFatal = false;
  int exist = -1;
  for(int i = 0; i < mMessages.size(); ++i)
  {
    if(mMessages.at(i).text != txt)
    {
      if(eError == mMessages.at(i).type) mHasError = true;
      if(eFatal == mMessages.at(i).type) mHasFatal = true;
      continue;
    }

    exist = i;
    // Don't break, maybe comes some error to be set "mHasError = true"
  }

  if(mHasFatal)  mHasError = true;
  if(exist > -1) mMessages.removeAt(exist);
}

void Newswire::clearMessages()
{
  mMessages.clear();
  mHasError = false;
  mHasFatal = false;
}

void Newswire::verboseP(const QString& func, const QString& txt, const VerboseLevel type/* = eInfo*/)
{
  *mErrConsole << formatMessage(makeMessage(func, txt, eInfoMsg), mFormat.value(eVerbose)) << endl;
}

void Newswire::addMessage(const Message& msg)
{
  for(int i = 0; i < mMessages.size(); ++i)
  {
    if(mMessages.at(i).text == msg.text) return;
  }

  mMessages.append(msg);

  if(eError == msg.type) mHasError = true;
  if(eFatal == msg.type) mHasFatal = true;
  if(mHasFatal)          mHasError = true;

  logMessage(msg);
}

void Newswire::logMessage(const Message& msg)
{
  if(!mNoErrorLogging)
  {
    if(msg.type == eErrInfo and !mHasError) return; // Don't log infos without an error

    *mErrConsole << formatMessage(msg, mFormat.value(eConsLog)) << endl;
  }

  if(!mNoErrorLogging or (msg.type == eFatal))
  {
    if(mLogFile) *mLogFile << formatMessage(msg, mFormat.value(eFileLog)) << endl;
  }
}

void Newswire::print(const QString& txt)
{
  // Don't print two sequenced empty lines
  static bool lastWasEmpty = false;

  if(txt.isEmpty())
  {
    if(lastWasEmpty) return;

    lastWasEmpty = true;;
  }
  else
  {
    lastWasEmpty = false;
  }

  *mErrConsole << txt << endl;
}
