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
                   Newswire();
    virtual       ~Newswire();

    enum NewsType
    {
      // Verbose Levels
      eNoVerbose  =  0,
      eInfo       =  1,
      eMax        =  2,

      // Error Types
      eNoError    =  0,
      eWarning    =  1,  // We expect something else, but go on
      eError      =  2,  // Really bad, we can't go on
      eCritical   =  3   // Should *never* happens
    };

    void            setVerboseLevel(const NewsType level);
    void            setVerboseLevel(const QString& func, const QString& level);
    NewsType        verboseLevel() { return mVerboseLevel; };

  protected:
    void            verbose(const QString& func, const QString& txt, const NewsType type = eInfo);
    void            setError(const QString& func, const QString& txt, const NewsType type = eError);
    void            removeError(const QString& txt);

    QString         buildFullText(const QString& func, const QString& txt)
                    {
                      // func looks like "void Newswire::verbose(...)"
                      QRegExp regex("\\w+::\\w+");
                      regex.indexIn(func);          // Extract to "Newswire::verbose"
                      return regex.cap() + ": " + txt;
                    };

//     void           addErrorText(const QStringList& errorMessage, MsgType type = eNotice);
//     bool           check4FiluError(const QString& errMessage);  // True if error
//     void           clearErrors();

  private:
    NewsType       mVerboseLevel;
    QStringList    mErrorMessage;
    bool           mHasError;
};

#endif
