//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011  loh.tar@googlemail.com
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
*   File created at 2010-03-15
*
*   Yes, he exports stuff out of the DB into a plain text file.
*   He is almost used by "agentf exp ..." command.
*
************************************************************************/

class Exporter : public FClass
{
  public:
                  Exporter(FClass* parent);
    virtual      ~Exporter();

    void          reset();
    bool          exxport(QStringList& command);

  protected:
    enum Effect
    {
      eEffectPending,
      eEffectOk,
      eEffectNote,
      eEffectFault
    };

    void          printStatus(Effect effect = eEffectPending, const QString& extraTxt = "");
    bool          noData(const QString& what = "");
    void          writeToFile();
    bool          selectFis();

    bool          expFiTypes();
    bool          expSymbolTypes();
    bool          expMarkets();
    bool          expFiNames();
    bool          expSymbols();
    bool          expEODRaw();
    bool          expSplits();
    //TODO: bool expUnderlyings();

    // User data exports
    bool          expCOs();
    bool          expGroups();
    bool          expGroup(int gid, bool ignoreIfEmpty = true);

    QStringList   mCmdLine;         // The command line with all options
    QStringList   mParm;            // A helper to get parameter by FTool::getParameter()
    QTextStream   mBuffer;
    QString       mBufferStr;
    QTextStream   mOutput;
    QTextStream   mConsole;
    QFile*        mOutFile;
    QSqlQuery*    mFis;

    // printStatus() stuff
    int           mLineNo;           // Hold the line number which was actual read
    QQueue<int>   mDataLineNo;       // Is cut to max 2 numbers
    int           mByteCount;
    int           mDataR;            // Data Read
    int           mDataW;            // Data Written
    QString       mDataText;
    QStringList   mHint;
    QTime         mRolex;
  private:

};

#endif
