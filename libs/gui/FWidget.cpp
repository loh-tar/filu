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

#include <QApplication>

#include "FWidget.h"

#include "FiluU.h"
#include "RcFile.h"

FWidget::FWidget(FClass* parent, const QString& className)
       : QWidget(0)
       , FClass(parent, className)
{}

FWidget::FWidget(FWidget* parent, const QString& className)
       : QWidget(parent)
       , FClass(parent, className)
{}

FWidget::~FWidget()
{}

FMainApp::FMainApp(const QString& connectionName, QApplication& app)
        : QMainWindow()
        , FClass(connectionName)
{
  mCommandLine = app.arguments();
  mConfigParms = mRcFile->takeConfigParms(mCommandLine);
  mFilu->openDB();
  addMessages(mFilu->messages());
}

FMainApp::~FMainApp()
{}
