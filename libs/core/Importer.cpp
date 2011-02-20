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

#include "Importer.h"

#include "FiTuple.h"
#include "SymbolTuple.h"
#include "FTool.h"

using namespace std;

Importer::Importer(FClass* parent)
        : FClass(parent, FUNC)
        , mSymbol(0)
        , mConsole(stdout)
{
  reset();
}

Importer::~Importer()
{
  if(!mPendingData.isEmpty())
  {
    if(mToDo.contains("eodBarsPending"))
    {
      mToDo.insert("commitEODBars");
      addEODBar();
    }
    else if(mToDo.contains("groupPending"))
    {
      mToDo.insert("commitGroup");
      addGroup();
    }
  }

  //qDebug() << "Importer::~: I''m dead";
  if(mSymbol) delete mSymbol;

  mConsole << endl;
}

void Importer::printDot()
{
  if(mRolex.elapsed() > 500)
  {
    mConsole << "." << flush;
    mRolex.start();
  }
}

void Importer::reset()
{
  clearErrors();
  mPendingData.clear();
  mHeader.clear();
  mHeaderExpanded.clear();
  mData.clear();
  mKnownSymbolTypes.clear();
  mKnownSTisProvider.clear();
  mAllSymbolTypes.clear();
  mToDo.clear();
  mPrepared = false;
  mTotalSymbolCount = 0;

  if(mSymbol)
  {
    delete mSymbol;
    mSymbol = 0;
  }

  // Read all symbol types out of the DB
  SymbolTypeTuple* symbolTypes = mFilu->getSymbolTypes(Filu::eAllTypes);

  if(!check4FiluError(FUNC, tr("ERROR while exec GetSymbolTypes.sql")))
  {
    if(symbolTypes)
    {
      while(symbolTypes->next())
      {
        mKnownSymbolTypes << symbolTypes->caption();

        mKnownSTisProvider.insert(symbolTypes->caption()
                                , symbolTypes->isProvider() );
      }

      delete symbolTypes;
    }
  }

  mMustBeUnique = "Provider Symbol Market RefSymbol";
  mAllSymbolTypes << "Symbol" << "RefSymbol" << mKnownSymbolTypes;

  mRcFile->saveGroup();
  mRcFile->sync();
  mMakeNameNice = mRcFile->getBL("MakeNameNice");
  mRcFile->restoreGroup();

  if(mMakeNameNice)
  {
    mNiceSearch.clear();
    mNiceReplace.clear();

    QFile file(mRcFile->getGlobalST("FiluHome") + "MakeNameNice.conf");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      const QString errorMsg1 = tr("Wrong \"=\" in line %1 of file 'MakeNameNice.conf'.");
      const QString errorMsg2 = tr("Invalid RegExp in line %1 of file 'MakeNameNice.conf', ErrTxt: %2.");

      QTextStream in(&file);
      int ln = 0; // Count lines
      while (!in.atEnd())
      {
        ++ln;
        QString line = in.readLine();
        if(line.startsWith("*")) continue; // Ignore remarks
        if(line.isEmpty()) continue;

        QStringList parts = line.split("\"=\"");

        if(parts.size() < 2)
        {
          error(FUNC, errorMsg1.arg(ln));
          continue;
        }

        parts[0].remove(0, 1);
        parts[1].chop(1);
        QRegExp regExp(parts.at(0));
        if(!regExp.isValid())
        {
          error(FUNC, errorMsg2.arg(ln).arg(regExp.errorString()));
          continue;
        }
        mNiceSearch  <<  parts.at(0);
        mNiceReplace <<  parts.at(1);
      }
    }
    else
    {
      warning(FUNC, tr("No file 'MakeNameNice.conf' found."));
    }
  }

  mRolex.start();
}

bool Importer::import(const QString& line)
{
  // line could looks like:
  // [Header]Name;Type;Provider;Symbol;Market;RefSymbol
  // Apple Computer;Stock;Yahoo;AAPL;NASDAQ;US0378331005

  QString data = line;

  data = data.replace(QRegExp("^[\"\']" ),  "");
  data = data.simplified();

  if(data.isEmpty()) return true;
  if(data.startsWith("*")) return true;

  QStringList row = data.split(";");

  for(int i = 0; i < row.size(); ++i)
  {
    row[i] = row.at(i).trimmed();
    row[i].replace(QRegExp("^[\"\']" ),  "");
    row[i].replace(QRegExp("[\"\']$" ),  "");
    row[i] = row.at(i).trimmed();
  }

  if(row.indexOf(QRegExp(".+")) == -1) return true; // No content in data, only ";"

  mOrigData = data;
  //qDebug() << "Importer::import: " << data;
  //qDebug() << "Importer::import: " << row;

  // Check if line is a tag line
  if(row.at(0).startsWith("["))
  {
    return handleTag(row);
  }

  // No, its not, therefore it must be a data line.
  // We place all in a QHash<QString, QString>
  if(mHeader.size() > row.size())
  {
    mConsole << "To less data in line: '" << line << "'";
    return true;
  }

  for(int i = 0; i < mHeader.size(); ++i)
  {
    mData.insert(mHeader.at(i), row.at(i));
  }

  if(!mPrepared) prepare();

  //return true; // debug only

  printDot();
  mToDo.insert("PrintNewLine");

  if(mToDo.contains("setSymbolTuple")) setSymbolTuple();

  if(mToDo.contains("addFiType"))     addFiType();
  if(mToDo.contains("addSymbolType")) addSymbolType();
  if(mToDo.contains("addMarket"))     addMarket();
  if(mToDo.contains("addFi"))         addFi();
  if(mToDo.contains("addSymbol"))     addSymbol();
  if(mToDo.contains("addUnderlying")) addUnderlying();
  if(mToDo.contains("addEODBar"))     addEODBar();
  if(mToDo.contains("addSplit"))      addSplit();
  if(mToDo.contains("addBroker"))     addBroker();

  if(mToDo.contains("addCO"))         addCO();
  if(mToDo.contains("addGroup"))      addGroup();
  if(mToDo.contains("addDepot"))      addDepot();
  if(mToDo.contains("addDepotPos"))   addDepotPos();
  if(mToDo.contains("addAccountPos")) addAccountPos();
  if(mToDo.contains("addOrder"))      addOrder();

  if(mSymbol)
  {
    delete mSymbol;
    mSymbol = 0;
  }

  return !hasError();
}

void Importer::getPreparedHeaderData(QStringList& header, QHash<QString, QString>& data)
{
  header = mHeaderExpanded;
  data   = mData;
}

QString Importer::makeNameNice(const QString& name)
{
  if(!mMakeNameNice) return name;

  QString niceName = name;
  makeNameNice(niceName);
  return niceName;
}

void Importer::makeNameNice(QString& name)
{
  if(!mMakeNameNice) return;

  name = name.simplified();
  for(int i = 0; i < mNiceSearch.size(); ++i)
  {
    QRegExp regExp(mNiceSearch.at(i));
    regExp.setCaseSensitivity(Qt::CaseInsensitive);
    name.replace(regExp, mNiceReplace.at(i));
  }

  if(name.size() < 5) return;        // Don't change names like BASF
  if(name != name.toUpper()) return; // Don't change names with lower case

  QStringList parts = name.split(QRegExp("\\b")); // Split at word boundary
  for(int i = 0; i < parts.size(); ++i)
  {
    if(parts.at(i).size() < 4) continue;
    parts[i] = parts.at(i).toLower();
    parts[i].replace(0, 1, parts.at(i).at(0).toUpper());
  }

  name = parts.join("");
}

bool Importer::handleTag(QStringList& row)
{
  if(!mPendingData.isEmpty())
  {
    if(mToDo.contains("eodBarsPending"))
    {
      mToDo.insert("commitEODBars");
      addEODBar();
    }
    else if(mToDo.contains("groupPending"))
    {
      mToDo.insert("commitGroup");
      addGroup();
    }
  }

  if(row.at(0).startsWith("[Reset]"))
  {
    reset();
    return true;
  }

  if(row.at(0).startsWith("[MakeNameNice]"))
  {
    row[0].remove("[MakeNameNice]");
    if(row.at(0).compare("On", Qt::CaseInsensitive) == 0) mMakeNameNice = true;
    else mMakeNameNice = false;

    return true;
  }

  mPrepared = false;

  if(row.at(0).startsWith("[Header]"))
  {
    // Delete old header keys, be on the save side
    for(int i = 0; i < mHeaderExpanded.size(); ++i)
    {
      mData.remove(mHeaderExpanded.at(i));
    }

    row[0].remove("[Header]");
    mHeader = row;
    mHeaderExpanded = row;
    for(int i = 0; i < mHeader.size(); ++i)
    {
      if(mHeader.at(i).isEmpty()) continue;

      if(mKnownSymbolTypes.contains(mHeader.at(i)))
      {
        // mHeader.at(i) e.g. "Yahoo" or "Reuters"
        QString symbol = makeUnique("Symbol");
        QString unique = symbol;
        unique.remove("Symbol"); // Now the number suffix
        mData.insert("Provider" + unique, mHeader.at(i));  // Set e.g. Provider0 = Yahoo
        mHeaderExpanded.append("Provider" + unique);
        //qDebug() << "Importer::handleTag: [Header]" << i  << mHeader.at(i) << symbol << mKnownSTisProvider.value(mHeader.at(i));
        if(!mKnownSTisProvider.value(mHeader.at(i)))
        {
          // mHeader.at(i) e.g. "Reuters"
          mData.insert("Market" + unique, "NoMarket");
          mHeaderExpanded.append("Market" + unique);
          //qDebug() << "Importer::handleTag: [Header]" << i << ("Market" + unique);
        }
        mHeader.replace(i, symbol);
        mHeaderExpanded.replace(i, symbol);
      }
      else
      {
        mHeader.replace(i, makeUnique(mHeader.at(i)));
        mHeaderExpanded.replace(i, mHeader.at(i));
        //qDebug() << "Importer::handleTag: [Header]" << i << mHeader.at(i);
      }
      mData.insert(mHeader.at(i), "");
    }

    return true;
  }

  if(row.at(0).startsWith("[CompList]"))
  {
    mData.insert("Mother", row[0].remove("[CompList]"));
    return true;
  }

  if(row.at(0).startsWith("[CompListEnd]"))
  {
    mData.remove("Mother");
    return true;
  }

  if(row.at(0).startsWith("[Stop]")) return false;

  // At this point we have a global key
  QString key, value;
  buildPair(key, value, row.at(0));

  mData.insert(key, value);
  //qDebug() << "Importer::handleTag: New global Key: " << key << value;

  return true;
}

QString Importer::makeUnique(const QString& key)
{
  if(!mMustBeUnique.contains(key)) return key;

  QString unique;
  for(int i = 0; true; ++i)
  {
    unique = key + QString::number(i);
    if(!mHeaderExpanded.contains(unique)) break;
  }

  return unique;
}

void Importer::buildPair(QString& key, QString& value, const QString& line)
{
  // Line looks like e.g. "[Provider]Yahoo" or "[Market1]Xetra"
  QStringList pair = line.split("]");
  pair[0].remove("[");
  // Don't call here: key = makeUnique(pair.at(0))
  // A global key can't be auto numbered.
  key = pair.at(0);

  if("Provider" == key)
  {
    if(!mData.contains("Market0"))    mData.insert("Market0", "");
    if(!mData.contains("Symbol0"))    mData.insert("Symbol0", "");
  }
  else if("Symbol" == key)
  {
    if(!mData.contains("Provider0"))  mData.insert("Provider0", "");
    if(!mData.contains("Market0"))    mData.insert("Market0", "");
  }
  else if("Market" == key)
  {
    if(!mData.contains("Provider0"))  mData.insert("Provider0", "");
    if(!mData.contains("Symbol0"))    mData.insert("Symbol0", "");
  }

  if(mKnownSymbolTypes.contains(key))
  {
    if(!mData.contains("Market0"))    mData.insert("Market0", "");
    if(!mData.contains("Symbol0"))    mData.insert("Symbol0", "");
  }

  if(mMustBeUnique.contains(key)) key.append("0");

  if(pair.size() > 1) value = pair.at(1);
  else value = "";
}

void Importer::prepare()
{
  const char* introTxt = "Import: ";
  QString tmpText;
  QTextStream text(&tmpText);
  //qDebug() << "Importer::prepare:";

  if(mToDo.contains("PrintNewLine"))
  {
    mConsole << endl;
    mToDo.remove("PrintNewLine");
  }

  text << introTxt;

  mPrepared = true;
  mTotalSymbolCount = 0;
  mSymbolXCount = 0;
  mUsedRefSymbols.clear();
  mUsedKnownSymbols.clear();
  mAllUsedSymbols.clear();
  mToDo.clear();

  // Examine the symbol types we have
  QString rawKey;
  QHashIterator<QString, QString> i(mData);
  while (i.hasNext())
  {
    i.next();
    rawKey = i.key();
    rawKey.remove(QRegExp("[\\d]*\\b"));
    //qDebug() << "Importer::prepare:" << i.key() << ":" << i.value() << rawKey;

    if(!mAllSymbolTypes.contains(rawKey)) continue;

    ++mTotalSymbolCount;
    mAllUsedSymbols.append(i.key());

    if(mKnownSymbolTypes.contains(rawKey))
    {
      mUsedKnownSymbols.append(i.key());
      continue;
    }

    if(rawKey == "RefSymbol")
    {
      mUsedRefSymbols.append(i.key());
      continue;
    }

    if(rawKey == "Symbol")
    {
      ++mSymbolXCount;
      continue;
    }

    fatal(FUNC, "Oops?! What's that? :" + rawKey);

  }
  //qDebug() << "Importer::prepare:" <<  mTotalSymbolCount << "symbols total used";
  //qDebug() << "Importer::prepare:" <<  mSymbolXCount << "SymbolX used";

  if(mData.contains("Market0") and mData.contains("CurrencySymbol"))
  {
    text << "Markets, ";
    mToDo.insert("addMarket");
  }

  if(mData.contains("FiType"))
  {
    text << "FiTypes, ";
    mToDo.insert("addFiType");
  }

  if(mData.contains("SymbolType"))
  {
    text << "SymbolTypes, ";
    mToDo.insert("addSymbolType");
  }

  if(mTotalSymbolCount > 0)
  {
    mToDo.insert("setSymbolTuple");

    int toBeInstalled = mSymbolXCount + mUsedKnownSymbols.size();

    // Check if we have FIs or symbols to add (no symbols to add, no FI to add)
    if(toBeInstalled)
    {
      if(mData.contains("Name") and mData.contains("Type") )
      {
        text << "FIs/Symbols, ";
        mToDo.insert("addFi");
        // One symbol is installed with the FI, but if we have more symbols
        // we have to add them separately
        if(toBeInstalled > 1) mToDo.insert("addSymbol");
      }
      else
      {
        mToDo.insert("addSymbol");
        text << "Symbols, ";
      }
    }
  }

  if(mData.contains("Mother") and mData.contains("Weight"))
  {
    text << "Underlyings, ";
    mToDo.insert("ClearCompList");
    mToDo.insert("addUnderlying");
  }

  if( mData.contains("Market0") and
      mData.contains("Date")    and
      mData.contains("Close")   and
     (mTotalSymbolCount > 0)       )
  {
    text << "EODBarData, ";
    mToDo.insert("addEODBar");
  }

  if(mData.contains("SplitDate"))
  {
    text << "Splits, ";
    mToDo.insert("addSplit");
  }

  if(mData.contains("CODate") and mData.contains("Plot"))
  {
    text << "ChartObjects, ";
    mToDo.insert("addCO");
  }

  if(mData.contains("GroupPath") and mData.contains("RefSymbol0"))
  {
    text << "Groups, ";
    mToDo.insert("addGroup");
  }

  if(mData.contains("BrokerName") and mData.contains("FeeFormula"))
  {
    text << "Broker, ";
    mToDo.insert("addBroker");
  }

  if(mData.contains("DepotName"))
  {
    if(mData.contains("Trader"))
    {
      text << "Depot, ";
      mToDo.insert("addDepot");
    }

    if(mData.contains("PDate"))
    {
      text << "DepotPosition, ";
      mToDo.insert("addDepotPos");
    }

    if(mData.contains("ODate"))
    {
      text << "DepotOrder, ";
      mToDo.insert("addOrder");
    }

    if(mData.contains("APDate"))
    {
      text << "AccountData, ";
      mToDo.insert("addAccountPos");
    }
  }

  if(tmpText > introTxt) mConsole << tmpText;

  //qDebug() << "Importer::prepare: mHeader" << mHeader;
  //qDebug() << "Importer::prepare: mData" << mData;
}

void Importer::setSymbolTuple()
{
  // ...and add them to a fresh mSymbol tuple
  mSymbol = new SymbolTuple(mTotalSymbolCount);

  // First, reference symbols...
  for(int i = 0; i < mUsedRefSymbols.size(); ++i)
  {
    mSymbol->next();
    mSymbol->setCaption(mData.value(mUsedRefSymbols.at(i)));
    // Don't set market & owner, so the symbol will not added
    // but used for searching for the FI
    mSymbol->setMarket("");
    mSymbol->setOwner("");
    //qDebug() << "Importer::setSymbolTuple 1:" << mSymbol->caption() << mSymbol->market() << mSymbol->owner();
  }

  // Second, none provider symbols which has to be installed
  for(int i = 0; i < mUsedKnownSymbols.size(); ++i)
  {
    mSymbol->next();
    mSymbol->setCaption(mData.value(mUsedKnownSymbols.at(i)));
    if(mKnownSTisProvider.value(mUsedKnownSymbols.at(i)))
    {
      // We cant install a provider symbol without a market
      mSymbol->setMarket("");
      mSymbol->setOwner("");
    }
    else
    {
      mSymbol->setMarket("NoMarket");
      mSymbol->setOwner(mUsedKnownSymbols.at(i));
    }
    //qDebug() << "Importer::setSymbolTuple 2:" << mSymbol->caption() << mSymbol->market() << mSymbol->owner();
  }

  // And last, provider symbols which has to be installed
  for(int i = 0; i < mSymbolXCount; ++i)
  {
    QString p = "Provider" + QString::number(i);
    QString s = "Symbol" + QString::number(i);
    QString m = "Market" + QString::number(i);
    mSymbol->next();
    mSymbol->setCaption(mData.value(s));
    mSymbol->setMarket(mData.value(m));
    mSymbol->setOwner(mData.value(p));
    //qDebug() << "Importer::setSymbolTuple 3:" << mSymbol->caption() << mSymbol->market() << mSymbol->owner();
  }

}

bool Importer::setFiIdBySymbol(const QString& symbol)
{
  // Here will Filu called to set the FiId and the SqlParm :symbol
  if(symbol == mData.value("_LastSymbol")) return true;  // We are up to date

  int retVal = mFilu->setSymbolCaption(symbol);

  if(retVal >= Filu::eData)
  {
    mId.insert("Fi", retVal);
    mData.insert("_LastSymbol", symbol);
    return true;
  }

  mId.insert("Fi", Filu::eNoData);
  return false;
}

bool Importer::setMarketId(const QString& market)
{
  // Here will Filu called to set the marketId and the SqlParm :market
  if(market == mData.value("_LastMarket")) return true;  // We are up to date

  int retVal = mFilu->setMarketName(market);

  if(retVal >= Filu::eData)
  {
    mId.insert("Market", retVal);
    mData.insert("_LastMarket", market);
    return true;
  }

  mId.insert("Market", Filu::eNoData);
  return false;
}

bool Importer::setCurrencyId(const QString& curr)
{
  if(curr == mData.value("_LastCurrency")) return true;  // We are up to date

  int retVal = mFilu->searchCaption("symbol", curr);

  if(retVal >= Filu::eData)
  {
    mId.insert("Currency", retVal);
    mData.insert("_LastCurrency", curr);
    return true;
  }

  mId.insert("Currency", Filu::eNoData);
  return false;
}

bool Importer::setDepotId(const QString& name, const QString& owner)
{
  QString lastDepot = QString("__%1_%2__").arg(name, owner); // Should be ugly enough to be unique

  if(lastDepot == mData.value("_LastDepot")) return true;    // We are up to date

  mFilu->setSqlParm(":name", name);
  mFilu->setSqlParm(":owner", owner);

  QSqlQuery* query = mFilu->execSql("GetDepotId");

  int retVal = mFilu->result(FUNC, query);

  if(retVal < Filu::eSuccess)
  {
//     addErrors(mFilu->errors());
    mId.insert("Depot", Filu::eNoData);
    return false;
  }

  mId.insert("Depot", retVal);
  mData.insert("_LastDepot", lastDepot);
  return true;
}

bool Importer::setQualityId()
{
  QString quality = mData.value("Quality", "Gold"); // If no quality set, use Gold

  if(quality == mData.value("_LastQuality")) return true;  // We are up to date

  mData.insert("_LastQuality", quality); // Save now, anyway if good or not

  int retVal = mFilu->quality(quality);

  if(retVal > Filu::eError) // Looks good?
  {
    mId.insert("Quality", retVal);
    return true;
  }

  warning(FUNC, tr("Quality '%1' is unknown, I will use Gold.").arg(quality));

  mId.insert("Quality", Filu::eGold);
  return false;
}

void Importer::addFiType()
{
  mFilu->addFiType(mData.value("FiType"));
}

void Importer::addSymbolType()
{
  bool isProvider = QVariant(mData.value("IsProvider")).toBool();

  mFilu->addSymbolType( mData.value("SymbolType")
                      , mData.value("SEQ").toInt()
                      , isProvider);

  mKnownSymbolTypes.append(mData.value("SymbolType"));
}

void Importer::addMarket()
{
  mFilu->addMarket( mData.value("Market0")
                  , mData.value("Currency")
                  , mData.value("CurrencySymbol"));
}

void Importer::addFi()
{
  mSymbol->rewind();
  int newFiId = Filu::eNoData;
  while(mSymbol->next())
  {
    // Skip reference and garbage Symbols
    if(mSymbol->caption().isEmpty()) continue;
    if(mSymbol->owner().isEmpty()) continue;
    if(mSymbol->market().isEmpty()) continue;

    // FIXME: Set Delete date, of the FI
    newFiId = mFilu->addFi(makeNameNice(mData.value("Name"))
                         , mData.value("Type")
                         , mSymbol->caption()
                         , mSymbol->market()
                         , mSymbol->owner());

    //qDebug() << "Importer::addFi: " << mData.value("Name") <<  mData.value("Type")
    //         << mSymbol->caption() << mSymbol->market() <<  mSymbol->owner() << newFiId;

    if(newFiId >= Filu::eData) return; // Success
  }

  if(newFiId < Filu::eData)
  {
//     if(check4FiluError(FUNC, tr("fail to add FI")))
//     {
//       mConsole << "fail to add FI " << mData.value("Name") <<  " "
//                << mData.value("Type") << endl;
//     }
    if(mFilu->hasError())
    {
      mConsole << endl << "FIXME:mFilu->errorText()";
    }
  }

  return;
}

void Importer::addSymbol()
{
  // Search the FiId fitting a Symbol
  int fiId = Filu::eNoData;
  mSymbol->rewind();
  while(mSymbol->next())
  {
    if(mSymbol->caption().isEmpty()) continue;
    fiId = mFilu->setSymbolCaption(mSymbol->caption());
    if(fiId >= Filu::eData) break;
  }

  if(fiId < Filu::eData)
  {
    QString msg = tr("Fail to add symbol. No fitting RefSymbol found: ");
    mSymbol->rewind();
    while(mSymbol->next())
    {
      if(mSymbol->caption().isEmpty()) continue;
      msg.append(mSymbol->caption() + ", ");
    }
    msg.chop(2);
    error(FUNC, msg);
    mConsole << endl << msg << endl;
    return;
  }

  mSymbol->rewind();
  while(mSymbol->next())
  {
    // Skip reference and garbage symbols
    if(mSymbol->caption().isEmpty()) continue;
    if(mSymbol->owner().isEmpty()) continue;
    if(mSymbol->market().isEmpty()) continue;
    //  FIXME: Use mFilu->addSymbol(mSymbol), and set Issue date, Maritury date
    /*int retVal = */mFilu->addSymbol(mSymbol->caption()
                                  , mSymbol->market()
                                  , mSymbol->owner()
                                  , fiId);

    //qDebug() << "Importer::addSymbol: retVal=" << retVal << mSymbol->caption() << mSymbol->market() << mSymbol->owner() << fiId;
    if(mFilu->hasError())
    {
      mConsole << endl << "FIXME:mFilu->errorText()" << endl;
    }
  }

  return;
}

void Importer::addUnderlying()
{
  if(mToDo.contains("ClearCompList"))
  {
    mToDo.remove("ClearCompList");

    int momId = mFilu->setSymbolCaption(mData.value("Mother"));
    if(momId < Filu::eData)
    {
      mToDo.remove("addUnderlying");
      QString msg = tr("Mother '%1' not found.").arg(mData.value("Mother"));
      error(FUNC, msg);
      mConsole << endl << msg << endl;
      return;
    }

    mFilu->setSqlParm(":momId", momId);
    mFilu->execSql("DelAllUnderlyingsFromMother");
    //qDebug() << "Importer::addUnderlying: Old stuff removed.";
  }

  mSymbol->rewind();
  bool ok(false);
  while(mSymbol->next())
  {
    if(mSymbol->caption().isEmpty()) continue;

    ok = mFilu->addUnderlying(mData.value("Mother")
                            , mSymbol->caption()
                            , mData.value("Weight").toDouble());

    if(ok) break;
  }

  if(!ok)
  {
    mConsole << endl
             << "Importer::addUnderlying: Fail to add underlying: [BEGIN MESSAGE]\n"
             << "FIXME:mFilu->errorText()" << endl
             << "[END MESSAGE]" << endl;
  }
}

void Importer::addEODBar()
{
  if(mToDo.contains("commitEODBars"))
  {
    mConsole << endl << "Importer::addEODBar: commit " << mData.value("_EODSymbol")
             << " " << mData.value("_EODMarket") << "..." << flush;

    int marketId = mFilu->setMarketName(mData.value("_EODMarket"));
    if(marketId < 1)
    {
      mConsole << "could not set Market'" << mData.value("_EODMarket") << "', nothing importet." << endl;
      mPendingData.clear();
      return;
    }

    int fiId = mFilu->setSymbolCaption(mData.value("_EODSymbol"));
    if(fiId < 1)
    {
      mConsole << "could not set Symbol'" << mData.value("_EODSymbol") << "', nothing importet." << endl;
      mPendingData.clear();
      return;
    }

    mToDo.remove("commitEODBars");
    mToDo.remove("eodBarsPending");

    mData.remove("_EODSymbol");
    mData.remove("_EODMarket");

    mFilu->addEODBarData(fiId, marketId, &mPendingData);
    mPendingData.clear();
    //mConsole << "done." << endl;
    return;
  }

  // Save market and symbol to detect if data owner was changing
  QString symbol = mData.value(mAllUsedSymbols.at(0));
  QString market = mData.value("Market0");

  if(!mData.contains("_EODSymbol"))
  {
    // At first call, set them
    mData.insert("_EODSymbol", symbol);
    mData.insert("_EODMarket", market);
  }

  if(mData.value("_EODSymbol") != symbol or
     mData.value("_EODMarket") != market)
  {
    mToDo.insert("commitEODBars");
    addEODBar(); // Call myself to make the commit

    mData.insert("_EODSymbol", symbol);
    mData.insert("_EODMarket", market);
  }

  if(mPendingData.isEmpty())
  {
    // Add the header needed by mFilu->addEODBarData()
    mPendingData << "Date;Open;High;Low;Close;Volume;OpenInterest;Quality";
  }

  QStringList data;
  data << mData.value("Date");

  // If no Open/High/Low use Close instead
  data << mData.value("Open", mData.value("Close"));
  data << mData.value("High", mData.value("Close"));
  data << mData.value("Low",  mData.value("Close"));

  data << mData.value("Close");
  data << mData.value("Volume", "0");
  data << mData.value("OpenInterest", "0");
  data << mData.value("Quality", "1");

  mPendingData << data.join(";");

  mToDo.insert("eodBarsPending");
}

void Importer::addSplit()
{
  // If no quality is set, use 1 as default
  // 1=gold, as final classified data by script
  // See doc/hacking-provider-scripts.txt
  int quality = mData.value("Quality", "1").toInt();

  QString comment;
  double  pre;
  double  post = 0.0;
  double  ratio;
  bool    ok = true;

  if(mData.contains("SplitPre:Post"))
  {
    comment = mData.value("SplitPre:Post");
    QStringList sl = comment.split(":");
    if(sl.size() < 2) ok = false;

    if(ok) pre  = sl[0].toDouble(&ok);
    if(ok) post = sl[1].toDouble(&ok);

  }
  else if(mData.contains("SplitPost:Pre"))
  {
    comment = mData.value("SplitPost:Pre");
    QStringList sl = comment.split(":");
    if(sl.size() < 2) ok = false;

    if(ok) pre  = sl[1].toDouble(&ok);
    if(ok) post = sl[0].toDouble(&ok);

  }
  else
  {
    ok = false;
  }

  if(post == 0) ok = false; //FIXME: add error message division 0

  if(ok)
  {
    if(pre < post) comment.prepend("Split ");
    else comment.prepend("Reverse Split ");

    ratio = pre / post;
  }
  else if(mData.contains("SplitRatio"))
  {
    ratio = mData.value("SplitRatio").toDouble(&ok);

    if(mData.contains("SplitComment")) comment = mData.value("SplitComment");
    else comment = "";
  }

  if(ok)
  {
    mSymbol->rewind();
    while(mSymbol->next())
    {
      int retVal = mFilu->addSplit(mSymbol->caption(), mData.value("SplitDate")
                                 , ratio, comment, quality);

      if(retVal >= Filu::eSuccess) break;
    }
  }
  else
  {
    mConsole << endl << "Importer::import: split data not valid, skiped:" << endl/* << data*/;
  }
}

void Importer::addBroker()
{
  // Hm, Filu has only a BrokerTuple add func, so we do it raw like
  mFilu->setSqlParm(":brokerId", 0);
  mFilu->setSqlParm(":name", mData.value("BrokerName"));
  mFilu->setSqlParm(":feeFormula", mData.value("FeeFormula"));
  mFilu->setSqlParm(":quality", mId.value("Quality"));

  QSqlQuery* query = mFilu->execSql("AddBroker");

  if(mFilu->result(FUNC, query) < Filu::eSuccess)//FIXME check4FiluError
  {
    addErrors(mFilu->errors());
  }
}

void Importer::addCO()
{
  mFilu->setSqlParm(":id", 0); // Force to insert new CO
  if(!setFiIdBySymbol(mData.value("RefSymbol0"))) return;
  if(!setMarketId(mData.value("Market0"))) return;
  mFilu->setSqlParm(":date", mData.value("CODate"));
  mFilu->setSqlParm(":plot", mData.value("Plot"));
  mFilu->setSqlParm(":type", mData.value("Type"));

  // We have to restore newline and semicolon
  // before do the insert
  mFilu->setSqlParm(":parameters", FTool::lineToTxt(mData.value("ArgV")));

  QSqlQuery* query = mFilu->execSql("PutCOs");

  if(!query)
  {
    if(check4FiluError(FUNC, tr("ERROR while exec PutCOs.sql"))) return;// false;

    mConsole << "no chart objects match settings." << endl;
    return /*true*/;
  }
}

void Importer::addGroup()
{
  if(mToDo.contains("commitGroup"))
  {
    mConsole << endl << "Importer::addGroup: Commit " << mData.value("GroupPath")
             << "..." << flush;

    mToDo.remove("commitGroup");
    mToDo.remove("groupPending");

    mFilu->setSqlParm(":groupPath", mData.value("_GroupPath"));
    QSqlQuery* query = mFilu->execSql("AddGroup");
    if(!query)
    {
      mPendingData.clear();
      if(check4FiluError(FUNC, tr("ERROR while exec AddGroup.sql")))
      {
        mConsole << "could not create group!?" << endl;
        return;// false;
      }
      return /*true*/;
    }
    query->next();
    int gId = query->value(0).toInt();

    for(int i = 0; i < mPendingData.size(); ++i)
    {
      int fiId = mFilu->setSymbolCaption(mPendingData.at(i));
      if(fiId < Filu::eData)
      {
        mConsole << "Symbol not found, skip " << mPendingData.at(i) << endl;
        continue;
      }

      printDot();
      mFilu->addToGroup(gId, fiId);
    }

    mData.remove("_GroupPath");

    mPendingData.clear();
    return;
  }

  // Save path to detect if group was changing
  QString path = mData.value("GroupPath");

  if(!mData.contains("_GroupPath"))
  {
    // At first call, set them
    mData.insert("_GroupPath", path);
  }

  if(mData.value("_GroupPath") != path)
  {
    mToDo.insert("commitGroup");
    addGroup(); // Call myself to make the commit

    mData.insert("_GroupPath", path);
  }

  mPendingData << mData.value("RefSymbol0");

  mToDo.insert("groupPending");
}

void Importer::addDepot()
{
  int retVal = mFilu->addDepot(mData.value("DepotName"), mData.value("DepotOwner"), mData.value("Trader")
                             , mData.value("CurrencySymbol"), mData.value("BrokerName"));
}

void Importer::addDepotPos()
{
  if(!setDepotId(mData.value("DepotName"), mData.value("DepotOwner"))) return;
  if(!setFiIdBySymbol(mData.value("RefSymbol0"))) return;
  if(!setMarketId(mData.value("Market0"))) return;

  int retVal = mFilu->addDepotPos(mId.value("Depot")
                                , QDate::fromString(mData.value("PDate"), Qt::ISODate)
                                , mId.value("Fi")
                                , mData.value("Pieces").toInt()
                                , mData.value("Price").toDouble()
                                , mId.value("Market")
                                , mData.value("Note") );
}

void Importer::addAccountPos()
{
  if(!setDepotId(mData.value("DepotName"), mData.value("DepotOwner"))) return;
  int postType = mFilu->accPostingType(mData.value("APType"));
  if(!postType)
  {
    // print error
    return;
  }

  int retVal = mFilu->addAccPosting(mId.value("Depot")
                                  , QDate::fromString(mData.value("APDate"), Qt::ISODate)
                                  , postType
                                  , mData.value("Text")
                                  , mData.value("Value").toDouble() );
}

void Importer::addOrder()
{
  if(!setDepotId(mData.value("DepotName"), mData.value("DepotOwner"))) return;
  if(!setFiIdBySymbol(mData.value("RefSymbol0"))) return;
  if(!setMarketId(mData.value("Market0"))) return;

  int orderType = mFilu->orderType(mData.value("Type"));
  if(orderType == eError)
  {
    // error
    return;
  }

  int orderStatus = mFilu->orderStatus(mData.value("Status"));
  if(orderStatus == eError)
  {
    // error
    return;
  }

  int retVal = mFilu->addOrder(mId.value("Depot")
                             , QDate::fromString(mData.value("ODate"), Qt::ISODate)
                             , QDate::fromString(mData.value("VDate"), Qt::ISODate)
                             , mId.value("Fi")
                             , mData.value("Pieces").toInt()
                             , mData.value("Limit").toDouble()
                             , orderType
                             , mId.value("Market")
                             , orderStatus
                             , mData.value("Note")
  );
}
