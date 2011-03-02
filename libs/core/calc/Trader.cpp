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

#include "Trader.h"

#include "Indicator.h"
#include "DataTupleSet.h"
#include "MyParser.h"

// FIXME: Short trades are not implemented. Partial code about short selling
//        should accept as it is with caution

Trader::Trader(FClass* parent)
      : FClass(parent, FUNC)
      , mFeeCalc(new MyParser(this))
      , mIndicator(0)
      , mBarsNeeded(0)
      , mData(0)
      , mFi(0)
{
  mTradingRulePath = mRcFile->getST("TradingRulePath");
}

Trader::~Trader()
{
  delete mFeeCalc;
  if(mIndicator) delete mIndicator;
  foreach(Rule rule, mRules) delete rule.first; // Delete the parser
  mRules.clear();
}

bool Trader::useRuleFile(const QString& fileName)
{
  clearErrors();

  QFile file(mTradingRulePath + fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    error(FUNC, tr("Can't load file '%1'.").arg(file.fileName()));
    return false;
  }

  QTime time;
  time.start();

  // Read/fill the TradingRule
  QTextStream fileStream(&file);
  mOrigRule.clear();

  while(!fileStream.atEnd())
  {
    mOrigRule.append(fileStream.readLine());
  }

  //verbose(FUNC, tr("File read in %1 milliseconds.").arg(time.elapsed()));

  mAutoLoadIndicator = true;

  return parseRule();
}

bool Trader::useRule(const QStringList& rule)
{
  mOrigRule = rule;
  mAutoLoadIndicator = false;

  return parseRule();
}

void Trader::getRule(QStringList& rule)
{
  rule = mOrigRule;
}

bool Trader::useIndicator(const QStringList& indicator)
{
  if(!mIndicator)
  {
    mIndicator = new Indicator(this);
    mIndicator->ignorePlot();
  }

  if(!mIndicator->useIndicator(indicator))
  {
    addErrors(mIndicator->errors());
    return false;
  }

  mBarsNeeded = mIndicator->barsNeeded();

  return true;
}

void Trader::getIndicator(QStringList& indicator)
{
  mIndicator->getIndicator(indicator);
}

bool Trader::prepare(const QSqlRecord& depot)
{
  // What we have todo is:
  // - Load and setup the trading rule
  // - Check open orders
  // - Check portfolio for signals

  if(depot.isEmpty())
  {
    fatal(FUNC, "No depot given."); // Yes, no tr()
    return false;
  }

  // Load and setup the trading rule
  if(!useRuleFile(depot.value("Trader").toString())) return false;

  //
  // Get and set our status
  //
  mLastCheckDate = mRcFile->getDT("LastDepotCheck");
  if(!mLastCheckDate.isValid())
  {
    mLastCheckDate = QDate::currentDate().addDays(-60);
    mRcFile->set("LastDepotCheck", mLastCheckDate.toString(Qt::ISODate));
  }

  mFromDate  = mLastCheckDate.addDays(mBarsNeeded * -1.4);
  mToDate    = QDate::currentDate();
  mDepotId   = depot.value("DepotId").toInt();
  mSettings.insert("DepotName", depot.value("Name").toString());
  mSettings.insert("DepotCurrency", depot.value("Currency").toString());
  mSettings.insert("DepotCurrencyId", depot.value("CurrencyId").toString());
  BrokerTuple* broker = mFilu->getBroker(depot.value("BrokerId").toInt());
  setFeeFormula(broker->feeFormula());
  delete broker;

  double depCash  = mFilu->getDepotCash(mDepotId);
  double depNeedC = mFilu->getDepotNeededCash(mDepotId);
  double depValue = mFilu->getDepotValue(mDepotId);
  mRealVar.insert("Cash", depCash - depNeedC);
  mRealVar.insert("TotalBalance", depValue + depCash);

  QSqlQuery* orders = mFilu->getOrders(mDepotId, FiluU::eOrderActive);

  mFilu->setSqlParm(":depotId", mDepotId);
  mFilu->setSqlParm(":fiId",  -1);
  QSqlQuery* positions = mFilu->execSql("GetDepotPositionsTraderView");

  verbose(FUNC, tr("Check depot: %1, Value: %L3 %2, AvCash: %L4 %2, Positions: %5, OpOrders: %6")
                  .arg(mSettings.value("DepotName"), mSettings.value("DepotCurrency"))
                  .arg(mRealVar.value("TotalBalance"), 0, 'f', 2)
                  .arg(mRealVar.value("Cash"), 0, 'f', 2)
                  .arg(positions->size())
                  .arg(orders->size()) );

  //
  // Check open orders
  //
  if(orders)
  {
    while(orders->next())
    {
      QSqlRecord order = orders->record();
      int pieces    = order.value("Pieces").toInt();
      bool buyOrder = order.value("Buy").toBool();
      double limit  = order.value("Limit").toDouble();
      QDate oDate   = order.value("ODate").toDate();
      QDate vDate   = order.value("VDate").toDate();

      QString oType = mFilu->orderType(buyOrder);
      QString limitTxt = limit == 0.0 ? tr("Best") : QString::number(limit, 'f', 2);
      if(limit) limitTxt.append(" " + order.value("Currency").toString());
      QString verbText = tr("Check order: %2, %1 %5x %3, %4")
                        .arg(oType, oDate.toString(Qt::ISODate), order.value("FiName").toString(), limitTxt)
                        .arg(pieces);

      verbText = verbText.leftJustified(80, '.');
      mSettings.insert("VerboseText", verbText);

      BarTuple* bars = mFilu->getBars(order.value("FiId").toInt(), order.value("MarketId").toInt()
                                    , oDate.addDays(mBarsNeeded * -1.4).toString(Qt::ISODate)
                                    , mToDate.toString(Qt::ISODate));
      if(!bars)
      {
        verbText.append(tr("*** WARNING *** Got no bars."));
        verbose(FUNC, verbText);
        continue;
      }

      mData = new DataTupleSet();
      mData->appendBarTuple(bars, "THIS");

      if(!initVariables()) return false;

      int idx = mData->findDate(oDate);
      if(idx < Tuple::eInRange)
      {
        verbText.append(tr("*** WARNING *** Order date is not in bars."));
        verbose(FUNC, verbText);
        continue;
      }

      mData->rewind(idx - 1);
      int orderId = order.value("OrderId").toInt();
      double price;
      double executedPrice = -1.0;

      if((limit == 0.0) and (buyOrder)) limit = DBL_MAX;

      while(mData->next())
      {
        QDate date;
        mData->getDate(date);

        if(date < oDate) continue;

        if(date > vDate)
        {
          // Order experied
          verbText.append(tr("Experied on %1").arg(date.toString(Qt::ISODate)));
          verbose(FUNC, verbText);

          mFilu->updateField("status", 1, ":user", "order", orderId); // 1=experied
          break;
        }

        if(buyOrder)
        {
          // Buy order
          mData->getValue("OPEN", price);
          if(price < limit)
          {
            executedPrice = price;
          }
          else
          {
            mData->getValue("LOW", price);
            if(price < limit) executedPrice = limit;
            else if(price == limit) executedPrice = -2.0; // Ask whats up
          }
        }
        else
        {
          // Sell order
          mData->getValue("OPEN", price);
          if(price > limit)
          {
            executedPrice = price;
          }
          else
          {
            mData->getValue("HIGH", price);
            if(price > limit) executedPrice = limit;
            else if(price == limit) executedPrice = -2.0; // Ask whats up
          }
        }

        if(executedPrice > -1.0)
        {
          postExecutedOrder(order, date, executedPrice);
          break;
        }
        else if(executedPrice == -2.0)
        {
          // Update DB
          mFilu->updateField("status", 4, ":user", "order", orderId); // 4=ask user

          verbose(FUNC, verbText + tr("Unsure if executed on %1").arg(date.toString(Qt::ISODate)));

          executedPrice = -1.0;
          // Don't break; go on
        }

      } // while(mData->next())

      if(executedPrice == -1)
      {
        verbText.append(tr("Nothing happens."));
        verbose(FUNC, verbText);
      }

      delete mData;
    } // while(orders->next())
  }
  else
  {
    // Because of an error while getting orders we should break here
    return false;
  }

  //
  // Check portfolio for signals
  //
  if(positions)
  {
    while(positions->next())
    {
      QSqlRecord pos = positions->record();

      QString verbText = tr("Check position: %1x %2...")
                           .arg(pos.value("Pieces").toInt(), 4)
                           .arg(pos.value("FiName").toString());

      verbText = verbText.leftJustified(80, '.');
      mSettings.insert("VerboseText", verbText);

      QDate posDate  = pos.value("Date").toDate();
      BarTuple* bars = mFilu->getBars(pos.value("FiId").toInt(), pos.value("MarketId").toInt()
                                    , posDate.addDays(mBarsNeeded * -1.4).toString(Qt::ISODate)
                                    , mToDate.toString(Qt::ISODate));

      if(!bars)
      {
        verbText.append(tr("*** WARNING *** Got no bars."));
        verbose(FUNC, verbText);
        continue;
      }

      mRealVar.insert("Long", pos.value("Pieces").toDouble());
      mRealVar.insert("AvgLong", pos.value("Price").toDouble());
      mRealVar.insert("OffMarket", 0.0);

      // Take here again mLastCheckDate to prevent doing
      // stupid things if --Filu LastDepotCheck='anno x' was given
      mLastCheckDate = mRcFile->getDT("LastDepotCheck");
      if(mLastCheckDate < posDate) mLastCheckDate = posDate;

      if(!check(bars))
      {
        verbText.append(tr("Nothing todo."));
        verbose(FUNC, verbText);
      }

      mInStock.insert(bars->fiId());

    } //while(positions->next())
  }

  if(mRealVar.value("Cash") < mSettings.value("MinPositionSize").toDouble())
  {
    verbose(FUNC, tr("Not enough cash to buy something."));
    return false;
  }

  mRealVar.remove("Long");
  mRealVar.remove("AvgLong");
  mRealVar.remove("OffMarket");

  mLastCheckDate = mRcFile->getDT("LastDepotCheck");
  mSettings.remove("VerboseText");

  verbose(FUNC, "");

  return true;
}

QDate Trader::needBarsFrom()
{
  return mFromDate;
}

QStringList Trader::workOnGroups()
{
  return mSettings.value("WorkOnFiGroup").split(",");
}

bool Trader::check(BarTuple* bars)
{
  // Returns true if you should buy and false...if not

  if(!bars) return false;
  if(mInStock.contains(bars->fiId())) return false;  // Nothing todo, already checked at prepare()

  mData = new DataTupleSet();
  mData->appendBarTuple(bars, "THIS");

  if(!initVariables()) return false;

  // Overwrite some status variables
  foreach(QString var, mRealVar.keys()) setTo(var, mRealVar.value(var));
  setTo("Cash", inFiCurrency(mVariable.value("Cash")));
  setTo("TotalBalance", inFiCurrency(mVariable.value("TotalBalance")));
  setTo("MinPositionSize", inFiCurrency(mVariable.value("MinPositionSize")));

  // Look for open orders and calc our status
  QSqlQuery* orders = mFilu->getOrders(mDepotId, bars->fiId());
  if(orders)
  {
    while(orders->next())
    {
      QSqlRecord order = orders->record();
      if(order.value("Buy").toBool())
      {
        addTo("OOLongBuy", 1);
        addTo("OpenLongBuy", order.value("Pieces").toInt());
        addTo("OpenVolume", order.value("Pieces").toInt() * order.value("Limit").toDouble());
      }
      else
      {
        addTo("OOLongSell", 1);
        addTo("OpenLongSell", order.value("Pieces").toInt());
      }
    }
  }

  // To decide what is todo now could be pretty complicate
  // if we had for some days not checked our depot.
  // I keep it simple and scan *backwards*.
  int idx = mData->dataTupleSize();
  while(idx)
  {
    mData->rewind(--idx);

    QDate date;
    mData->getDate(date);
    if(date <= mLastCheckDate) break;

    calcGain();
    checkOpenOrders();
    // Third, check each rule
    for(int i = 0; i < mRules.size(); ++i)
    {
      double condition;
      int ret = mRules.at(i).first->calc(condition);
      if(ret == 1) continue; // No valid value in mData
      else if(ret == 2)
      {
        error(FUNC, "Bad value from mu::Parser");
        continue;
      }

      if(condition > 0.0) takeActions(mRules.at(i).second);
    }

    if(mOrders.size()) break; // Ok, we have a winner

    // Fourth, sync mData with mVariable
    for(int i = 0; i < mDataAdded.size(); ++i)
      mData->setValue(mDataAdded.at(i), mVariable.value(mDataAdded.at(i)));
    // Fifth, ...play it again Sam
  }

  delete mData;

  if(mOrders.size())
  {
    QStringList o = mOrders.at(0);
    // Order looks like
    // <date>, BUY, <type>, <piece>, <limit value>, <rest validity>, <condition>
    QDate  oDate  = QDate::fromString(o.at(0), Qt::ISODate);
    oDate.addDays(1); // Today we have a signal, the order could only be placed tomorrow
    while(oDate.dayOfWeek() > 5) oDate.addDays(1); // Skip Saturday/Sunday

    QDate  vDate  = oDate.addDays(o.at(5).toInt() * 1.4);
    bool   buy    = o.at(1).startsWith("BUY")  ? true : false;
    int    pieces = o.at(3).toInt();
    double limit  = o.at(4).startsWith("Best") ?  0.0 : o.at(4).toDouble();

    mFilu->addOrder(mDepotId, oDate, vDate, bars->fiId(), pieces
                  , limit, buy, bars->marketId(), 0, o.at(6)); // 0=status=suggestion

    verbose(FUNC, mSettings.value("VerboseText") + tr("Signal: %1").arg(o.at(6)));

    if(verboseLevel(eAmple))
    {
      FiTuple* fi = mFilu->getFi(bars->fiId());
      MarketTuple* market = mFilu->getMarket(bars->marketId());
      QString limitTxt = o.at(4);
      if(limit) limitTxt.append(" " + market->currSymbol());

      verbose(FUNC, tr("Order for %5, %8: %1 at %7: %2x %3, Limit: %4, Validity: %6\n")
                      .arg(o.at(1), o.at(3), fi->name())
                      .arg(limitTxt)
                      .arg(oDate.toString(Qt::ISODate))
                      .arg(vDate.toString(Qt::ISODate))
                      .arg(market->name(), mSettings.value("DepotName")), eAmple);
      delete fi;
    }

    return true;
  }

  return false;
}

void Trader::postExecutedOrder(const QSqlRecord& order, const QDate& execDate, double execPrice)
{
  bool buyOrder  = order.value("Buy").toBool();
  int pieces     = order.value("Pieces").toInt();
  QString fiName = order.value("FiName").toString();
  QString curry  = order.value("Currency").toString();
  QString oType  = mFilu->orderType(buyOrder);

  QString verbText = mSettings.value("VerboseText")
                   + tr("Executed on %1 at %L3 %2").arg(execDate.toString(Qt::ISODate), curry)
                                                   .arg(execPrice);
  verbose(FUNC, verbText);

  // Update DB
  mFilu->updateField("status", 2, ":user", "order", order.value("OrderId").toInt()); // 2=executed

  int p = execPrice < 2.0 ? 4 : 2; // Precision
  QString txt = tr("%1 %4x %2 at %L5 %3").arg(oType, fiName, curry).arg(pieces).arg(execPrice, 0, 'f', p);
  if(mSettings.value("FiCurrencyId") != mSettings.value("DepotCurrencyId"))
  {
    txt.append(QString(", %1/%2=%3").arg(curry, mSettings.value("DepotCurrency")).arg(inDepotCurrency(1.0)));
  }
  double volume = inDepotCurrency(execPrice) * pieces;

  if(buyOrder)
  {
    mFilu->addDepotPos(mDepotId, execDate, order.value("FiId").toInt()
                      , pieces, execPrice
                      , order.value("MarketId").toInt()
                      , order.value("Note").toString());

    mFilu->addAccPosting(mDepotId, execDate, FiluU::ePostFiBuy, txt, -volume);
  }
  else
  {
    mFilu->addDepotPos(mDepotId, execDate, order.value("FiId").toInt()
                      , -pieces, execPrice
                      , order.value("MarketId").toInt()
                      , order.value("Note").toString());

    mFilu->addAccPosting(mDepotId, execDate, FiluU::ePostFiSell, txt, volume);
  }

  // Add commission to account
  txt = tr("Commission for %1 %3x %2").arg(oType, fiName).arg(pieces);
  mFilu->addAccPosting(mDepotId, execDate, FiluU::ePostFee, txt, calcFee(volume));
}

bool Trader::parseRule()
{
  QTime time;
  time.start();

  clearErrors();
  mLineNumber = 0;
  mOkSettings = false;
  mOkRules    = false;
  while(nextLine(eNextBlock))
  {
    if(mLine.startsWith("[Settings]")) readSettings();
    else if(mLine.startsWith("[Rules]")) readRules();
  }

  if(!mOkSettings) error(FUNC, tr("No [Settings] in rule file."));
  if(!mOkRules)    error(FUNC, tr("No [Rules] in rule file."));

  if(hasError()) return false;

  //verbose(FUNC, tr("Rule setup in %1 milliseconds.").arg(time.elapsed()));
  return true;
}

bool Trader::nextLine(bool nextBlock/* = false*/)
{
  while(true)
  {
    ++mLineNumber;
    if(mLineNumber > mOrigRule.size()) return false;

    mOrigLine = mOrigRule.at(mLineNumber - 1);
    mLine = mOrigLine.trimmed();
    if(mLine.startsWith("["))
    {
      if(nextBlock) return true;
      else
      {
        --mLineNumber;
        return false;
      }
    }

    if(mLine.startsWith("*")) continue; // Ignore remarks
    if(mLine.isEmpty()) continue;

    return true;
  }

  fatal(FUNC, "??? you should never read this.");
  return false;
}

void Trader::readSettings()
{
  mOkSettings = true;

  mSettings.clear();
  mSettings.insert("UseIndicator", "");
  mSettings.insert("GainRefLong", "CLOSE");
  mSettings.insert("GainRefShort", "CLOSE");
  mSettings.insert("PeakRefLong", "HIGH");
  mSettings.insert("FeeFormula", "");
  mSettings.insert("InitialCash", "5000");
  mSettings.insert("MinPositionSize", "");
  mSettings.insert("WorkOnFiGroup", "");
  //mSettings.insert("", "");

  while(nextLine())
  {
    if(!mLine.contains("="))
    {
      error(FUNC, tr("No equal sign at line %1.").arg(QString::number(mLineNumber)));
      continue;
    }

    mLine.remove(" "); // Not at nextLine(), because muParser needs sometimes a blank
    QStringList setting = mLine.split("=");
    if(setting.size() < 2)
    {
      error(FUNC, tr("No right argument at line %1.").arg(QString::number(mLineNumber)));
      continue;
    }

    if(!mSettings.contains(setting.at(0)))
    {
      error(FUNC, tr("Unknown setting at line %1.").arg(QString::number(mLineNumber)));
      continue;
    }

    mSettings.insert(setting.at(0), setting.at(1));
  }//while(nextLine());

  // Check if MinPositionSize was set
  if(mSettings.value("MinPositionSize").isEmpty())
  {
    mSettings.insert("MinPositionSize", QString::number(mSettings.value("InitialCash").toDouble()  * 0.20));
  }

  // Set the fee formula
  setFeeFormula(mSettings.value("FeeFormula"));

  // Load the indicator...or not
  if(mAutoLoadIndicator)
  {
    if(!mIndicator)
    {
      mIndicator = new Indicator(this);
      mIndicator->ignorePlot();
    }

    if(!mIndicator->useFile(mSettings.value("UseIndicator")))
    {
      error(FUNC, tr("Problem while loading used indicator."));
      addErrors(mIndicator->errors());
      return;
    }

    mBarsNeeded = mIndicator->barsNeeded();
  }

//   if(verboseLevel(eMax))
//   {
//     QHashIterator<QString, QString> i(mSettings);
//     while(i.hasNext())
//     {
//       i.next();
//       verbose(FUNC, QString("Key:%1, Valule:%2").arg(i.key(), i.value()));
//     }
//   }
}

void Trader::readRules()
{
  mOkRules = true;

  // Clear rule if it's not the first run
  foreach(Rule rule, mRules) delete rule.first; // Delete the parser
  mRules.clear();

  while(nextLine())
  {
    if(!mLine.contains(":"))
    {
      error(FUNC, tr("Missing colon at line %1.").arg(mLineNumber));
      continue;
    }

    // mLine looks like "ENTERLONG & OffMarket :BUY(Long, 100%);"
    QStringList rule = mLine.split(":");
    if(rule.size() < 2)
    {
      error(FUNC, tr("No right argument at line %1.").arg(mLineNumber));
      continue;
    }

    mRules.append(Rule());                    // Append an empty new rule
    MyParser* parser = new MyParser(this);
    parser->useVariables(&mVariable);
    parser->setExp(rule.at(0).trimmed());

    rule[1].replace("(",",");
    rule[1].remove(")");
    rule[1].remove(" ");                      // Looks now "BUY,Long,100%;"

    // More than one action per condition is supported
    if(rule.at(1).endsWith(";")) rule[1].chop(1);
    QList<QStringList> actions;
    QStringList actionList = rule.at(1).split(";");
    QSet<QString> knownActions;
    knownActions << "BUY" << "SELL";
    for(int i = 0; i < actionList.size(); ++i)
    {
      // Full format is BUY(<type>, <size>, <limit>, <validity>)
      QStringList action = actionList.at(i).split(",");

      if(!knownActions.contains(action.at(0)))
      {
        error(FUNC, tr("Unknown action '%1' at line %2.").arg(action.at(0)).arg(mLineNumber));
        continue;
      }

      if(action.size() == 3) action << "OPEN" << "20";
      if(action.size() == 4) action << "20";
      if(action.size() != 5)
      {
        error(FUNC, tr("Wrong parameter count at line %1.").arg(mLineNumber));
        errInfo(FUNC, mOrigLine);
        continue;
      }
      action[2].remove("%");
      // Append the condition. So it's later easier to understand
      // why the trader does something
      action.append(rule.at(0).trimmed());
      actions.append(action);
    }

    mRules.last().first = parser;             // Set the parser
    mRules.last().second = actions;           // Set the actions
  }//while(nextLine());

//   if(verboseLevel(eMax))
//   {
//     for(int i = 0; i < mRules.size(); ++i)
//     {
//       verbose(FUNC, "Condition:" + mRules.at(i).first->getExp());
//       verbose(FUNC, "Action:" + mRules.at(i).second);
//       verbose(FUNC, "");
//     }
//   }

}

void Trader::appendMData()
{
  // Add trading calculation variables to mData,
  // uncomment some or add more if you like
  appendToMData("Cash");
  appendToMData("TotalBalance");
  appendToMData("OOLongBuy");
  appendToMData("OOLongSell");
  //appendToMData("OOShortBuy");
  //appendToMData("OOShortSell");
  appendToMData("Long");
  //appendToMData("Short");
  appendToMData("OffMarket");
  appendToMData("GainLong");
  //appendToMData("GainShort");
  appendToMData("Gain");
  appendToMData("WonMoney");
  appendToMData("LostMoney");
  //appendToMData("");
}

bool Trader::setFeeFormula(const QString& exp)
{
  if(exp.isEmpty())
  {
    mFeeCalc->setExp("10 + OV * 0.001");
    return true;
  }

  verbose(FUNC, tr("Set fee formula to: %1").arg(exp), eAmple);

  if(!mFeeCalc->setExp(exp))
  {
    error(FUNC, tr("Can't set fee formula."));
    addErrors(mFeeCalc->errors());
    return false;
  }

  if(verboseLevel(eInfo))
  {
    double ov;
    ov = 1000.0;
    verbose(FUNC, tr("Fee for order volume %L1 would be %L2.").arg(ov, 0, 'f', 2).arg(calcFee(ov), 0, 'f', 2), eAmple);
    ov = 100000.0;
    verbose(FUNC, tr("Fee for order volume %L1 would be %L2.").arg(ov, 0, 'f', 2).arg(calcFee(ov), 0, 'f', 2), eAmple);
  }

  return true;
}

double Trader::calcFee(double& volume)
{
  static double* ovVar;

  if(ovVar != &volume)
  {
    ovVar = &volume;
    mFeeCalc->useVariable("OV", volume);
  }

  double fee;
  mFeeCalc->calc(fee);

  return fee;
}

bool Trader::initVariables()
{
  mOpenOrders.clear();
  mOrders.clear();
  mReport.clear();
  mDataAdded.clear();

  // We can't call mVariable.clear() because mu::Parser had register
  // the addresses of the hash variables. But we mußt erease the variables
  // used by mu::Parser if this is not the first run of simulate()
  for(int i = 0; i < mDataAdded.size(); ++i)
    setTo(mDataAdded.at(i), 0.0);

  if(!mIndicator->calculate(mData)) return false;

//   verbose(FUNC, tr("Indicator calculated in %1 ms.").arg(time.restart()));

  // Give each Parser at this point mData. Here includes mData only
  // indicator variables. So the Parser can check which of the variables
  // he use he have to read out of mData. This is important because
  // the Parser can only use mVariable to do the job. So if he have to
  // calc an expression with a mData variable he must copy them into mVariable.
  // But because mVariable includes already all used mData variable names by
  // the Parser he can later not distinguish between mData and mVariable
  // variables, what could make big trouble...
  for(int i = 0; i < mRules.size(); ++i)
  {
    mRules.at(i).first->useData(mData);
  }

  appendMData();

  // Set status variables
  setTo("Cash", mSettings.value("InitialCash").toDouble());
  setTo("TotalBalance", mSettings.value("InitialCash").toDouble());
  setTo("MinPositionSize", mSettings.value("MinPositionSize").toDouble());
  setTo("EntryBalanceLong", 0.0);
  //setTo("", mSettings.value("").toDouble());


  setTo("OOLongBuy", 0.0);
  setTo("OOLongSell", 0.0);
  setTo("OOShortBuy", 0.0);
  setTo("OOShortSell", 0.0);
  setTo("OpenLongBuy", 0.0);
  setTo("OpenLongSell", 0.0);
  setTo("OpenVolume", 0.0);
  setTo("Long", 0.0);
  setTo("Short", 0.0);
  setTo("OffMarket", 1.0);
  setTo("AvgLong", 0.0);
  setTo("RAvgLong", 0.0);
  setTo("AvgShort", 0.0);
  setTo("RAvgShort", 0.0);
  setTo("GainLong", 0.0);
  setTo("GainShort", 0.0);
  setTo("Gain", 0.0);
  setTo("PeakGainLong", 0.0);
  setTo("WonL", 0.0);
  setTo("LostL", 0.0);
  setTo("WonMoney", 0.0);
  setTo("LostMoney", 0.0);
  setTo("SumGainL", 0.0);
  setTo("SumLostL", 0.0);
  setTo("TotalCommission", 0.0);
  //setTo("", 0.0);

  // Got the currency of the FI and save the question for later reuse
  // so that we not each time to fetch the market
  int fiId, marketId;
  mData->getIDs("THIS", fiId, marketId);
  QString curr = mSettings.value(QString("MarketCurrency%1").arg(marketId));
  if(curr.isEmpty())
  {
    MarketTuple* market = mFilu->getMarket(marketId);
    curr = QString::number(market->currId());
    mSettings.insert(QString("MarketCurrency%1").arg(marketId), curr);
    mSettings.insert("FiCurrencyId", curr);
    if(mSettings.contains("SimulationOnly")) mSettings.insert("DepotCurrencyId", curr);

    // Test if we can convert money
    clearErrors();
    inFiCurrency(1.0);
    if(mFilu->hasError()) // FIXME: don't ask filu
    {
      verbose(FUNC, tr("WARNING No data to convert currency %1, printed pieces are wrong.").arg(market->currName()));
      mSettings.insert(QString("MarketCurrency%1").arg(marketId), mSettings.value("DepotCurrencyId"));
    }
    delete market;
  }
  else
  {
    mSettings.insert("FiCurrencyId", curr);
    if(mSettings.contains("SimulationOnly")) mSettings.insert("DepotCurrencyId", curr);
  }

  if(mSettings.value("FiCurrencyId") != mSettings.value("DepotCurrencyId"))
  {
    // Convert all money data
    setTo("Cash", inFiCurrency(mVariable.value("Cash")));
    setTo("TotalBalance", inFiCurrency(mVariable.value("TotalBalance")));
    setTo("MinPositionSize", inFiCurrency(mVariable.value("MinPositionSize")));
  }

  return true;
}

double Trader::inFiCurrency(double money)
{
  if(mSettings.value("FiCurrencyId") == mSettings.value("DepotCurrencyId"))
    return money; // Nice, nothing todo

  QDate date;
  mData->getDate(date);
  if(!date.isValid())
  {
    mData->rewind(mData->dataTupleSize() - 1);
    mData->getDate(date);
    mData->rewind();
  }
  QString dateString = date.toString(Qt::ISODate);
  mFilu->convertCurrency(money
                       , mSettings.value("DepotCurrencyId").toInt()
                       , mSettings.value("FiCurrencyId").toInt()
                       , date);

  if(mFilu->hasError())
  {
    // Don't try again to convert money
    mSettings.insert("FiCurrencyId", mSettings.value("DepotCurrencyId"));
  }

  return money;
}

double Trader::inDepotCurrency(double money)
{
  if(mSettings.value("FiCurrencyId") == mSettings.value("DepotCurrencyId"))
    return money; // Nice, nothing todo

  QDate date;
  mData->getDate(date);
  if(!date.isValid())
  {
    mData->rewind(mData->dataTupleSize() - 1);
    mData->getDate(date);
    mData->rewind();
  }
  QString dateString = date.toString(Qt::ISODate);

  mFilu->convertCurrency(money
                       , mSettings.value("FiCurrencyId").toInt()
                       , mSettings.value("DepotCurrencyId").toInt()
                       , date);

  if(mFilu->hasError())
  {
    // Don't try again to convert money
    mSettings.insert("FiCurrencyId", mSettings.value("DepotCurrencyId"));
  }

  return money;
}

bool Trader::simulate(DataTupleSet* data)
{
  if(hasError()) return false;

  if(data->dataTupleSize() < mBarsNeeded)
  {
    error(FUNC, tr("Too less bars for simulation."));
    return false;
  }

//   QTime time;
//   time.start();

  mData = data;

  // Don't throw a currency warning if we only simulate.
  // We have no depot currency in that case.
  // FIXME: Looks bad or ok?
  mSettings.insert("SimulationOnly", "Yes");

  if(!initVariables()) return false;
//   verbose(FUNC, tr("Variables setup in").arg(time.restart()));

  QDate date;
  QStringList report;

  mData->rewind(mBarsNeeded - 1);
  mData->getDate(date);
  report << date.toString(Qt::ISODate) << "Begin of Simulation";
  mReport.append(report);

  mData->rewind(mBarsNeeded - 2);
  while(mData->next())
  {
    // First, calculate our status
    calcGain();

    // Second, check open orders...
    checkOpenOrders();

    // Third, check each rule
    for(int i = 0; i < mRules.size(); ++i)
    {
      double condition;
      int ret = mRules.at(i).first->calc(condition);
      if(ret == 1) continue; // No valid value in mData
      else if(ret == 2)
      {
        error(FUNC, "Bad value from mu::Parser");
        continue;
      }

      if(condition > 0.0) takeActions(mRules.at(i).second);
    }

    // Fourth, sync mData with mVariable
    for(int i = 0; i < mDataAdded.size(); ++i)
      mData->setValue(mDataAdded.at(i), mVariable.value(mDataAdded.at(i)));

    // Fifth, ...play it again Sam
  }

  mData->rewind(data->dataTupleSize() - 1);
  mData->getDate(date);
  report.clear();
  report << date.toString(Qt::ISODate) << "End of Simulation";
  mReport.append(report);

  double v1, v2;
  report.clear(); report << "[Score]"; mReport.append(report);

  int fiId, marketId;
  if(!mData->getIDs("THIS", fiId, marketId))
  {
    fatal(FUNC, "No IDs in mData (!?)");
    return false;
  }

  SymbolTuple* st = mFilu->getSymbols(fiId);
  if(!st)
  {
    error(FUNC, tr("Could not find symbols to Fi (!?)."));
    return false;
  }

  while(st->next()) if(st->marketId() == marketId) break;

  report.clear(); report << "Simulated Fi Symbol" << st->caption() << "FIS";
  mReport.append(report);
  report.clear(); report << "Simulated Fi Id" << QString::number(fiId) << "FIID";
  mReport.append(report);
  report.clear(); report << "Simulated on Market Symbol" << st->market() << "MS";
  mReport.append(report);
  report.clear(); report << "Simulated on Market Id" << QString::number(marketId) << "MI";
  mReport.append(report);

  delete st;

  v1 = mVariable.value("WonL") + mVariable.value("LostL")/*+short W/L */;
  appendReport("Total count of trades", v1, "TCT", 0);

  v2 = mVariable.value("WonL");
  appendReport("Won long trades", v2, "WL", 0);

  v2 = mVariable.value("LostL");
  appendReport("Lost long trades", v2, "LL", 0);

  if(v1 > 0.0)
  {
    v2 = 100 * mVariable.value("WonL") / v1;
    appendReport("Won long trades %", v2, "pWL");

    v2 = 100 * mVariable.value("LostL") / v1;
    appendReport("Lost long trades %", v2, "pLL");
  }
  else
  {
    appendReport("Won long trades %", 0.0, "pWL");
    appendReport("Lost long trades %", 0.0, "pLL");
  }

  if(mVariable.value("WonL") > 0.0) v2 = mVariable.value("SumGainL") / mVariable.value("WonL");
  else v2 = 0.0;
  appendReport("Avg gain of won lt %", v2, "pAGL");
  double agwl = v2;

  if(mVariable.value("LostL") > 0.0) v2 = mVariable.value("SumLostL") / mVariable.value("LostL");
  else v2 = 0.0;
  appendReport("Avg lost of lost lt %", v2, "pALL");
  double allt = v2;

  //ln(exp(won long trades * Avg gain of won lt%)/exp(lost long trades * Avg lost of lost lt%)) / total count of trades
  if(v1 > 0.0) v2 = log(exp(mVariable.value("WonL") * agwl) / exp(mVariable.value("LostL") * allt)) / v1;
  else v2 = 0.0;
  appendReport("Score", v2, "SCORE");

  v2 = mSettings.value("InitialCash").toDouble();
  v1 = 100 * (mVariable.value("TotalBalance") - v2) / v2;
  appendReport("Total Performance %", v1, "pTP");
  appendReport("InitialCash", v2, "IC");

  v1 = mVariable.value("WonMoney");
  appendReport("Total won money", v1, "WM");

  v1 = mVariable.value("LostMoney");
  appendReport("Total lost money", v1, "LL");

  if(mVariable.value("WonL") > 0.0) v1 = mVariable.value("WonMoney") / mVariable.value("WonL");
  else v1 = 0.0;
  appendReport("Ratio WonMoney/WonL", v1, "rWMWL");

  if(mVariable.value("LostL") > 0.0) v1 = mVariable.value("LostMoney") / mVariable.value("LostL");
  else v1 = 0.0;
  appendReport("Ratio LostMoney/LostL", v1, "rLMLL");

  v1 = mVariable.value("TotalBalance");
  appendReport("TotalBalance at simulation end", v1, "TB");

  v1 = mVariable.value("TotalCommission");
  appendReport("Payed commission", v1, "TC");

//    v1 = mVariable.value("");
//    appendReport("", v1);

  //verbose(FUNC, "Trader::simulate() simulation done in" << time.restart() << "milliseconds";

//    for(int i = 0; i < mReport.size(); ++i) verbose(FUNC, mReport.at(i);

  return true;
}

int Trader::prepare(const QDate& fromDate, const QDate& toDate)
{
  // We do here two jobs:
  // 1st, and that's the major task, adjust the fromDate
  // 2nd, return the amound of Fi in the group to use for a forcast
  //      of the needet run time
  // FIXME: add to the group search the "path". currently the first fitting
  //        group is used, anyway who is the mother group

  QString group = mSettings.value("WorkOnFiGroup");

  if(group.isEmpty())
  {
    error(FUNC, tr("No group set to use."));
    return -1;
  }

  QSqlQuery* allGroups = mFilu->getGroups();
  if(!allGroups)
  {
    error(FUNC, tr("No groups found."));
    return -1;
  }

  bool found = false;
  while(allGroups->next())
  {
    if(allGroups->value(1).toString() == group)
    {
      found = true;
      break;
    }
  }

  if(!found)
  {
    error(FUNC, tr("Group '%1' not found.").arg(group));
    return -1;
  }

  mFi = mFilu->getGMembers(allGroups->value(0).toInt());

  if(!mFi)
  {
    error(FUNC, tr("No FI found in group '%1'.").arg(group));
    return -1;
  }

  // Adjust the fromDate. we want start with the simulation at fromDate.
  // But because the indicator needs a minumum amount of bars to produce
  // a result, we have to sub a fitting count of days...
  // FIXME: (7 days a week, 5days open markets) you know a better formula?
  mFromDate = fromDate.addDays(mBarsNeeded * -1.4);
  mToDate   = toDate;

  return mFi->size();
}

int Trader::simulateNext()
{
  // Returns
  //   0 if no more Fi left to simulate
  //   1 all looks fine
  //   2 any problem while simulation

  if(!mFi)
  {
    fatal(FUNC, "No mFi");
    return 0;
  }

  if(!mFi->next()) return 0;

  //verbose(FUNC, "simsalabim" << mFi->value(2).toString() << mFi->value(3).toString();

  BarTuple* bars = mFilu->getBars( mFi->value(1).toInt(), mFi->value(4).toInt(),
                                   mFromDate.toString(Qt::ISODate),
                                   mToDate.toString(Qt::ISODate) );

  if(!bars)
  {
    verbose(FUNC, QString("No bars for: %1, %2").arg(mFi->value(2).toString(), mFi->value(3).toString()), eEver);
    return 2; // Don't break complete simulation
  }

  DataTupleSet* data = new DataTupleSet();
  data->appendBarTuple(bars, "THIS");

  if(!simulate(data))
  {
    delete data;
    delete bars;
    return 2;
  }

  delete data;
  delete bars;

  return 1;
}

void Trader::getOrders(QList<QStringList>& orders)
{
  orders = mOrders;
}

void Trader::getReport(QList<QStringList>& report)
{
  report = mReport;
}

void Trader::getVariablesList(QSet<QString>* list)
{
  mIndicator->getVariableNames(list);

  if(mDataAdded.isEmpty())
  {
    DataTupleSet* dts = new DataTupleSet();

    mData = dts;

    appendMData();

    for(int i = 0; i < mDataAdded.size(); ++i) list->insert(mDataAdded.at(i));

    delete dts;
    mDataAdded.clear();

    return;
  }

  for(int i = 0; i < mDataAdded.size(); ++i) list->insert(mDataAdded.at(i));
}

void Trader::takeActions(const QList<QStringList>& actions)
{
  for(int i = 0; i < actions.size(); ++i)
  {
    if(actions.at(i).at(0) == "BUY") actionBuy(actions.at(i));
    if(actions.at(i).at(0) == "SELL") actionSell(actions.at(i));
  }
}

void Trader::actionBuy(const QStringList& action)
{
  // action looks like
  // <BUY>, <Long/Short>, <size in %>, <limit>, <validity>, <condition>
  // BUY, Long, 20, OPEN, 5, SCAN4

  QDate date;
  mData->getDate(date);
  QString dateString = date.toString(Qt::ISODate);

  if(action.at(1) == "Long")
  {
    // Copy, because we add some comment to newOrder
    QStringList newOrder = action;

    // Get some needed values
    double orderSize, limit;
    orderSize = action[2].toDouble();
    if(orderSize > 100.0) orderSize = 100.0; // No jokes
    mData->getValue(action.at(3), limit);

    double cash = mVariable.value("Cash");
    double openVolume = mVariable.value("OpenVolume");
    double availableCash = cash - openVolume;
    if(availableCash < mVariable.value("MinPositionSize")) return; // Not enough cash left

    // Desired order volume is...
    // And take care it's enough cash left in the future
    int pieces = static_cast<int>((availableCash * orderSize / 100) / limit);
    double orderVolume = pieces * limit;
    if(orderVolume < mVariable.value("MinPositionSize"))
    {
      pieces = static_cast<int>(mVariable.value("MinPositionSize") / limit);

      QString txt = "order size too small, buy more";

      if(pieces == 0)
      {
        pieces = static_cast<int>(availableCash / limit);
        if(pieces == 0) return; // Not enough money left
        txt = "order size too small, buy for all money left";
      }

      orderVolume = pieces * limit;
      newOrder.append(txt);
    }


//     if(orderVolume < mVariable.value("MinPositionSize"))
//     {
//       //verbose(FUNC, "Trader::actionBuy() not enough cash left";
//     }
//     else
//     {

      // Update some status variables
      addTo("OpenVolume", orderVolume);
      addTo("OOLongBuy", 1.0);
      addTo("OpenLongBuy", (double)pieces);

      newOrder.replace(2, QString::number(pieces));

      // In case of OPEN set limit to a very big value
      if(action.at(3) == "OPEN")
      {
        newOrder.replace(3, QString::number(DBL_MAX));
        mOpenOrders.append(newOrder);
        // Change once more for real uses
        newOrder.replace(3, "Best");
        newOrder.prepend(dateString);
        mOrders.append(newOrder);
      }
      else
      {
        newOrder.replace(3, QString::number(limit,'f',2));
        mOpenOrders.append(newOrder);
        newOrder.prepend(dateString);
        mOrders.append(newOrder);
      }

      mReport.append(newOrder);

    //}
  }
  else
  {
    error(FUNC, "FIXME: Short orders not implemented.");
  }
}

void Trader::actionSell(const QStringList& action)
{
  // action looks like
  // <SELL>, <Long/Short>, <size in %>, <limit>, <validity>, <condition>
  // SELL, Long, 100, OPEN, 5, Gain > 50

  QDate date;
  mData->getDate(date);
  QString dateString = date.toString(Qt::ISODate);

  if(action.at(1) == "Long")
  {
    // Check if it's something in depot to sell...
    if( (mVariable.value("Long") -  mVariable.value("OpenLongSell")) < 1 )
    {
      return;
    }

    // Copy, because we add some comment to newOrder
    QStringList newOrder = action;

    // Get some needed values
    double orderSize, limit;
    orderSize = action[2].toDouble();
    if(orderSize > 100.0) orderSize = 100.0; // No jokes
    mData->getValue(action.at(3), limit);

    // Calc orderSize to pieces...
    int inDepot = static_cast<int>(mVariable.value("Long"));
    int pieces  = static_cast<int>(inDepot * orderSize / 100);

    double orderVolume = limit * pieces;

    if(orderVolume < mVariable.value("MinPositionSize"))
    {
      QString txt = "order volume to small, sell more";

      pieces = static_cast<int>(mVariable.value("MinPositionSize") / limit) + 1;
      if(pieces > inDepot) pieces = inDepot;

      orderVolume = limit * pieces;

      if(orderSize != 100.0) newOrder.append(txt);
    }

    double restPosition = ( limit * inDepot ) - orderVolume;

    if(restPosition < mVariable.value("MinPositionSize"))
    {
      pieces = inDepot;
      orderVolume = limit * pieces;

      if(orderSize != 100) newOrder.append("rest position too small, sell all");
    }

    newOrder.replace(2, QString::number(pieces));

    // In case of OPEN set limit to a very tiny value
    if(action.at(3) == "OPEN")
    {
      newOrder.replace(3, QString::number(DBL_EPSILON));
      mOpenOrders.append(newOrder);
      // Change once more for real uses
      newOrder.replace(3, "Best");
      newOrder.prepend(dateString);
      mOrders.append(newOrder);
    }
    else
    {
      newOrder.replace(3, QString::number(limit,'f',2));
      mOpenOrders.append(newOrder);
      newOrder.prepend(dateString);
      mOrders.append(newOrder);
    }

    mReport.append(newOrder);

    addTo("OOLongSell", 1.0);
    addTo("OpenLongSell", (double)pieces);
  }
  else
  {
    error(FUNC, "FIXME: Short orders not implemented.");
  }
}

void Trader::checkOpenOrders()
{
  // mOpenOrders is a QList<QStringList>. One order entry looks like
  // <BUY/SELL>, <Long/Short>, <size in piece>, <limit>, <validity>, <condition>
  // BUY, Long, 97, OPEN, 5, SCAN4
  // See also doc/trading-rule-file-format.txt

  int i = 0;
  while(i < mOpenOrders.size())
  {
    if(mOpenOrders.at(i).at(0) == "BUY") checkOpenBuyOrder(mOpenOrders[i]);
    if(mOpenOrders.at(i).at(0) == "SELL") checkOpenSellOrder(mOpenOrders[i]);

    if(  (mOpenOrders.at(i).at(4) == "executed")
      or (mOpenOrders.at(i).at(4) == "expired")  )
    {
      mOpenOrders.removeAt(i);
      continue;
    }

    ++i;
  }
}

void Trader::checkOpenBuyOrder(QStringList& order)
{
  // order looks like
  // BUY, <type>, <piece>, <limit value>, <rest validity>, <condition>
  // BUY, Long, 97, 123.45, 5, SCAN4

  QDate date;
  mData->getDate(date);
  QString dateString = date.toString(Qt::ISODate);

  double price, limit, orderSize, orderVolume;
  double executedPrice = -1.0;
  if(order.at(1) == "Long")
  {
    limit = order.at(3).toDouble();
    mData->getValue("OPEN", price);

    if(price <= limit) executedPrice = price;
    else
    {
      mData->getValue("LOW", price);
      if(price <= limit) executedPrice = limit;
    }

    if(executedPrice > 0.0)
    {
      orderSize = order.at(2).toDouble();
      orderVolume = orderSize * executedPrice;
      setTo("AvgLong", (mVariable.value("AvgLong") * mVariable.value("Long") + orderVolume) / (mVariable.value("Long") + orderSize));
      setTo("RAvgLong", (mVariable.value("RAvgLong") * mVariable.value("Long") + orderVolume) / (mVariable.value("Long") + orderSize));
      setTo("PeakGainLong", 0.0);
      if(mVariable.value("EntryBalanceLong") == 0.0) setTo("EntryBalanceLong", mVariable.value("TotalBalance"));
      else setTo("EntryBalanceLong", (mVariable.value("EntryBalanceLong") + mVariable.value("TotalBalance")) / 2);

      addTo("Long", orderSize);
      addTo("Cash", -orderVolume);

      addTo("OOLongBuy", -1.0);
      addTo("OpenLongBuy", -orderSize);

      if(mVariable.value("OOLongBuy") + mVariable.value("OOShortBuy") == 0)  setTo("OpenVolume", 0.0);
      else addTo("OpenVolume", -orderVolume);

      price = calcFee(orderVolume);
      addTo("Cash", -price);
      addTo("TotalCommission", price);

      order.replace(3, QString::number(executedPrice,'f',2));
      order.replace(4, "executed");
      order.prepend(dateString);
      mReport.append(order);
      return;
    }

  }
  else // Type = "Short"
  {
    error(FUNC, "FIXME: Short orders not implemented.");
  }

  bool isNumber;
  int validity = order.at(4).toInt(&isNumber);

  if(isNumber) order.replace(4, QString::number(--validity));
  else validity = 0;

  if(validity < 1)
  {
    addTo("OOLongBuy", -1.0);
    if(mVariable.value("OOLongBuy") + mVariable.value("OOShortBuy") == 0)  setTo("OpenVolume", 0.0);
    else
    {
      // Calculate a circa open order volume
      orderSize = order.at(2).toDouble();
      mData->getValue("OPEN", price);
      orderVolume = orderSize * price;
      addTo("OpenVolume", -orderVolume);
    }

    order.replace(4, "expired");
    order.prepend(dateString);
    mReport.append(order);
  }
}

void Trader::checkOpenSellOrder(QStringList& order)
{
  // order looks like
  // SELL, <type>, <piece>, <limit value>, <rest validity>, <condition>
  // SELL, Long, 97, 123.45, 5, Gain > 50

  QDate date;
  mData->getDate(date);
  QString dateString = date.toString(Qt::ISODate);

  double price;
  double limit;
  double orderSize = 0.0;
  double orderVolume;
  double executedPrice = -1.0;
  if(order.at(1) == "Long")
  {
    limit = order.at(3).toDouble();
    mData->getValue("OPEN", price);

    if(price >= limit) executedPrice = price;
    else
    {
      mData->getValue("HIGH", price);
      if(price >= limit) executedPrice = limit;
    }

    if(executedPrice > 0.0)
    {
      orderSize = order.at(2).toDouble();
      orderVolume = orderSize * executedPrice;
      double rAvgLong = mVariable.value("RAvgLong");
      double entryVolume = orderSize * rAvgLong;
      //double percentGain = 100 * (executedPrice - rAvgLong) / rAvgLong;
      double percentGain = 100 * ((mVariable.value("TotalBalance") / mVariable.value("EntryBalanceLong")) - 1);

      // Check if was a win or a lost trade
      if(executedPrice > rAvgLong)
      {
        addTo("WonL", 1.0);
        addTo("WonMoney", orderVolume - entryVolume);
        //if(mVariable.value("SumGainL") == 0.0) setTo("SumGainL", percentGain);
        //else setTo("SumGainL", (mVariable.value("SumGainL") + percentGain) / 2);
        addTo("SumGainL", percentGain);
      }
      else
      {
        addTo("LostL", 1.0);
        addTo("LostMoney", entryVolume - orderVolume);
        //if(mVariable.value("SumLostL") == 0.0) setTo("SumLostL", -percentGain);
        //else setTo("SumLostL", (mVariable.value("SumLostL") - percentGain) / 2);
        addTo("SumLostL", -percentGain);
      }

      // "reset" gain calculation
      setTo("AvgLong", executedPrice);
      setTo("PeakGainLong", 0.0);
      //setTo("EntryBalanceLong", mVariable.value("TotalBalance"));

      addTo("Long", -orderSize);
      addTo("Cash", orderVolume);
      addTo("OOLongSell", -1.0);
      addTo("OpenLongSell", -orderSize);

      price = calcFee(orderVolume);
      addTo("Cash", -price);
      addTo("TotalCommission", price);

      calcGain();

      order.replace(3, QString::number(executedPrice,'f',2));
      order.replace(4, "executed");
      order.prepend(dateString);
      order.append("pGain=" + QString::number(percentGain, 'f', 2));
      mReport.append(order);
      return;
    }

  }
  else // Type = "Short"
  {
    error(FUNC, "FIXME: Short orders not implemented.");
  }

  bool isNumber;
  int validity = order.at(4).toInt(&isNumber);

  if(isNumber) order.replace(4, QString::number(--validity));
  else validity = 0;

  if(validity < 1)
  {
    addTo("OOLongSell", -1.0);
    addTo("OpenLongSell", -orderSize);

    order.replace(4, "expired");

//     QStringList r(order);
//     r.prepend(dateString);
//     mReport.append(r);
    order.prepend(dateString);
    mReport.append(order);
  }
}

void Trader::calcGain()
{
  setTo("TotalBalance", mVariable.value("Cash"));

  double temp1, temp2;
  if(mVariable.value("Long") > 0.0)
  {
    mData->getValue(mSettings.value("GainRefLong"), temp1);
    temp2 = 100 * (temp1 / mVariable.value("AvgLong") - 1);
    setTo("GainLong", temp2);
    addTo("TotalBalance", mVariable.value("Long") * temp1 );

    mData->getValue(mSettings.value("PeakRefLong"), temp1);
    temp2 = 100 * (temp1 - mVariable.value("AvgLong")) / mVariable.value("AvgLong");
    if(temp2 > mVariable.value("PeakGainLong")) setTo("PeakGainLong", temp2);
  }
  else
  {
    setTo("AvgLong", 0.0);
    setTo("RAvgLong", 0.0);
    setTo("GainLong", 0.0);
    setTo("PeakGainLong", 0.0);
    setTo("EntryBalanceLong", 0.0);
  }

  if(mVariable.value("Short") > 0.0)
  {
    mData->getValue(mSettings.value("GainRefShort"), temp1);
    temp2 = -100 * (temp1 / mVariable.value("AvgShort") - 1);
    setTo("GainShort", temp2);
    addTo("TotalBalance", (mVariable.value("Short") * temp1) );
  }
  else
  {
    setTo("AvgShort", 0.0);
    setTo("GainShort", 0.0);
  }

  setTo("Gain", (mVariable.value("GainLong")));// + mVariable.value("GainShort")) / 2);

  if( (mVariable.value("Long") == 0) and (mVariable.value("Short") == 0.0) ) setTo("OffMarket", 1.0);
  else setTo("OffMarket", 0.0);

}

void Trader::setTo(const QString& name, double v)
{
  mVariable.insert(name, v);
  //mData->setValue(name, v);  // In case of mData don't know name, nothing happens
}

void Trader::addTo(const QString& name, double v)
{
  double newValue = mVariable.value(name) + v;
  mVariable.insert(name, newValue);
  //mData->setValue(name, newValue);  // In case of mData don't know name, nothing happens
}

void Trader::appendToMData(const QString& name)
{
  mData->append(name);
  mData->setValidRange(name, mBarsNeeded - 1, mData->dataTupleSize() - mBarsNeeded);
  mDataAdded.append(name);
}

void Trader::appendReport(const QString& txt, const double v, const QString& vname, const int precision/* = 2*/)
{
  QStringList sl;
  sl.append(txt);
  sl.append(QString::number(v,'f', precision));
  sl.append(vname);

  mReport.append(sl);
}
