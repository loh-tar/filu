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

#include <float.h>

#include "Depots.h"

#include "Trader.h"
#include "FTool.h"

Depots::Depots(FClass* parent)
       : FClass(parent, FUNC)

{
  setMsgTargetFormat(eVerbose, "%c: %x");
}

Depots::~Depots()
{}

bool Depots::exec(const QStringList& command)
{
  // Look for each command, and execute them if was given.
  // The order of look up is important.
  if(command.contains("--verbose"))   setVerboseLevel(FUNC, command);
  if(command.contains("--check"))     check(command);
  if(command.contains("--lso"))  listOrders(command);

  return hasError();
}

void Depots::check(const QStringList& parm)
{
  QStringList opt;
  FTool::getParameter(parm, "--check", opt);

  if(!opt.size())
  {
    checkAll(parm);
    return;
  }

//   bool autoSetup = false;
//   if(parm.contains("--auto")) autoSetup = true;
}

void Depots::checkAll(const QStringList& parm)
{
  QSqlQuery* depots = mFilu->execSql("GetAllDepots");
  if(!depots)
  {
    verbose(FUNC, tr("No depots found."));
    return;
  }

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

  if(hasError()) return;

  mRcFile->set("LastDepotCheck", QDate::currentDate().toString(Qt::ISODate));

  depots->seek(-1); // Like rewind();
  while(depots->next()) listOrders(depots->record());

  return;
}


void Depots::listOrders(const QStringList& parm)
{
  QSqlQuery* depots = mFilu->execSql("GetAllDepots");
  if(!depots)
  {
    verbose(FUNC, tr("No depots found."));
    return;
  }

  while(depots->next())
  {
    listOrders(depots->record());
  }
}

void Depots::listOrders(const QSqlRecord& depot)
{
  if(depot.isEmpty())
  {
    fatal(FUNC, "No depot given."); // Yes, no tr()
    return;
  }

  mLineNo = 0;

  int    depotId  = depot.value("DepotId").toInt();
  double depCash  = mFilu->getDepotCash(depotId);
  double depNeedC = mFilu->getDepotNeededCash(depotId);
  double depValue = mFilu->getDepotValue(depotId);
  double cash     = depCash - depNeedC;
  double balance  = depValue + depCash;

  mFilu->setSqlParm(":depotId", depotId);
  mFilu->setSqlParm(":fiId",  -1);
  QSqlQuery* positions = mFilu->execSql("GetDepotPositionsTraderView");

  QSqlQuery* orders = mFilu->getOrders(depotId);

  verbose(FUNC, tr("All orders for depot: %1, Value: %L3 %2, AvCash: %L4 %2, Positions: %5")
                  .arg(depot.value("Name").toString(), depot.value("Currency").toString())
                  .arg(balance, 0, 'f', 2)
                  .arg(cash, 0, 'f', 2)
                  .arg(positions->size()) );

  listOrders(orders, FiluU::eOrderCanceled);
  listOrders(orders, FiluU::eOrderNeedHelp);
  listOrders(orders, FiluU::eOrderAdvice);
  listOrders(orders, FiluU::eOrderActive);
  listOrders(orders, FiluU::eOrderExecuted);
  listOrders(orders, FiluU::eOrderExperied);

  verbose(FUNC, "");
}

void Depots::listOrders(QSqlQuery* orders, int status)
{
  if(!orders) return;

  orders->seek(-1); // Like rewind();
  while(orders->next())
  {
    if(orders->value(10).toInt() != status) continue;

    printOrder(orders->record());
  }
}

void Depots::printOrder(const QSqlRecord& order)
{
  if(order.isEmpty()) return;

  ++mLineNo;

  QChar fc = QChar(' ');
  //fc = mLineNo % -2 ? QChar(' ') : QChar(183); // Fill character middle dot

  //
  // Search ISIN or as fallback Reuters symbol
  SymbolTuple* st = mFilu->getSymbols(order.value("FiId").toInt());
  while(st->next()) if(st->owner() == "ISIN") break;

  if(st->owner() != "ISIN")
  {
    st->rewind();
    while(st->next()) if(st->owner() == "Reuters") break;
  }

  QString symbol = st->caption();

  QString statusTxt = mFilu->orderStatus(order.value("Status").toInt());

  int pieces    = order.value("Pieces").toInt();
  bool buyOrder = order.value("Buy").toBool();
  double limit  = order.value("Limit").toDouble();
  QDate oDate   = order.value("ODate").toDate();
  QDate vDate   = order.value("VDate").toDate();

  QString oType = mFilu->orderType(buyOrder);
  QString limitTxt = limit == 0.0 ? tr("Best") : QString::number(limit, 'f', 2);
  if(limit) limitTxt.append(" " + order.value("Currency").toString());
  else limitTxt.append(QString(fc).repeated(4));

  QString note = QString("%1%1").arg(fc);

  if(order.value("Status").toInt() == FiluU::eOrderAdvice)
  {
    note.append(order.value("Note").toString());
  }
  else if(order.value("Status").toInt() == FiluU::eOrderActive)
  {
    BarTuple* bars = mFilu->getBars(order.value("FiId").toInt()
                                  , order.value("MarketId").toInt()
                                  , oDate.toString(Qt::ISODate));

    if(bars) // Be on the safe side
    {
      QDate maxDate, minDate;
      double min = DBL_MAX;
      double max = 0.0;
      while(bars->next())
      {
        if(bars->high() > max)
        {
          max = bars->high();
          maxDate = bars->date();
        }

        if(bars->low() < min)
        {
          min = bars->low();
          minDate = bars->date();
        }
      }

      delete bars;

      if(buyOrder)
      {
        int days = minDate.daysTo(QDate::currentDate());
        note.append(tr("%L1, %2 d.ago")
                      .arg(min, 7, 'f', 2, fc).arg(days, 2));
      }
      else
      {
        int days = minDate.daysTo(QDate::currentDate());
        note.append(tr("%L1, %2 d.ago")
                      .arg(max, 7, 'f', 2, fc).arg(days, 2));
      }

      note.append(", ");
    }

    if(note.length() == 2) note.append(QString(fc).repeated(19));
    note.append(tr("%1 d.left").arg(QDate::currentDate().daysTo(vDate), 2));

  }

  if(note.length() == 2) note.clear();

  QString fiName = order.value("FiName").toString();
  if(fiName.length() > 30)
  {
    fiName.truncate(28);
    fiName.append("~+");
  }

  QString verbText = QString("%2%1%1%3%1%4x%1%5%1%6%7%1%1%8%9")
                    .arg(fc)
                    .arg(oDate.toString(Qt::ISODate))
                    .arg(oType, -4, fc)
                    .arg(pieces, 4, 10, fc)
                    .arg(symbol, 12, fc)
                    .arg(fiName, -30, fc)
                    .arg(limitTxt, 12, fc)
                    .arg(statusTxt, -8, fc)
                    .arg(note);

  verbose(FUNC, verbText);
}
