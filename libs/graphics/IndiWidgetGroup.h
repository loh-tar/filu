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

#ifndef INDIWIDGETGROUP_HPP
#define INDIWIDGETGROUP_HPP

#include "FWidget.h"

class MyMouseEvent;

/***********************************************************************
*
*
*
************************************************************************/


class IndiWidgetGroup : public FWidget
{
  Q_OBJECT

  public:
                  IndiWidgetGroup(FWidget* parent);
                  IndiWidgetGroup(const QString& name, FWidget* parent);
    virtual      ~IndiWidgetGroup();

    const QString&  indiSetName() { return mSetName; } ;

  public slots:
    void          addWindow();
    void          removeWindow();
    void          loadSetup(const QString& setup);
    void          showBarData(BarTuple* bars);
    void          childSplitterMoved(QList<int> *size);
    void          mouseSlot(MyMouseEvent *);
    void          chartObjectChosen(QAction* action);

  protected:
    void          init();
    void          readSettings();
    void          saveSetup();

    QString       mSetName;
    QString       mFullIndiSetsPath;
    QSplitter*    mSplitter;
    BarTuple*     mBars;
};

#endif
