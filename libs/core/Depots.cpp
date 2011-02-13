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

#include "Depots.h"

#include "Trader.h"

Depots::Depots(FClass* parent)
       : FClass(parent, FUNC)

{
//   reset();
}

Depots::~Depots()
{
//   foreach(Indicator* indi, mIndicators) delete indi;
}

bool Depots::exec(const QStringList& command)
{
  // Look for each command, and execute them if was given.
  // The order of look up is important.
//   if(command.contains("--reset"))     reset();
  if(command.contains("--verbose"))   setVerboseLevel(FUNC, command);
//   if(command.contains("--force"))     mForce = true;
  if(command.contains("--check"))     checkAll();


  return hasError();
}

bool Depots::checkAll()
{
  QSqlQuery* depots = mFilu->execSql("GetAllDepots");
  if(!depots) return false;

  QList<Trader*> traders;
  QList<QStringList> groups;
  QSet<QString> allGroups;
  QDate fromDate = QDate::currentDate();

  while(depots->next())
  {
    Trader* trader = new Trader(this);
    trader->setMsgTargetFormat(eVerbose, "%c: %x");

    QSqlRecord depot = depots->record();
    if(!trader->prepare(depot))
    {
      if(trader->hasError()) addErrors(trader->errors());
      delete trader;
      continue;
    }

    traders.append(trader);
    groups.append(trader->workOnGroups());
    foreach(QString group, trader->workOnGroups()) allGroups.insert(group);

    if(fromDate > trader->needBarsFrom()) fromDate = trader->needBarsFrom();
    //qDebug() << trader->needBarsFrom() << trader->workOnGroups() << allGroups;
  }

  QSet<int> alreadyChecked;
  foreach(QString group, allGroups)
  {
    QSqlQuery* fis = mFilu->getGMembers(mFilu->getGroupId(group));
    if(!fis) continue;

    while(fis->next())
    {
      //qDebug() << fis->value(1) << fis->value(2) << fis->value(3) << fis->value(4);

      int fiId     = fis->value(1).toInt();
      int marketId = fis->value(4).toInt();

      if(alreadyChecked.contains(fiId)) continue;
      alreadyChecked.insert(fiId);

      BarTuple* bars = mFilu->getBars(fiId, marketId, fromDate.toString(Qt::ISODate));
      if(!bars) continue;

      for(int i = 0; i < traders.size(); ++i)
      {
        if(!groups.at(i).contains(group)) continue;

        traders.at(i)->check(bars);
      }

      delete bars;
    }
  }

  // Clean up
  foreach(Trader* trader, traders) delete trader;

  return hasError();
}
