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

#include <float.h>

#include <QSqlQuery>
#include <QSqlRecord>
#include <QTextStream>

#include "Depots.h"

#include "BarTuple.h"
#include "CmdHelper.h"
#include "FiluU.h"
#include "RcFile.h"
#include "SymbolTuple.h"
#include "Trader.h"

const QString cCmd1 = "depots";
const QString cYes  = QObject::tr("Yes");
const QString cCmd1Brief = QObject::tr("Performs actions with your portfolios");
const QString cUseLso4Id = QObject::tr("Use '%1' to obtain the OrderId.");


Depots::Depots(FClass* parent)
       : FClass(parent, FUNC)
       , mCmd(0)

{
  setMsgTargetFormat(eVerbose, "%c: %x");
}

Depots::~Depots()
{}

void Depots::briefIn(CmdHelper* cmd)
{
   if(!cmd) return;

   cmd->inCmdBrief(cCmd1, cCmd1Brief);
}

bool Depots::exec(CmdHelper* ch)
{
  if(!ch)
  {
    fatal(FUNC, "Called with NULL pointer.");
    return false;
  }

  mCmd = ch;
  mCmd->regSubCmds("delete simtrade cancel cho clo check lsd lso");
  mCmd->regOpts("4day to from owner dpid");

  if(mCmd->subCmdLooksBad()) return false;

  if(mCmd->wantHelp())
  {
    mCmd->inSubBrief("delete", tr("Delete one or more depots"));
    mCmd->inSubBrief("simtrade", tr("Run a full complex trading simulation"));
    mCmd->inSubBrief("cancel", tr("To cancel an active order"));
    mCmd->inSubBrief("cho", tr("Change an order"));
    mCmd->inSubBrief("clo", tr("Clears all not active orders"));
    mCmd->inSubBrief("check", tr("Check the depots if anything happens"));
    mCmd->inSubBrief("lsd", tr("List depots"));
    mCmd->inSubBrief("lso", tr("List orders"));

    mCmd->inOptBrief("4day", "<Date>", tr("To specify which day you like to see. The same as '--to'"));
    mCmd->inOptBrief("to", "<Date>", tr("To limit the most newest date"));
    mCmd->inOptBrief("from", "<Date>", tr("To expand the investigated date range"));
    mCmd->inOptBrief("owner", "<Name>", tr("Work on all depots of named owner"));
    mCmd->inOptBrief("dpid", "<Id>", tr("Work only on exact this depot"));

    mCmd->inOptGroup("DepotFilter", tr("depot filter"), "owner dpid");
    mCmd->inOptGroup("TimeFilter", tr("time filter"), "4day to from");
  }

  if(mCmd->needHelp(2))
  {
    if(mCmd->printThisWay("<Command> [<ParmList>] [--<Command> [<ParmList>]]..")) return !hasError();

    mCmd->printComment(tr("The order of given commands has no influence on the execution order."));
    mCmd->printNote(tr("All relating to depot stuff is fresh under development, maybe buggy and "
                       "not very comfortable to use. More to read at doc/depot-management.txt."));
    mCmd->printForInst("check --owner Me --lso --clo");
    mCmd->aided();
    return !hasError();
  }

  // Look for each command, and execute them if was given.
  // The order of look up is important.
  if(mCmd->hasOpt("verbose"))   setVerboseLevel(FUNC, mCmd->cmdLine());

  mToday     = mCmd->optDate("4day", QDate::currentDate());
  mToday     = mCmd->optDate("to", mToday);
  mLastCheck = mCmd->optDate("from", mRcFile->getDT("LastDepotCheck"));

  if(mCmd->hasError() and !mCmd->wantHelp()) return false; // FIXME

  if(mCmd->hasSubCmd("delete"))    deleteDepots();
  if(mCmd->hasSubCmd("simtrade"))  simtrade();

  if(hasError()) return false;

  if(mCmd->hasSubCmd("cancel"))    cancelOrder();
  if(mCmd->hasSubCmd("cho"))       changeOrder();
  if(mCmd->hasSubCmd("clo"))       clearOrders();
  if(mCmd->hasSubCmd("check"))     check();
  if(mCmd->hasSubCmd("lsd"))       listDepots();
  if(mCmd->hasSubCmd("lso"))       listOrders();

  return !hasError();
}

QDate Depots::nextCheckday(const QList<int>& checking)
{
  mToday = mToday.addDays(checking.at(0));

  if(checking.size() > 1)
  {
     while(mToday.dayOfWeek() != checking.at(1))
    {
      //qDebug() << "Shift to given week day" << mToday;
      mToday = mToday.addDays(1);
    }
  }

  while(mToday.dayOfWeek() > 5)
  {
    //qDebug() << "Shift weekend" << mToday;
    mToday = mToday.addDays(1);
  }

  return mToday;
}

void Depots::simtrade()
{
  mCmd->regOpts("weekly checkDay");

  if(mCmd->isMissingParms(4))
  {
    mCmd->inOptBrief("weekly", "[<n:1>]", tr("Check only each n weeks. When not given each day is checked"));
    mCmd->inOptBrief("checkDay", "<DayName:Fri>", tr("At the end of which day take a look on the market"));

    if(mCmd->printThisWay("<Trader> <BrokerName> <FromDate> <ToDate> \\ "
                          "[~~weekly [~~checkDay]]")) return;

    mCmd->printComment(tr("Creates a new depot owned by 'Simulator' with a auto generated name "
                          "and a fix start cash of 10,000.00"));

    mCmd->printForInst("SimpleRule MyBanK 2009-01-01 2011-01-01 --checkDay Wed --weekly 4 --verbose 0");
    mCmd->aided();
    return;
  }

  QString depotName = mCmd->argStr(1);
  QDate fromDate = mCmd->argDate(3);
  QDate toDate = mCmd->argDate(4);

  if(mCmd->hasError()) return;

  QList<int> checking; // Hold at pos 0 day interval, at pos 1 the day of week
                       // FIXME Use and name it something better :-/

  checking << 1; // Check each day

  if(mCmd->hasOpt("weekly"))
  {
    depotName.append("Weekly");
    checking << 5; // Add checkDay=Friday

    checking[0] = 7 * mCmd->optInt("weekly", 1);

    if(mCmd->hasOpt("checkDay"))
    {
      QStringList dayNames;
      dayNames << "Mon" << "Tue" << "Wed" << "Thu" << "Fri";
      int idx = dayNames.indexOf(QRegExp(mCmd->optStr("checkDay", "fault"), Qt::CaseInsensitive, QRegExp::Wildcard));
      if(idx < 0)
      {
        error(FUNC, tr("Given --checkDay is unknown."));
        errInfo(FUNC, tr("Try (not case sensitive): %1").arg("Mon, Tue, Wed, Thu, Fri"));
      }
      else
      {
        depotName.append(dayNames.at(idx));
        checking[1] = idx + 1;
      }
    }
  }

  if(mCmd->hasError()) return;
  if(hasError()) return;

  // FIXME Ask here Trader, check if Trading rule is ok. Ask for hash. Ask for indicator.

  // Test if depotName is unique
  QSqlQuery* depot = getDepots("Simulator");
  QStringList names;
  while(depot->next())
  {
    names << depot->value(1).toString();
  }
  QString tryName = depotName;
  for(int i = 1;;++i)
  {
//     qDebug() << "try name: " << tryName;
    if(!names.contains(tryName)) break;
    tryName = QString("%1%2").arg(depotName).arg(i);
  }

  // Create depot
  int depotId = mFilu->addDepot(tryName, "Simulator", mCmd->argStr(1), mCmd->argStr(2));
  if(check4FiluError(FUNC, tr("Can't create depot."))) return;

  // Add initial cash to depot
  mFilu->addAccPosting(depotId, fromDate.addDays(-1), FiluU::ePostCashIn, "Your fake chance", 10000.00);

  // Travel across the time
  depot = getDepots(depotId);
  mLastCheck = fromDate.addDays(-1);
  mToday = mLastCheck;
  mToday = nextCheckday(checking);
  int totalDays = fromDate.daysTo(toDate);
  while(mToday <= toDate)
  {
    if(!verboseLevel())
    {
      // FIXME Do it much more nicer
      int done = 100 * fromDate.daysTo(mToday) / totalDays;
      print(tr("Processing...%1%").arg(done));
    }

    // Check depot
    checkDepots(depot);

    //
    // Check for order advices and change them to active
    // but take care that we not run out of money.
    QSqlQuery*  advices = mFilu->getOrders(depotId, FiluU::eOrderAdvice);

    double cash;
    if(advices->size()) cash = mFilu->getDepotCash(depotId, mToday); // Only asks if it make sense

    // FIXME That expensive stuff could be avoid if had a always a price
    //       in the order table. (Best-Price is noted with 0.0)
    while(advices->next())
    {
      if(advices->value(12).toBool()) // Check if buy order
      {
        // Ok, make them active...
        mFilu->updateField("status", FiluU::eOrderActive, ":user", "order", advices->value(0).toInt());

        // ..but check if we have now some cash left
        double neededCash = mFilu->getDepotNeededCash(depotId, advices->value(2).toDate());
        if(cash - neededCash < 0)
        {
          // No, we haven't. Kill that order
          mFilu->updateField("status", FiluU::eOrderCanceled, ":user", "order", advices->value(0).toInt());
        }
      }
      else
      {
        // No problem, we can always sell
        mFilu->updateField("status", FiluU::eOrderActive, ":user", "order", advices->value(0).toInt());
      }
    }

    // Next day
    depot->seek(-1);
    mLastCheck = mToday;
    mToday = nextCheckday(checking);
  }
}

void Depots::check()
{
  // Ignore filter settings --from --to
  //mToday = QDate::currentDate();
  //mLastCheck = mRcFile->getDT("LastDepotCheck");

  if(mCmd->isMissingParms())
  {
    if(mCmd->printThisWay("[<DepotFilter>]")) return;

    mCmd->printForInst("--owner Me");
    mCmd->aided();
    return;
  }

  checkDepots(getDepots());

  if(hasError()) return;

  // FIXME What if we have more depots but --dpid was given?
  // add a "LastChecked" field in depot table?
  mRcFile->set("LastDepotCheck", QDate::currentDate().toString(Qt::ISODate));
  //mRcFile->set("LastDepotCheck", mToday.toString(Qt::ISODate));
}

void Depots::checkDepots(QSqlQuery* depots)
{
  if(!depots) return;

  QList<Trader*> traders;
  QList<QStringList> groups;
  QSet<QString> allGroups;

  QDate fromDate = mLastCheck;

  while(depots->next())
  {
    Trader* trader = new Trader(this);
    trader->setMsgTargetFormat(eVerbose, "%c: %x");

    QSqlRecord depot = depots->record();
    if(!trader->prepare(depot, mLastCheck, mToday))
    {
      if(trader->hasError()) addErrors(trader->errors());
      delete trader;
      continue;
    }

    traders.append(trader);
    groups.append(trader->workOnGroups());
    foreach(QString group, trader->workOnGroups()) allGroups.insert(group);

    if(fromDate > trader->needBarsFrom()) fromDate = trader->needBarsFrom();

    if(verboseLevel(eAmple))
    {
      verbose(FUNC, tr("Depot with ID:  %1").arg(depots->value(0).toInt()));
      verbose(FUNC, tr("Work on groups: %1").arg(trader->workOnGroups().join(" ")));
      verbose(FUNC, tr("Need bars from: %1").arg(trader->needBarsFrom().toString(Qt::ISODate)));
    }
  }

  if(verboseLevel(eAmple) and depots->size() > 1)
  {
    verbose(FUNC, tr("Total work on groups: %1").arg(QStringList(allGroups.toList()).join(" ")));
    verbose(FUNC, tr("Total need bars from: %1").arg(fromDate.toString(Qt::ISODate)));
  }

  verbose(FUNC, tr("Check for day:  %1").arg(mToday.toString(Qt::ISODate)), eAmple);

  QSet<int> alreadyChecked;
  foreach(QString group, allGroups)
  {
    int gid = mFilu->getGroupId(group);
    if(gid < 0)
    {
      warning(FUNC, tr("Group '%1' not found.").arg(group));
      continue;
    }

    QSqlQuery* fis = mFilu->getGMembers(gid);
    if(!fis)
    {
      if(check4FiluError(FUNC)) continue;
      warning(FUNC, tr("No FIs in group '%1'").arg(group));
      continue;
    }

    verbose(FUNC, tr("Group %1 contains %2 FIs.").arg(group).arg(fis->size()), eAmple);

    while(fis->next())
    {
      int fiId     = fis->value(1).toInt();
      int marketId = fis->value(4).toInt();

      if(alreadyChecked.contains(fiId)) continue;
      alreadyChecked.insert(fiId);

      verbose(FUNC, tr("Check %1 at %2").arg(fis->value(2).toString(), fis->value(3).toString()), eAmple);

      BarTuple* bars = mFilu->getBars(fiId, marketId, fromDate.toString(Qt::ISODate)
                                                    , mToday.toString(Qt::ISODate));
      if(!bars) continue;

      for(int i = 0; i < traders.size(); ++i)
      {
        if(!groups.at(i).contains(group)) continue;

        traders.at(i)->check(bars, mLastCheck);
      }

      delete bars;
    }
  }

  // Clean up
  foreach(Trader* trader, traders) delete trader;
}

void Depots::changeOrder()
{
  if(mCmd->isMissingParms(1))
  {
    if(mCmd->printThisWay("<OrderId> [<Key=Value>]..")) return;

    mCmd->printComment(tr("The order must not have status 'Executed'. After the change the order is 'Active'."));
    mCmd->printComment(QString(cUseLso4Id).arg("lso"));
    mCmd->printComment(tr("Possible Keys are: %1").arg("date valid limit pieces"));

    mCmd->printForInst(QString("123 pieces=45 limit=67.89 valid=%1")
                          .arg(QDate::currentDate().addDays(7).toString(Qt::ISODate)));
    mCmd->aided();
    return;
  }

  int id = mCmd->argInt(1);

  if(mCmd->hasError()) return;

  QSqlRecord order;
  if(!getOrder(id, order)) return;

  if(order.value("Status").toInt() == FiluU::eOrderExecuted)
  {
    warning(FUNC, tr("Order with Id %1 was executed, can't change.").arg(id));
    return;
  }

  // Take care that we can restore changes
  // in case of a later detected bad parameter
  mFilu->transaction();

  bool ok;
  QString pair;
  for(int i = 2;; ++i)
  {
    if(mCmd->argStr(i, "STOP") == "STOP") break;

    pair.append(mCmd->argStr(i));

    QStringList keyVal = pair.split("=", QString::SkipEmptyParts);
    if(keyVal.size() < 2) continue; // Collect more (accept also spaces)
    pair.clear();

    QString key = keyVal.at(0).toLower();
    if(key == "date" or key == "valid")
    {
      QDate date = QDate::fromString(keyVal.at(1), Qt::ISODate);
      if(date.isValid())
      {
        if(key == "date")
        {
          mFilu->updateField("odate", date.toString(Qt::ISODate), ":user", "order", id);
        }
        else
        {
          mFilu->updateField("vdate", date.toString(Qt::ISODate), ":user", "order", id);
        }
      }
      else
      {
        error(FUNC, tr("Bad date '%1'").arg(keyVal.at(1)));
        break;
      }
    }
    else if(key == "limit")
    {
      double limit = 0.0;
      if(keyVal.at(1).compare("Best", Qt::CaseInsensitive) == 0)
      {
        ok = true;
      }
      else
      {
        limit = keyVal.at(1).toDouble(&ok);
      }

      if(ok)
      {
        mFilu->updateField("olimit", limit, ":user", "order", id);
      }
      else
      {
        error(FUNC, tr("Bad Limit '%1'").arg(keyVal.at(1)));
        break;
      }
    }
    else if(key == "pieces")
    {
      int pieces = keyVal.at(1).toInt(&ok);

      if(ok)
      {
        mFilu->updateField("pieces", pieces, ":user", "order", id);
      }
      else
      {
        error(FUNC, tr("Bad Pieces '%1'").arg(keyVal.at(1)));
        break;
      }
    }
    else
    {
      error(FUNC, tr("Unknown or Unsupported Field '%1'.").arg(keyVal.at(0)));
      break;
    }

    if(check4FiluError(FUNC)) break;
  }

  if(hasError())
  {
    // Restore original data
    mFilu->rollback();
    return;
  }

  mFilu->updateField("status", FiluU::eOrderActive, ":user", "order", id);
  mFilu->commit();  // All looks good

  if(verboseLevel())
  {
    verbose(FUNC, tr("Order changed."));
    getOrder(id, order);
    mOptions.insert("PrintOrder", "Ticket");
    printOrder(order);
  }
}

void Depots::cancelOrder()
{
  if(mCmd->isMissingParms(1))
  {
    if(mCmd->printThisWay("<OrderId>")) return;

    mCmd->printComment(QString(cUseLso4Id).arg("lso"));
    mCmd->printForInst("123");
    mCmd->aided();
    return;
  }

  int id = mCmd->argInt(1);

  if(mCmd->hasError()) return;

  QSqlRecord order;
  if(!getOrder(id, order)) return;

  if(order.value("Status").toInt() < FiluU::eOrderActive)
  {
    warning(FUNC, tr("Order with Id %1 is not active, can't cancel.").arg(id));
    return;
  }

  mFilu->updateField("status", FiluU::eOrderCanceled, ":user", "order", id);

  if(verboseLevel())
  {
    verbose(FUNC, tr("Order canceled."));
    getOrder(id, order);
    mOptions.insert("PrintOrder", "Human");
    printOrder(order);
  }
}

void Depots::clearOrders()
{
  if(mCmd->isMissingParms())
  {
    if(mCmd->printThisWay("[<DepotFilter>]")) return;

    mCmd->aided();
    return;
  }

  QSqlQuery* depots = getDepots();
  if(!depots) return;

  while(depots->next())
  {
    QSqlQuery* orders = mFilu->getOrders(depots->record().value("DepotId").toInt());
    while(orders->next())
    {
      if(orders->value(11).toInt() >= FiluU::eOrderActive) continue;

      mFilu->deleteRecord(":user", "order", orders->value(0).toInt());
    }
  }
}

void Depots::deleteDepots()
{
  mCmd->regOpts("I-am-sure");

  if(mCmd->isMissingParms())
  {
    mCmd->inOptBrief("I-am-sure", "", "Don't ask if I'am sure");

    if(mCmd->printThisWay("<DepotFilter> [~~I-am-sure]")) return;

    mCmd->printComment(tr("Be very carefully with the use of --I-am-sure, "
                          "especially if you don't give --dpid."));
    mCmd->aided();
    return;
  }

  QSqlQuery* depots = getDepots();
  if(!depots) return;

  while(depots->next())
  {
    if(!mCmd->has("I-am-sure"))
    {
      print("");
      print("!!!");
      print(tr("!!! BE WARNED - You will lost the complete depot history !!!"));
      print("!!!");

      printDepotHeader(depots->record());
      print("");

      QTextStream out(stdout);
      out << tr("Are you sure to delete this depot [No]/%1/Cancel? ").arg(cYes) << flush;

      QTextStream in(stdin);
      QString line;
      line = in.readLine(10);

      if(line.compare(cYes, Qt::CaseInsensitive))
      {
        if(line.startsWith("C", Qt::CaseInsensitive))
        {
          print(tr("Operation aborted."));
          break;
        }

        print(tr("Ok, nothing happens."));
        continue;
      }
    }

    mFilu->deleteRecord(":user", "depot", depots->value(0).toInt());

    verbose(FUNC, tr("R.I.P. Depot with Id %1 has been deleted.").arg(depots->value(0).toInt()));
  }
}

void Depots::listDepots()
{
  if(mCmd->isMissingParms())
  {
    if(mCmd->printThisWay("[<DepotFilter>] [~~4day]")) return;

    mCmd->printForInst(QString("--owner Me --4day %1-12-31")
                              .arg(QDate::currentDate().year() - 1));
    mCmd->printComment(tr("The example shows your depot(s) at the end of last year."));
    mCmd->aided();
    return;
  }

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

  printDepotHeader(depot);

  mFilu->setSqlParm(":depotId", depot.value("DepotId").toInt());
  mFilu->setSqlParm(":fiId",  -1);
  mFilu->setSqlParm(":today",  mToday.toString(Qt::ISODate));
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

void Depots::listOrders()
{
  if(mCmd->isMissingParms())
  {
    if(mCmd->printThisWay("[<Format>] [<DepotFilter>]")) return;

    mCmd->printComment(tr("Format could be [m|t] (machine/ticket)."));
    mCmd->printForInst("t --owner Me");
    mCmd->aided();
    return;
  }

  QSqlQuery* depots = getDepots();
  if(!depots) return;

  if(mCmd->argStr(1).isEmpty()) mOptions.insert("PrintOrder", "Human");
  else if(mCmd->argStr(1) == "m") mOptions.insert("PrintOrder", "Machine");
  else if (mCmd->argStr(1) == "t") mOptions.insert("PrintOrder", "Ticket");
  else warning(FUNC, tr("Unknown argument '%1' ignored.").arg(mCmd->argStr(1)));

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
  printDepotHeader(depot);

  QSqlQuery* orders = mFilu->getOrders(depot.value("DepotId").toInt());

  listOrders(orders, FiluU::eOrderCanceled);
  listOrders(orders, FiluU::eOrderNeedHelp);
  listOrders(orders, FiluU::eOrderAdvice);
  listOrders(orders, FiluU::eOrderActive);
  listOrders(orders, FiluU::eOrderExecuted);
  listOrders(orders, FiluU::eOrderExpired);

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
  if(!bt)
  {
    warning(FUNC, tr("Oops? Absolutely no bars for %1.").arg(pos.value("FiName").toString()));
    return;
  }

  bt->next();

  if(bt->date() > mToday)  // Last bar in DB is newer then requested
  {
    delete bt;
    bt = mFilu->getBars(pos.value("FiId").toInt(), pos.value("MarketId").toInt()
                                                 , mToday.toString(Qt::ISODate));
    if(!bt)
    {
      warning(FUNC, tr("Oops? No bars for %1 at %2.").arg(pos.value("FiName").toString()
                                                         , mToday.toString(Qt::ISODate)));
      return;
    }
    bt->next();
  }

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

  QString fiName    = order.value("FiName").toString();
  QString symbol    = isin(order.value("FiId").toInt());
  int orderId       = order.value("OrderId").toInt();
  int status        = order.value("Status").toInt();
  QString statusTxt = mFilu->orderStatus(status);
  int pieces        = order.value("Pieces").toInt();
  bool buyOrder     = order.value("Buy").toBool();
  double limit      = order.value("Limit").toDouble();
  QString curr      = order.value("Currency").toString();
  QDate oDate       = order.value("ODate").toDate();
  QDate vDate       = order.value("VDate").toDate();
  QString oType     = mFilu->orderType(buyOrder);
  QString market    = order.value("Market").toString();
  QString note      = order.value("Note").toString();

  QString printOrder = mOptions.value("PrintOrder");

  QString limitTxt = limit == 0.0 ? tr("Best") : QString::number(limit, 'f', 2);
  if(printOrder != "Machine")
  {
    if(limit) limitTxt.append(" " + curr);
    else limitTxt.append(QString(fc).repeated(4));
  }

  QString info;
  if(printOrder != "Machine" and status >= FiluU::eOrderActive)
  {
    BarTuple* bars = mFilu->getBars(order.value("FiId").toInt()
                                  , order.value("MarketId").toInt()
                                  , oDate.toString(Qt::ISODate), mToday.toString(Qt::ISODate));

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
        int days = minDate.daysTo(mToday);
        info.append(tr("Low  %L1 %3, %2 d.ago")
                      .arg(min, 7, 'f', 2, fc).arg(days, 2).arg(curr));
      }
      else
      {
        int days = maxDate.daysTo(mToday);
        info.append(tr("High %L1 %3, %2 d.ago")
                      .arg(max, 7, 'f', 2, fc).arg(days, 2).arg(curr));
      }

      info.append(", ");
    }

    info.append(tr("%1 d.left").arg(mToday.daysTo(vDate), 2));
  }

  if(printOrder == "Human")
  {
    if(fiName.length() > 30)
    {
      fiName.truncate(28);
      fiName.append("~+");
    }

    QString text = "%9 %1 %2 %3x %5%6  %7 %8";
    text.replace(' ', fc); // Do it below would looks like
                          // QString text = "%2%1%1%3%1%4x%1%5%1%6%7%1%1%8%9"
                          // and that's pretty confusing

    // Don't print here vDate and market, would be to long, user is clever enough
    text = text.arg(oDate.toString(Qt::ISODate))
              .arg(oType, -4, fc)
              .arg(pieces, 4, 10, fc)
              .arg(fiName, -30, fc)
              .arg(limitTxt, 12, fc)
              .arg(statusTxt, -8, fc)
              .arg(status >= FiluU::eOrderActive ? info : note)
              .arg(orderId);

    print(text);
  }
  else if(printOrder == "Machine")
  {
    QString text = "%11 %1 %2 %3 %4 %5 %6 %7 %8 \"%9\"";
    text.replace(' ', fc); // Do it below would looks like
                          // QString text = "%2%1%1%3%1%4x%1%5%1%6%7%1%1%8%9"
                          // and that's pretty confusing

    text = text.arg(oDate.toString(Qt::ISODate))
               .arg(vDate.toString(Qt::ISODate))
               .arg(symbol, 12, fc)
               .arg(market, 10, fc)
               .arg(pieces, 4, 10, fc)
               .arg(limitTxt, 12, fc)
               .arg(oType, -4, fc)
               .arg(statusTxt, -8, fc)
               .arg(note)
               .arg(orderId);

    text.replace(QRegExp("\\s+"), " ");

    print(text);
  }
  else if(printOrder == "Ticket")
  {
    QString txt = "%1 : %2";
    int width = -10; // Negative value = left-aligned

    print("");
    print(txt.arg(tr("OrderId"), width).arg(orderId));
    print(txt.arg(tr("Date"), width).arg(oDate.toString(Qt::ISODate)));
    print(txt.arg(tr("Type"), width).arg(oType));
    print(txt.arg(tr("Pieces"), width).arg(pieces));
    print(txt.arg(tr("Name"), width).arg(fiName));
    print(txt.arg(tr("Symbol"), width).arg(symbol));
    print(txt.arg(tr("Market"), width).arg(market));
    print(txt.arg(tr("Limit"), width).arg(limitTxt));
    print(txt.arg(tr("Valid"), width).arg(vDate.toString(Qt::ISODate)));
    print(txt.arg(tr("Status"), width).arg(statusTxt));
    print(txt.arg(tr("Note"), width).arg(note));
    if(info.size()) print(txt.arg(tr("Info"), width).arg(info));
//     print(txt.arg(tr(""), width).arg());
  }
}

bool Depots::getOrder(int id, QSqlRecord& order)
{
  mFilu->setSqlParm(":orderId", id);
  mFilu->setSqlParm(":depotId", -1);
  mFilu->setSqlParm(":fiId", -1);
  mFilu->setSqlParm(":status", -1);

  QSqlQuery* query = mFilu->execSql("GetDepotOrders");

  if(query->size() < 1)
  {
    error(FUNC, tr("Order with Id %1 not found.").arg(id));
    return false;
  }

  query->next();
  order = query->record();
  return true;
}

QSqlQuery* Depots::getDepots(const QString& owner)
{
  mFilu->setSqlParm(":owner",  owner);
  mFilu->setSqlParm(":depotId", -1);

  if(mCmd->hasError()) return 0;

  return getDepots2();
}

QSqlQuery* Depots::getDepots(int id)
{
  mFilu->setSqlParm(":depotId", id);

  if(mCmd->hasError()) return 0;

  return getDepots2();
}

QSqlQuery* Depots::getDepots()
{
  mFilu->setSqlParm(":owner",  mCmd->optStr("owner"));
  mFilu->setSqlParm(":depotId", mCmd->optInt("dpid", -1));

  if(mCmd->hasError()) return 0;

  return getDepots2();
}

QSqlQuery* Depots::getDepots2()
{
  QSqlQuery* depots = mFilu->execSql("GetDepots");
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

void Depots::printDepotHeader(const QSqlRecord& depot)
{
  mDP.id         = depot.value("DepotId").toInt();
  mDP.cash       = mFilu->getDepotCash(mDP.id, mToday);
  mDP.neededCash = mFilu->getDepotNeededCash(mDP.id, mToday);
  mDP.value      = mFilu->getDepotValue(mDP.id, mToday);
  mDP.availCash  = mDP.cash - mDP.neededCash;
  mDP.balance    = mDP.value + mDP.cash;

  mFilu->setSqlParm(":depotId", mDP.id);
  mFilu->setSqlParm(":fiId",  -1);
  mFilu->setSqlParm(":today",  mToday.toString(Qt::ISODate));
  QSqlQuery* positions = mFilu->execSql("GetDepotPositionsTraderView");

  if((mDP.balance < mDP.value) or (mDP.value < 0.0))
  {
    warning(FUNC, tr("It seems it lacks currency data."));
  }

  QSqlQuery* orders = mFilu->getOrders(mDP.id, FiluU::eOrderActive);
  int opOrders = 0;
  if(orders) opOrders = orders->size();

  print(tr("DepotId: %1, Name: %2, Owner: %3, Date: %4")
          .arg(mDP.id)
          .arg(depot.value("Name").toString(), depot.value("Owner").toString(), mToday.toString(Qt::ISODate)));

  print(tr("Positions: %2, Value: %L3 %1, AvCash: %L4 %1, OpenOrders: %5")
          .arg(depot.value("Currency").toString())
          .arg(positions->size())
          .arg(mDP.balance, 0, 'f', 2)
          .arg(mDP.availCash, 0, 'f', 2)
          .arg(opOrders));
}

QString Depots::isin(int fiId)
{
  // Search ISIN or as fallback Reuters symbol
  SymbolTuple* st = mFilu->getSymbols(fiId);

  while(st->next()) if(st->owner() == "ISIN") break;

  if(st->owner() != "ISIN")
  {
    // No ISIN, take the first Symbol (which is the lovely one)
    st->rewind();
    st->next();
  }

  QString isin = st->caption();
  delete st;

  return isin;
}
