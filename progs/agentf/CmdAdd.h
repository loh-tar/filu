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

#ifndef CMDADD_HPP
#define CMDADD_HPP

#include "CmdClass.h"

class Importer;
class Validator;

/***********************************************************************
*
*   Yes, represent the 'add' command of AgentF.
*
*   He is like an convenience interface for Importer
*   to add one dataset to the database.
*
************************************************************************/

class CmdAdd : public CmdClass
{
  public:
                      CmdAdd(AgentF* agent);
    virtual          ~CmdAdd();

    static bool       isCmd(const QString& cmd);
    static QString    regCmd(CmdHelper* ch);
    static void       briefIn(CmdHelper* ch);
    bool              exec(CmdHelper* ch);

  protected:
    void          import();

    void          addBroker();
    void          addEodBar();
    void          addFi();
    void          addMarket();
    void          addSplit();
    void          addSymbol();
    void          addSymbolType();
    void          addOrder();
    void          addDepot();
    void          addDepotPos();
    void          addAccPosting();
    void          addUnderlyg();

    void          takeDepotOptions();

    Validator*        mValid;
    Importer*         mImporter;
    QStringList       mHeader;
    QStringList       mData;

  private:

};

#endif
