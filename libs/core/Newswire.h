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

#ifndef NEWSWIRE_HPP
#define NEWSWIRE_HPP

#include <QtCore>

class RcFile;

#define FFI_ Q_FUNC_INFO

/***********************************************************************
*
*   The News Agency
*
************************************************************************/

class Newswire
{
  Q_DECLARE_TR_FUNCTIONS(Newswire)

  public:
                   Newswire(const QString& connectionName);
                   Newswire(Newswire* parent);
    virtual       ~Newswire();

    enum VerboseLevel
    {
      eNoVerbose  =  0,
      eEver       =  0,
      eInfo       =  1,
      eMax        =  2
    };

    enum ErrorType
    {
      eNoError    =  0,
      eErrInfo    =  0,
      eWarning    =  1,  // We expect something else, but go on
      eError      =  2,  // Really bad, we can't go on
      eFatal      =  3   // Should *never* happens
    };

    struct Error  // It's a typedef
    {
      QString   func;
      QString   text;
      ErrorType type;
    };

    void            setVerboseLevel(const VerboseLevel level);
    void            setVerboseLevel(const QString& func, const QString& level);
    VerboseLevel    verboseLevel() { return mVerboseLevel; };

    const QList<Error>& errors() const { return mErrors; };
    bool                hasError() const { return mHasError; };

    friend class RcFile;

  protected:
    void            setLogFile(const QString& path);
    void            print(const QString& txt) { *mErrConsole << txt << endl; };
    void            verbose(const QString& func, const QString& txt, const VerboseLevel type = eInfo)
                           { if(mVerboseLevel >= type) verboseP(func, txt, type); };

    void            addErrors(const QList<Error>& error);
    void            errInfo(const QString& func, const QString& txt);
    void            warning(const QString& func, const QString& txt);
    void            error(const QString& func, const QString& txt);
    void            fatal(const QString& func, const QString& txt);
    void            setError(const QString& func, const QString& txt, const ErrorType type);
    void            removeError(const QString& txt);
    bool            isRoot() { return mRoot; };
    void            clearErrors();

  private:
                        // P for private
    void            verboseP(const QString& func, const QString& txt, const VerboseLevel type = eInfo);

    void            addError(const QString& func, const QString& txt, const ErrorType type);
    void            logError(const QString& func, const QString& txt, const QString& type);

    QString         rawFunc(const QString& func)
                    {
                      // func looks like "void Newswire::verbose(...)"
                      mRawFuncRegex.indexIn(func);
                      return mRawFuncRegex.cap() + ":";
                    };

    bool           mRoot;
    VerboseLevel   mVerboseLevel;
    QList<Error>   mErrors;
    bool           mHasError;
    QTextStream*   mErrConsole;
    QFile*         mLogFileFile;
    QTextStream*   mLogFile;
    QRegExp        mRawFuncRegex;
    QString        mConnName;       // ConnectionName/ProgramName for logfile entries
};

#endif
