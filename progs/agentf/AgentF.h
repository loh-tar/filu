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

#include <QSet>
#include <QTextStream>
class QProcess;

#include "FCoreApp.h"
class CmdClass;
class Exporter;
class Importer;
class Scanner;

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

    friend class  CmdClass;

  protected slots:
    void run();
    void quit();
    void cloneIsReady();
    void cloneHasFinished();

  protected:
    void startClones();
    void readSettings();

    void cmdExec(const QString& cmd);
    void updateAllBars();
    void exxport();
    void scan();
    void depots();

    void printSettings();
    void printError(const QString message);
    void check4MasterCMD();

    QHash<QString, CmdClass*>   mCmds;        // Hold all used commands FIXME Rename later to mCommands
    QSet<QString>               mKnownCmds;   // Hold all known commands by CmdClass

    Exporter*          mExporter;
    Scanner*           mScanner;
    QList<QProcess*>   mClones;
    QStringList        mCloneNames;

    bool               mQuit;

    QList<QStringList> mCommands;  // Holds all prepared commands for assigning to clones

    QTextStream        mConsole;/*(stdout);*/
    QTextStream        mErrConsole;
};

#endif
