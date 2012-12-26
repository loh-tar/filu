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

#ifndef AGENTF_HPP
#define AGENTF_HPP

#include <QTextStream>
class QProcess;

#include "FCoreApp.h"
class CmdClass;
class Exporter;
class Importer;
class Scanner;
class Script;

/***********************************************************************
*
*
*
************************************************************************/

class AgentF : public FCoreApp
{
  Q_OBJECT

  public:
    AgentF(QCoreApplication& app);
   ~AgentF();

    void  exec(const QStringList& parm);

  protected slots:
    void run();
    void quit();
    void cloneIsReady();
    void cloneHasFinished();

  protected:
    void startClones();
    void readSettings();

    void cmdExec(const QString& cmd);
    void cmdFetch();
    void addEODBarData();
    void updateAllBars();
    bool lineToCommand(const QString& line, QStringList& cmd);
    void cmdRcf();
    void beEvil();
    void exxport();
    void scan();
    void depots();
    void deleteBars();
    void splitBars();
    void cmdSet();

    void printSettings();
    void printError(const QString message);
    void check4MasterCMD();
    void addEODBarData(const QString& symbol, const QString& market, const QString& provider
                     , const QDate& fromDate, const QDate& toDate, int fiId, int marketId);

    QHash<QString, CmdClass*>   mCmds; // Hold all used commands FIXME Rename later to mCommands

    Script*            mScript;
    Exporter*          mExporter;
    Scanner*           mScanner;
    QList<QProcess*>   mClones;
    QStringList        mCloneNames;

    bool               mQuit;
    bool               mIamEvil;   // True if in deamon mode, avoid recursive calls

    QList<QStringList> mCommands;  // Holds all prepared commands for assigning to clones

    QTextStream        mConsole;/*(stdout);*/
    QTextStream        mErrConsole;
};

#endif
