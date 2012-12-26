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

#ifndef CMDEXORCISE_HPP
#define CMDEXORCISE_HPP

#include "CmdClass.h"

/***********************************************************************
*
*   Yes, represent the 'exo' command of AgentF.
*
************************************************************************/

class CmdExorcise : public CmdClass
{
  public:
                  CmdExorcise(FClass* parent);
    virtual      ~CmdExorcise();

    static void   regCmd(CmdHelper* ch);
    static void   briefIn(CmdHelper* ch);
    bool          exec(CmdHelper* ch);

  protected:

  private:

};

#endif