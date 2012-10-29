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

#ifndef FWIDGET_HPP
#define FWIDGET_HPP

#include <QtGui>

#include "FClass.h"

/***********************************************************************
*
*   The base of all QWidgets which use Filu
*
************************************************************************/

class FWidget : public QWidget, public FClass
{
  public:
              // Here no 'const FFoo' because QWidget don't take const
              FWidget(FClass* parent, const QString& className);
              FWidget(FWidget* parent, const QString& className);
    virtual  ~FWidget();
};

class FMainApp : public QMainWindow, public FClass
{
  public:
              FMainApp(const QString& connectionName, QApplication& app);
    virtual  ~FMainApp();

  protected:
    QStringList mCommandLine;
    QStringList mConfigParms;
};

#endif
