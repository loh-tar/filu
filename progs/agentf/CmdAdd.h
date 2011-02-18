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

#ifndef CMDADD_HPP
#define CMDADD_HPP

#include "FClass.h"

class Importer;

/***********************************************************************
*
*   Yes, represent the 'add' command of AgentF.
*
*   He is like an convenience interface for Importer
*   to add one dataset to the database.
*
************************************************************************/

class CmdAdd : public FClass
{
  public:
                CmdAdd(FClass* parent);
    virtual    ~CmdAdd();

    bool        exec(const QStringList& cmdLine);

  protected:
    void        printDataTypes();
    void        import(const QString& header, const QString& data);

//     void        addBroker(const QStringList& parm);
    void        addEodBar(const QStringList& parm);
    void        addFi(const QStringList& parm);
    void        addMarket(const QStringList& parm);
    void        addSplit(const QStringList& parm);
    void        addSymbol(const QStringList& parm);
    void        addSymbolType(const QStringList& parm);
//     void        add(const QStringList& parm);
//     void        add(const QStringList& parm);
//     void        add(const QStringList& parm);
    void        addUnderlyg(const QStringList& parm);

    Importer*       mImporter;
    bool            mWantHelp;  // Flag to print help info
    QStringList     mCmdArg;    // Like a helper, used to fetch arguments

    QHash<QString, QString>   mInfoTxt; // Hold some often used error messages

  private:

};

#endif
