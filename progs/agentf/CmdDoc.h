//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
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

#ifndef CMDDOC_HPP
#define CMDDOC_HPP

#include "CmdClass.h"

/***********************************************************************
*
*   Yes, represent the 'doc' command of AgentF.
*
************************************************************************/

class CmdDoc : public CmdClass
{
  public:
                      CmdDoc(AgentF* agent);
    virtual          ~CmdDoc();

    static bool       isCmd(const QString& cmd);
    static QString    regCmd(CmdHelper* ch);
    static void       briefIn(CmdHelper* ch);
    bool              exec(CmdHelper* ch);

  protected:
    void              list();
    void              grep();

    bool              loadFileNames(const QString& pattern = "");
    void              listFiles();
    bool              lessFile(int fileNumber = 0);

    QHash<int, QString>   mAllFiles;
    QStringList           mTxtFiles;
    QStringList           mConfFiles;
    QStringList           mImpFiles;
    QStringList           mFooFiles;
    QList<int>            mFooFileNumbers;

private:

};

#endif
