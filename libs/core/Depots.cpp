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
  if(command.contains("--clo"))       clearOrders(command);
  if(command.contains("--check"))     check(command);
  if(command.contains("--lsd"))       listDepots(command);
  if(command.contains("--lso"))       listOrders(command);

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
  QSqlQuery* depots = getDepots();
  if(!depots) return;

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

void Depots::clearOrders(const QStringList& parm)
{
  QSqlQuery* depots = getDepots();
  if(!depots) return;

  while(depots->next())
  {
    QSqlQuery* orders = mFilu->getOrders(depots->record().value("DepotId").toInt());
    while(orders->next())
    {
      if(orders->value(11).toInt() == FiluU::eOrderActive) continue;

      mFilu->deleteRecord(":user", "order", orders->value(0).toInt());
    }
  }
}

void Depots::listDepots(const QStringList& parm)
{
  QSqlQuery* depots = getDepots();
  if(!depots) return;

  while(depots->next())
  {
    listDepot(depots->record());
  }
}

void Depots::listDepot(const QSqlRecord& depot)
{
  if(depot.isEmpty())
  {
    fatal(FUNC, "No depot given."); // Yes, no tr()
    return;
  }

  print(tr("All positions for depot: %1").arg(depotStatusLine(depot)));

  QSqlQuery* positions = mFilu->execSql("GetDepotPositionsTraderView");

  if(!positions->size())
  {
    print(tr("Depot is empty."));
  }

  while(positions->next())
  {
    printPosition(positions->record());
  }

  print("");
}

void Depots::listOrders(const QStringList& parm)
{
  QSqlQuery* depots = getDepots();
  if(!depots) return;

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

  print(tr("All orders for depot: %1").arg(depotStatusLine(depot)));

  QSqlQuery* orders = mFilu->getOrders(depot.value("DepotId").toInt());

  listOrders(orders, FiluU::eOrderCanceled);
  listOrders(orders, FiluU::eOrderNeedHelp);
  listOrders(orders, FiluU::eOrderAdvice);
  listOrders(orders, FiluU::eOrderActive);
  listOrders(orders, FiluU::eOrderExecuted);
  listOrders(orders, FiluU::eOrderExperied);

  if(!orders->size())
  {
    print(tr("No orders found."));
  }

  print("");
}

void Depots::listOrders(QSqlQuery* orders, int status)
{
  if(!orders) return;

  orders->seek(-1); // Like rewind();
  while(orders->next())
  {
    if(orders->value(11).toInt() != status) continue;

    printOrder(orders->record());
  }
}

void Depots::printPosition(const QSqlRecord& pos)
{
  if(pos.isEmpty()) return;

  QChar fc = QChar(' ');
  //fc = mLineNo % -2 ? QChar(' ') : QChar(183); // Fill character middle dot

  QString symbol = isin(pos.value("FiId").toInt());

  int pieces    = pos.value("Pieces").toInt();
  double ePrice = pos.value("Price").toDouble();

  BarTuple*  bt = mFilu->getBars(pos.value("FiId").toInt(), pos.value("MarketId").toInt(), 1);
  bt->next();
  double price  = bt->close();
  delete bt;

  double change = 100 * (price - ePrice) / ePrice;
  double value  = pieces * price;
  //QDate date    = pos.value("Date").toDate();
  double slice = 100 * (value) / mDP.balance;

  QString text = "%1 %2 %3 %4 %L5 %L6%  %L7 %L8%";
  text.replace(' ', fc); // For a comment see printOrder(..)

  text = text.arg(pos.value("Date").toString())
             .arg(pos.value("FiName").toString(), -30, fc)
             .arg(symbol, 12, fc)
             .arg(pieces, 4, 10, fc)
             .arg(ePrice, 8, 'f', 2, fc)
             .arg(change, 4, 'f', 0, fc)
             .arg(value, 10, 'f', 2, fc)
             .arg(slice, 4, 'f', 0, fc);

  print(text);
}

void Depots::printOrder(const QSqlRecord& order)
{
  if(order.isEmpty()) return;

  ++mLineNo;

  QChar fc = QChar(' ');
  //fc = mLineNo % -2 ? QChar(' ') : QChar(183); // Fill character middle dot

  QString symbol    = isin(order.value("FiId").toInt());
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

  QString text = "%1 %2 %3 %4 %5%6  %7 %8";
  text.replace(' ', fc); // Do it below would looks like
                         // QString text = "%2%1%1%3%1%4x%1%5%1%6%7%1%1%8%9"
                         // and that's pretty confusing

  // Don't print here vDate and market, would be to long, user is clever enough
  text = text.arg(oDate.toString(Qt::ISODate))
             .arg(oType, -4, fc)
             .arg(pieces, 4, 10, fc)
             .arg(symbol, 12, fc)
             .arg(fiName, -30, fc)
             .arg(limitTxt, 12, fc)
             .arg(statusTxt, -8, fc)
             .arg(note);

  print(text);
}

QSqlQuery* Depots::getDepots()
{
  QSqlQuery* depots = mFilu->execSql("GetAllDepots");
  if(!depots)
  {
    check4FiluError(FUNC);
    return 0;
  }

  if(mFilu->lastResult() == Filu::eNoData)
  {
    verbose(FUNC, tr("No depots found."));
    return 0;
  }

  return depots;
}

QString Depots::depotStatusLine(const QSqlRecord& depot)
{
  mDP.id = depot.value("DepotId").toInt();
  mDP.cash = mFilu->getDepotCash(mDP.id);
  mDP.neededCash = mFilu->getDepotNeededCash(mDP.id);
  mDP.value = mFilu->getDepotValue(mDP.id);
  mDP.availCash = mDP.cash - mDP.neededCash;
  mDP.balance = mDP.value + mDP.cash;

  mFilu->setSqlParm(":depotId", mDP.id);
  mFilu->setSqlParm(":fiId",  -1);
  QSqlQuery* positions = mFilu->execSql("GetDepotPositionsTraderView");

  if((mDP.balance < mDP.value) or (mDP.value < 0.0))
  {
    warning(FUNC, tr("It seems it lacks currency data."));
  }

  return QString("%1, Id: %6, Value: %L3 %2, AvCash: %L4 %2, Positions: %5")
                .arg(depot.value("Name").toString(), depot.value("Currency").toString())
                .arg(mDP.balance, 0, 'f', 2)
                .arg(mDP.availCash, 0, 'f', 2)
                .arg(positions->size())
                .arg(mDP.id);
}

QString Depots::isin(int fiId)
{
  // Search ISIN or as fallback Reuters symbol
  SymbolTuple* st = mFilu->getSymbols(fiId);

  while(st->next()) if(st->owner() == "ISIN") break;

  if(st->owner() != "ISIN")
  {
    st->rewind();
    while(st->next()) if(st->owner() == "Reuters") break;
  }

  QString isin = st->caption();
  delete st;

  return isin;
}
