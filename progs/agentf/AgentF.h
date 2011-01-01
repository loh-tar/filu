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

#ifndef AGENTF_HPP
#define AGENTF_HPP

#include <QtCore>

#include "FObject.h"

class Script;
class Importer;
class Exporter;


/***********************************************************************
*
*
*
************************************************************************/

using namespace std;

class AgentF : public FObject
{
  Q_OBJECT

  public:
    AgentF(QCoreApplication* app);
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
    bool dateIsNotValid(QString& date);

    QStringList* fetchBarsFromProvider(const QString& provider,
                                       const QStringList& parameters);

    void addEODBarData(const QStringList& parm);
    void addEODBarDataFull(const QStringList& parm);
    void updateAllBars(const QStringList& parm);
    void addFi(const QStringList& parm);
    bool lineToCommand(const QString& line, QStringList& cmd);
    void readCommandFile(const QStringList& parm);
    void beEvil(const QStringList& parm);
    void import(const QStringList& parm);
    void exxport(const QStringList& parm);
    void addSplit(const QStringList& parm);
    void printUsage();
    void printSettings();
    void check4FiluError(const QString message);
    void printError(const QString message);

    Script*            mScript;
    Importer*          mImporter;
    Exporter*          mExporter;
    QList<QProcess*>   mClones;

    bool               mQuit;
    bool               mIamEvil;   // True if in deamon mode, avoid recursive calls

    QList<QStringList> mCommands;  // Holds all prepared commands for assigning to clones
    //QStringList        mCMD;       // Holds the command line on start up

    QTextStream        mConsole;/*(stdout);*/
    QTextStream        mErrConsole;

    // Setting variables
    int                mMaxClones;
    //int                mDebugLevel;
    QString            mLogFile;
};

#endif
