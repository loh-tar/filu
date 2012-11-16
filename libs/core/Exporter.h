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

#ifndef EXPORTER_HPP
#define EXPORTER_HPP

#include "FClass.h"

class Indicator;
class CmdHelper;

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

    static void   briefIn(CmdHelper* cmd);
    bool          exec(CmdHelper* cmd);

    void          reset();
    bool          exxport();

  protected:
    enum Effect
    {
      eEffectPending,
      eEffectOk,
      eEffectNote,
      eEffectFault
    };

    void          cmdAll();
    void          cmdCore();
    void          cmdUser();
    void          cmdData();

    void          printStatus(Effect effect = eEffectPending, const QString& extraTxt = "");
    bool          noData(const QString& what = "", const VerboseLevel when = eInfo);
    bool          badQuery(QSqlQuery* query);
    void          writeToFile();
    bool          selectFis();

    bool          expFiTypes();
    bool          expSymbolTypes();
    bool          expMarkets();
    bool          expFiNames();
    bool          expUnderlyings();
    bool          expSymbols();
    bool          expEODRaw();
    bool          expSplits();
    bool          expBroker();

    // User data exports
    bool          expCOs();
    bool          expGroups();
    bool          expGroup(int gid, bool ignoreIfEmpty = true);
    bool          expDepots();
    void          expPositions(QSqlQuery* pos);
    void          expAccount(QSqlQuery* acc);
    void          expOrders(QSqlQuery* orders);

    CmdHelper*    mCmd;
    QStringList   mExport;          // Which data has to be exported
    QStringList   mOLine;           // Out Line, will join('\n')
    QStringList   mDLine;           // Data Line, will join(';')
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
