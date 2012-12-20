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

#ifndef CMDCLASS_HPP
#define CMDCLASS_HPP

#include "FClass.h"

class CmdHelper;

/***********************************************************************
*
*   The base class of all CmdFoo classes
*   which are represend a AgentF command
*
************************************************************************/

class CmdClass : public FClass
{
  public:
                          CmdClass(FClass* parent, const QString& func);
    virtual              ~CmdClass();

    static  CmdClass*     createNew(const QString& type, FClass* parent);
    static void           allBriefIn(CmdHelper* ch);

    static void           briefIn(CmdHelper* ch);
    virtual bool          exec(CmdHelper* ch) = 0;

  protected:
    bool                  init(CmdHelper* ch);

    CmdHelper*            mCmd;

  private:

};

#endif
