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

#ifndef LOGBOOKPAGE_H
#define LOGBOOKPAGE_H

#include "ManagerPage.h"

class LogBookPage : public ManagerPage
{
  //Q_OBJECT

  public:
                  LogBookPage(ManagerF* parent);
                  LogBookPage(FWidget* parent);
    virtual      ~LogBookPage();

  //public slots:
    void          addToLog(const QString& msg, const bool error);

  protected:
    void          createPage();

    QTextEdit     mLogBook;
};

#endif
