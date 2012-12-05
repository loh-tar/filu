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
class Script;
class Importer;
class Exporter;
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

    void  execCmd(const QStringList& parm);

  protected slots:
    void run();
    void quit();
    void cloneIsReady();
    void cloneHasFinished();

  protected:
    void startClones();
    void readSettings();

    QStringList* fetchBarsFromProvider(const QString& provider,
                                       const QStringList& parameters);

    void addEODBarData();
    void updateAllBars();
    bool lineToCommand(const QString& line, QStringList& cmd);
    void readCommandFile();
    void beEvil();
    void import();
    void exxport();
    void scan();
    void depots();
    void filu();
    void cmdAdd();
    void deleteBars();
    void splitBars();
    void summon();
    void exorcise();
    void cmdSet();
    void printSettings();
    void printError(const QString message);
    void check4MasterCMD();

    Script*            mScript;
    Importer*          mImporter;
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
