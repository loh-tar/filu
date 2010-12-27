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

#ifndef FOBJECT_HPP
#define FOBJECT_HPP

#include "FClass.h"

/***********************************************************************
*
*   The base of all QObjects which use Filu
*
************************************************************************/

class FObject : public QObject, public FClass
{
  public:
              FObject(FClass* parent);
              FObject(FObject* parent);
              FObject(const QString& connectionName, QObject* parent = 0);
    virtual  ~FObject();

  protected:
};

#endif
