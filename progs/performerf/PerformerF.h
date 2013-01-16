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

#ifndef PERFORMERF_HPP
#define PERFORMERF_HPP

#include "FWidget.h"

class IndiWidgetGroup;
class FiGroupWidget;
class LaunchPad;
class IndiWidgetSimple;

/***********************************************************************
*
*
*
************************************************************************/

class PerformerF : public FMainApp
{
  Q_OBJECT

  public:
                    PerformerF(QApplication& app);
    virtual        ~PerformerF();

  public slots:
    void            showWindowTitle(const QString& symbol, const QString& market);
    void            loadData(int fiId, int marketId);

  protected:
    IndiWidgetGroup*  mIndiGroup;
    FiGroupWidget*    mGroupNavi;
    FiGroupWidget*    mGroupNavi2;
    IndiWidgetSimple* mZoomWidget;
    LaunchPad*        mLaunchPad;
    QString           mWindowTitel;
};

#endif
