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

#ifndef CMDDB_HPP
#define CMDDB_HPP

#include "CmdClass.h"

/***********************************************************************
*
*   Yes, represent the 'db' command of AgentF.
*
************************************************************************/

class CmdDB : public CmdClass
{
  public:
                      CmdDB(AgentF* agent);
    virtual          ~CmdDB();

    static bool       isCmd(const QString& cmd);
    static QString    regCmd(CmdHelper* ch);
    static void       briefIn(CmdHelper* ch);
    bool              exec(CmdHelper* ch);

  protected:
    void          remake();
    void          list();
    void          patch();
    void          tinker();
    void          show();

    void          showPrintTable(QSqlQuery* query);
    void          showPrintTicket(QSqlQuery* query);

    QStringList   mTypes;
private:

};

#endif
