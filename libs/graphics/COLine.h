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

#ifndef COLINE_HPP
#define COLINE_HPP

#include "COType.h"

/***********************************************************************
*
* Yes, a line chart object
*
************************************************************************/

class COLine : public COType
{
  public:
                COLine(IndicatorPainter* painter);
    virtual    ~COLine();

    bool        isInvolved(const QPoint& pos);

  protected slots:
    void        editWindowChanged();

  protected:
    void        prepare(const QHash<QString, QString> &keyValue);
    void        readAttributes(bool firstCall = false);
    void        writeAttributes();
    bool        paintObject();
    bool        clipLine();
    Grip*       completeBrandNew(const QPoint& pos);
    void        gripMoved( const QPoint& deltaPos
                         , const int deltaIdx
                         , const double deltaValue);

    QLineF      mLine;  // hold the line as they was painted
    QLineF      mHelp;  // a tiny perpendicular line to our painted line, used at isInvolved()
};
#endif
