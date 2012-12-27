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

#ifndef CMDDO_HPP
#define CMDDO_HPP

#include "CmdClass.h"

/***********************************************************************
*
*   Yes, represent the 'do' command of AgentF.
*
************************************************************************/

class CmdDo : public CmdClass
{
  public:
                      CmdDo(AgentF* agent);
    virtual          ~CmdDo();

    static bool       isCmd(const QString& cmd);
    static QString    regCmd(CmdHelper* ch);
    static void       briefIn(CmdHelper* ch);
    bool              exec(CmdHelper* ch);

  protected:
    bool              fromStdIn();
    bool              fromFile();
    bool              fromDB();

    bool              lineToCommand(const QString& line, QStringList& cmd);

    bool              mIamEvil;   // True if in deamon mode, avoid recursive calls

  private:

};

#endif
