//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011  loh.tar@googlemail.com
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

#ifndef SCANNER_HPP
#define SCANNER_HPP

#include "FClass.h"

class Indicator;

/***********************************************************************
*
*   Yes, he scan the DB for interesting stuff.
*   He is almost used by "agentf scan ..." command. Options are
*
************************************************************************/

class Scanner : public FClass
{
  public:
                Scanner(FClass* parent);
    virtual    ~Scanner();

    bool        exec(const QStringList& command);
    void        reset();
    void        autoSetup();
    void        scanThis(const QStringList& parm);
    void        scanThis(int fiId, int marketId);
    void        mark();

  protected:
    void        loadIndicator(const QStringList& parm);
    void        setTimeFrame(const QStringList& parm);
    void        scanGroup(const QStringList& parm);
    void        scanAll();
    void        scan(BarTuple* bars);

    QList<Indicator*> mIndicators;
    QStringList       mLoadedIndicators;
    QList<int>        mGroupIDs;
    QList<int>        mTimeFrames;
    QDate             mToday;
    QString           mGroupPath;
    int               mBarsToLoad;
    bool              mForce;
    int               mForcedFrame;

  private:

};

#endif
