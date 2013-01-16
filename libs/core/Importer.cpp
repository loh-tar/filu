//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
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

#include <QFile>
#include <QSqlQuery>

#include "Importer.h"

#include "FTool.h"
#include "FiTuple.h"
#include "FiluU.h"
#include "RcFile.h"
#include "SymbolTuple.h"
#include "SymbolTypeTuple.h"
#include "Validator.h"

using namespace std;

int dataPerSecond(int data, int time)
{
  // time comes in milliseconds

  if(!time) return 99 * data; // Don't div by zero, and report not too much ;-)

  return (1000 * data) / time;
}

Importer::Importer(FClass* parent)
        : FClass(parent, FUNC)
        , mSymbol(0)
        , mConsole(stderr)
        , mValid(new Validator(this))
{
  reset();
}

Importer::~Importer()
{
  if(!mPendingData.isEmpty() and !hasError())
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
    else if(mToDo.contains("compListPending"))
    {
      mToDo.insert("commitCompList");
      addCompList();
    }

    mPendingData.clear();
  }

  if(mDataW)
  {
    if(mDataLineNo.size()) mDataLineNo.dequeue();
    if(!hasError()) printStatus(eEffectOk);
  }

  if(mSymbol) delete mSymbol;

  delete mValid;

  verbose(FUNC, "I'm dead.", eMax);
}

void Importer::printStatus(Effect effectEnum/* = eEffectPending*/, const QString& extraTxt/* = ""*/)
{
  static const QStringList effectList = QStringList() << "" << "Ok" << "FAULT";
  static const char c[] = {'-', '\\', '|', '/'};
  static int cIdx = 0;
  static int maxMsgLength = 0;
  static int dataWidth = 3;
  static QString exTxt = "";

  if(verboseLevel() == eNoVerbose) return;

  if(mDataW and eEffectFault == effectEnum) printStatus(eEffectOk);

  bool force = false;
  QString effect = effectList.at(effectEnum);

  if(extraTxt.size())
  {
    if(mHint.size()) exTxt = mHint.join(extraTxt);
    else exTxt = extraTxt;

    force = true;
  }
  else if(effect.size())
  {
    force = true;
  }

  if(!force and mRolex.elapsed() < 200) return;

  if(effect.isEmpty())
  {
    if(4 == ++cIdx) cIdx = 0;
    effect = c[cIdx];
  }

  int lineNo = mLineNo;
  if(eEffectPending != effectEnum)
  {
    if(mDataLineNo.size()) lineNo = mDataLineNo.dequeue();
  }
  if(eEffectFault == effectEnum) // We have again to "chop"
  {
    if(mDataLineNo.size()) lineNo = mDataLineNo.dequeue();
  }

  QString byteTxt = QString("%1B").arg(mByteCount);
  double bytes = mByteCount;
  if(bytes > 999.0)
  {
    bytes /= 1000.0;
    byteTxt = QString("%1%2").arg(bytes, 3, 'f', 1).arg("KB");
  }
  if(bytes > 999.0)
  {
    bytes /= 1000.0;
    byteTxt = QString("%1%2").arg(bytes, 3, 'f', 2).arg("MB"); // One decimal more
  }

  if(mDataR > 999999) dataWidth = qMax(7, dataWidth);
  else if(mDataR > 99999) dataWidth = qMax(6, dataWidth);
  else if(mDataR > 9999) dataWidth = qMax(5, dataWidth);
  else if(mDataR > 999) dataWidth = qMax(4, dataWidth);

  QString msg = QString("[Line: %1 %2][%3/%4] %5%6 %7")
                        .arg(lineNo, 4)
                        .arg(byteTxt, 8)
                        .arg(mDataR,  dataWidth)
                        .arg(mDataW, -dataWidth)
                        .arg(mImportData, -20, '.')
                        .arg(effect, 8, '.')
                        .arg(exTxt);

  if(msg.size() > maxMsgLength) maxMsgLength = msg.size();

  if(msg.size() < maxMsgLength)
  {
    // Make sure that the whole line is repainted
    msg.append(QString(maxMsgLength - msg.size(), ' '));
  }

  if(force and eEffectPending != effectEnum)
  {
    mConsole << msg << endl;
    maxMsgLength = 0;
  }
  else
  {
    mConsole << msg << '\r' << flush;
  }

  if(eEffectPending != effectEnum)
  {
    exTxt.clear();
    mDataR = 0;
    mDataW = 0;
  }

  mHint.clear();
  mRolex.start();
}

bool Importer::notAdded(const QString& what/* = ""*/)
{
  if(mFilu->lastResult() > Filu::eNoData)
  {
    ++mDataW;
    mHint.clear();
    return false;
  }

  if(mFilu->hasFatal())
  {
    addErrors(mFilu->errors());
    printStatus(eEffectFault, "*** FATAL ***");
    return true;
  }

  QString msg;
//   if(mFilu->hasMessage()) msg = formatMessages(mFilu->errors().at(0), "%x");
  msg = mFilu->formatMessages("%x");

  if(what.size()) msg.append(QString(" (%1)").arg(what));
  else if(mHint.size()) msg.append(QString(" (%1)").arg(mHint.join(" ")));

  mHint.clear();
  printStatus(eEffectFault, msg);
  return true;
}

bool Importer::notFound(const QString& what/* = ""*/)
{
  if(mFilu->lastResult() > Filu::eNoData)
  {
    mHint.clear();
    return false;
  }

  if(mFilu->hasFatal())
  {
    addErrors(mFilu->errors());
    printStatus(eEffectFault, "*** FATAL ***");
    return true;
  }

  if(what.isEmpty() and mHint.isEmpty()) return true;

  QString msg = formatMessage(mFilu->errors().at(0), "%x");

  if(what.size() and what != "PrintError") msg.append(QString(" (%1)").arg(what));
  else if(mHint.size()) msg.append(QString(" (%1)").arg(mHint.join(" ")));

  mHint.clear();
  printStatus(eEffectFault, msg);
  return true;
}

void Importer::reset()
{
  mFilu->setNoErrorLogging(); // FIXME It bothers me
  clearErrors();
  mPendingData.clear();
  mHeader.clear();
  mHeaderExpanded.clear();
  mData.clear();
  mKnownSymbolTypes.clear();
  mKnownSTisProvider.clear();
  mAllSymbolTypes.clear();
  mToDo.clear();
  mTotalSymbolCount = 0;

  mLineNo    = 0;
  mByteCount = 0;
  mDataR     = 0;
  mDataW     = 0;
  mImportData.clear();

  if(mSymbol)
  {
    delete mSymbol;
    mSymbol = 0;
  }

  // Read all symbol types out of the DB
  SymbolTypeTuple* symbolTypes = mFilu->getSymbolTypes(Filu::eAllTypes);

  if(!check4FiluError(FUNC))
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

  mLineNo    += 1;
  mByteCount += line.size();

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
  //qDebug() << "Importer::import: " << mLineNo << mDataR << data;
  //qDebug() << "Importer::import: " << mLineNo << mDataR << row;

  // Check if line is a tag line
  if(row.at(0).startsWith("[")) return handleTag(row);

  // No, its not, therefore it must be a data line.
  mDataLineNo.enqueue(mLineNo);
  if(mDataLineNo.size() > 2) mDataLineNo.dequeue(); // Store max two line numbers

  ++mDataR;

  if(!mHeader.size())
  {
    static const QString dataBefore = tr("Data before header");

    if(verboseLevel(eAmple))
    {
      mImportData = dataBefore;
      printStatus(eEffectFault, data.left(30) + "...");
    }
    else
    {
      mImportData = tr("*** Warning ***");
      ++mDataW;
      printStatus(eEffectPending, dataBefore);
    }

    return true;
  }

  // Last check if all looks good
  if(mHeader.size() > row.size())
  {
    printStatus(eEffectFault, tr("To less data in the line."));
    return true;
  }

  // We place all in a QHash<QString, QString>
  for(int i = 0; i < mHeader.size(); ++i)
  {
    mData.insert(mHeader.at(i), row.at(i));
  }

  if(mToDo.contains("prepare")) prepare();
  else if(verboseLevel(eMax))
  {
    verbose(FUNC, QString("*** Data line No: %1 ***").arg(mLineNo));
    QStringList sl;
    QHashIterator<QString, QString> i(mData);
    while(i.hasNext()) { i.next(); sl << QString("%1=%2").arg(i.key(), i.value()); }
    sl.sort();
    foreach(QString data, sl) verbose(FUNC, data);
  }
  //return true; // debug only

  printStatus();

  setQualityId(); // Nearly always needed

  if(mToDo.contains("setSymbolTuple")) setSymbolTuple();

  if(mToDo.contains("addFiType"))     addFiType();
  if(mToDo.contains("addSymbolType")) addSymbolType();
  if(mToDo.contains("addMarket"))     addMarket();
  if(mToDo.contains("addFi"))         addFi();
  if(mToDo.contains("addSymbol"))     addSymbol();
  if(mToDo.contains("addUnderlying")) addUnderlying();
  if(mToDo.contains("addCompList"))   addCompList();
  if(mToDo.contains("addEODBar"))     addEODBar();
  if(mToDo.contains("addSplit"))      addSplit();
  if(mToDo.contains("addBroker"))     addBroker();

  if(mToDo.contains("addCO"))         addCO();
  if(mToDo.contains("addGroup"))      addGroup();
  if(mToDo.contains("addDepot"))      addDepot();
  if(mToDo.contains("addDepotPos"))   addDepotPos();
  if(mToDo.contains("addAccountPos")) addAccountPos();
  if(mToDo.contains("addOrder"))      addOrder();

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
  if(hasError())
  {
    mImportData = "Please report that bug";
    printStatus(eEffectFault);
    return false;
  }

  if(!mPendingData.isEmpty())
  {
    if(mDataLineNo.size()) mDataLineNo.dequeue();

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
    else if(mToDo.contains("compListPending"))
    {
      mToDo.insert("commitCompList");
      addCompList();
    }

    mPendingData.clear();
  }

  if(row.at(0).startsWith("[EOF]"))
  {
    if(mDataW)
    {
      if(mDataLineNo.size()) mDataLineNo.dequeue();
      printStatus(eEffectOk);
    }

    --mLineNo; // One less because it is not part of the file (or should be)
    mImportData = "End Of File";
    printStatus(eEffectOk);
    return false;
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

  mToDo.insert("prepare");

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

  if(row.at(0).startsWith("[CompListEnd]"))
  {
    mData.remove("CompList");
    return true;
  }

  if(row.at(0).startsWith("[Stop]"))
  {
    printStatus(eEffectOk, tr("Read Special Tag %1").arg("[Stop]"));
    return false;
  }

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
  verbose(FUNC, QString("*** Let's get ready to rumble! Line No: %1 ***").arg(mLineNo), eMax);

  printStatus();

  if(mSymbol) { delete mSymbol; mSymbol = 0; }
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
    verbose(FUNC, QString("Examine Symbols: %1 %2 %3").arg(i.key(), i.value(), rawKey), eMax);

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

  if(verboseLevel(eMax))
  {
    verbose(FUNC, QString("Symbols total used: %1").arg(mTotalSymbolCount));
    verbose(FUNC, QString("UsedKnownSymbols: %1").arg(mUsedKnownSymbols.size()));
    verbose(FUNC, QString("UsedRefSymbols: %1").arg(mUsedRefSymbols.size()));
    verbose(FUNC, QString("SymbolXCount: %1").arg(mSymbolXCount));
  }

  QStringList importData;

  if(mData.contains("MarketSymbol") and mData.contains("CurrencySymbol"))
  {
    importData << "Markets";
    mToDo.insert("addMarket");
  }

  if(mData.contains("FiType"))
  {
    importData << "FiTypes";
    mToDo.insert("addFiType");
  }

  if(mData.contains("SymbolType"))
  {
    importData << "SymbolTypes";
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
        importData << "FIs/Symbols";
        mToDo.insert("addFi");
        // One symbol is installed with the FI, but if we have more symbols
        // we have to add them separately
        if(toBeInstalled > 1) mToDo.insert("addSymbol");
      }
      else if(mTotalSymbolCount > 1)
      {
        mToDo.insert("addSymbol");
        importData << "Symbols";
      }
    }
  }

  if(mData.contains("Mother") and mData.contains("Weight"))
  {
    importData << "Underlyings";
    mToDo.insert("addUnderlying");
  }

  if(mData.contains("CompList") and mData.contains("Weight"))
  {
    importData << "CompList";
    mToDo.insert("addCompList");
  }

  if( mData.contains("Market0") and
      mData.contains("Date")    and
      mData.contains("Close")   and
     (mTotalSymbolCount > 0)       )
  {
    importData << "EODBarData";
    mToDo.insert("addEODBar");
  }

  if(mData.contains("SplitDate"))
  {
    importData << "Splits";
    mToDo.insert("addSplit");
  }

  if(mData.contains("CODate") and mData.contains("Plot"))
  {
    importData << "ChartObjects";
    mToDo.insert("addCO");
  }

  if(mData.contains("GroupPath") and mData.contains("RefSymbol0"))
  {
    importData << "Group";
    mToDo.insert("addGroup");
  }

  if(mData.contains("BrokerName") and mData.contains("FeeFormula") and mData.contains("CurrencySymbol"))
  {
    importData << "Broker";
    mToDo.insert("addBroker");
  }

  if(mData.contains("DepotName") or mData.contains("DepotId"))
  {
    if(mData.contains("Trader") and mData.contains("BrokerName"))
    {
      importData << "Depot";
      mToDo.insert("addDepot");
    }

    if(mData.contains("PDate"))
    {
      importData << "DepotPosition";
      mToDo.insert("addDepotPos");
    }

    if(mData.contains("ODate"))
    {
      importData << "DepotOrder";
      mToDo.insert("addOrder");
    }

    if(mData.contains("APDate"))
    {
      importData << "AccountData";
      mToDo.insert("addAccountPos");
    }
  }

  const QString impDat = importData.join(", ");
  if(mDataW and /*mImportData.size() and*/ impDat != mImportData)
  {
    mDataR -= 1;
    printStatus(eEffectOk);
    mDataR  = 1;
  }

  mImportData = impDat;

  printStatus();

  if(verboseLevel(eMax))
  {
    verbose(FUNC, QString("*** Data line No: %1 ***").arg(mLineNo));
    QStringList sl;
    QHashIterator<QString, QString> i(mData);
    while(i.hasNext()) { i.next(); sl << QString("%1=%2").arg(i.key(), i.value()); }
    sl.sort();
    foreach(QString data, sl) verbose(FUNC, data);
  }
}

void Importer::setSymbolTuple()
{
  // ...and add them to a fresh mSymbol tuple
  if(!mSymbol) mSymbol = new SymbolTuple(mTotalSymbolCount);

  mSymbol->rewind();

  // First, reference symbols...
  for(int i = 0; i < mUsedRefSymbols.size(); ++i)
  {
    mSymbol->next();
    // Don't set market & owner, so the symbol will not added
    // but used for searching for the FI
    mSymbol->set(mData.value(mUsedRefSymbols.at(i)), "", "");
    //qDebug() << "Importer::setSymbolTuple 1:" << mSymbol->caption() << mSymbol->market() << mSymbol->owner();
  }

  // Second, none provider symbols which has to be installed
  for(int i = 0; i < mUsedKnownSymbols.size(); ++i)
  {
    mSymbol->next();
    if(mKnownSTisProvider.value(mUsedKnownSymbols.at(i)))
    {
      // We cant install a provider symbol without a market
      mSymbol->set(mData.value(mUsedKnownSymbols.at(i)), "", "");
    }
    else
    {
      mSymbol->set(mData.value(mUsedKnownSymbols.at(i))
                  , "NoMarket"
                  , mUsedKnownSymbols.at(i));
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
    mSymbol->set(mData.value(s), mData.value(m), mData.value(p));
    //qDebug() << "Importer::setSymbolTuple 3:" << mSymbol->caption() << mSymbol->market() << mSymbol->owner();
  }

  if(verboseLevel(eMax))
  {
    QStringList sl;
    mSymbol->rewind();
    while(mSymbol->next())
    {
      sl << mSymbol->caption() << mSymbol->market() << mSymbol->owner();
      verbose(FUNC, sl.join(" "));
      sl.clear();
    }
  }
}

bool Importer::setFiIdByAnySymbol(const QString faultTxt/* = ""*/)
{
  QStringList tested;

  mSymbol->rewind();
  while(mSymbol->next())
  {
    if(mSymbol->caption().isEmpty()) continue;
    tested << mSymbol->caption();
    if(setFiIdBySymbol(mSymbol->caption())) return true;
  }

  if(faultTxt.size())
  {
    printStatus(eEffectFault, QString("%1 (Tested: %2)").arg(faultTxt, tested.join(" ")));
  }

  return false;
}

bool Importer::setFiIdBySymbol(const QString& symbol, const QString faultTxt/* = ""*/)
{
  // Here will Filu called to set the FiId and the SqlParm :symbol
  if(symbol == mData.value("_LastRefSymbol")) return true;  // We are up to date

  mFilu->getFiIdBySymbol(symbol);
  if(notFound())
  {
    mId.insert("Fi", Filu::eNoData);
    mData.remove("_LastRefSymbol");
    mData.remove("_RefSymbolHint");
    if(faultTxt.size())
    {
      if("FaultIfNot" == faultTxt) printStatus(eEffectFault, tr("Symbol not found: %2").arg(symbol));
      else printStatus(eEffectFault, QString("%1: %2").arg(faultTxt, symbol));
    }
    return false;
  }

  mId.insert("Fi", mFilu->lastResult());
  mData.insert("_LastRefSymbol", symbol);
  mData.insert("_RefSymbolHint", QString("(Ref: %1)").arg(symbol));
  return true;
}

bool Importer::setMarketId(const QString& market, const QString faultTxt/* = ""*/)
{
  // Here will Filu called to set the marketId and the SqlParm :market
  if(market == mData.value("_LastMarket")) return true;  // We are up to date

  mFilu->getMarketId(market);
  if(notFound())
  {
    mId.insert("Market", Filu::eNoData);
    if(faultTxt.size())
    {
      if("FaultIfNot" == faultTxt) printStatus(eEffectFault, tr("Market not found: %2").arg(market));
      else printStatus(eEffectFault, QString("%1: %2").arg(faultTxt, market));
    }
    return false;
  }

  mId.insert("Market", mFilu->lastResult());
  mData.insert("_LastMarket", market);
  return true;
}

bool Importer::setCurrencyId(const QString& curr)
{
  if(curr == mData.value("_LastCurrency")) return true;  // We are up to date

  mFilu->searchCaption("symbol", curr);
  if(notFound())
  {
    mId.insert("Currency", Filu::eNoData);
    return false;
  }

  mId.insert("Currency", mFilu->lastResult());
  mData.insert("_LastCurrency", curr);
  return true;
}

bool Importer::setDepotId(const QString faultTxt/* = ""*/)
{
  bool ok;
  int depotId = mData.value("DepotId").toInt(&ok);
  if(ok)
  {
    mId.insert("Depot", depotId);
    return true;
  }

  // Should be ugly enough to be unique
  QString lastDepot = QString("__%1_%2__").arg(mData.value("DepotName")
                                             , mData.value("DepotOwner"));

  if(lastDepot == mData.value("_LastDepot")) return true;    // We are up to date

  mFilu->setSqlParm(":name", mData.value("DepotName"));
  mFilu->setSqlParm(":owner", mData.value("DepotOwner"));
  QSqlQuery* query = mFilu->execSql("GetDepotId");
  mFilu->result(FUNC, query);
  if(notFound())
  {
    mId.insert("Depot", Filu::eNoData);
    if(faultTxt.size())
    {
      QString txt = tr("Depot not found: %1, %2").arg(mData.value("DepotName"), mData.value("DepotOwner"));
      printStatus(eEffectFault, txt);
    }
    return false;
  }

  mId.insert("Depot", mFilu->lastResult());
  mData.insert("_LastDepot", lastDepot);
  return true;
}

bool Importer::setQualityId()
{
  QString quality = mValid->sQuality(mData.value("Quality", "Gold")); // If no quality set, use Gold

  if(quality == mData.value("_LastQuality")) return true;  // We are up to date

  mData.insert("_LastQuality", quality); // Save now, anyway if good or not

  int q = mValid->iQuality(quality);

  if(q > Filu::eError) // Looks good?
  {
    mId.insert("Quality", q);
    return true;
  }

  warning(FUNC, tr("Quality '%1' is unknown, I will use Gold.").arg(quality));

  mId.insert("Quality", Filu::eGold);
  return false;
}

void Importer::addFiType()
{
  mFilu->addFiType(mData.value("FiType"));

  if(notAdded(mData.value("FiType"))) return;

  if(verboseLevel(eAmple))
  {
    printStatus(eEffectOk, mData.value("FiType"));
  }
}

void Importer::addSymbolType()
{
  bool isProvider = QVariant(mData.value("IsProvider")).toBool();

  mFilu->addSymbolType( mData.value("SymbolType")
                      , mData.value("SEQ").toInt()
                      , isProvider);

  if(notAdded(mData.value("SymbolType"))) return;

  mKnownSymbolTypes.append(mData.value("SymbolType"));

  if(verboseLevel(eAmple))
  {
    mHint << mData.value("SymbolType") << QString("(SEQ: %1)").arg(mData.value("SEQ"));
    printStatus(eEffectOk, " ");
  }
}

void Importer::addMarket()
{
  mFilu->addMarket( mData.value("MarketSymbol")
                  , mData.value("Currency")
                  , mData.value("CurrencySymbol"));

  mHint << mData.value("MarketSymbol") << mData.value("Currency") << mData.value("CurrencySymbol");
  if(notAdded()) return;

  if(verboseLevel(eAmple))
  {
    mHint << mData.value("MarketSymbol") << QString("(%1, %2)").arg(mData.value("Currency"), mData.value("CurrencySymbol"));;
    printStatus(eEffectOk, ", ");
  }
}

void Importer::addFi()
{
  if(setFiIdByAnySymbol())
  {
    mFilu->addFi(makeNameNice(mData.value("Name")), mData.value("Type"), mId.value("Fi"));

    mHint << mData.value("_RefSymbolHint") << mData.value("Name") << mSymbol->caption();
    if(notAdded()) return;

    if(verboseLevel(eAmple))
    {
      mHint << "Updated:" << mData.value("_RefSymbolHint") << mData.value("Name") << mData.value("Type");
      printStatus(eEffectOk, " ");
    }
    return;
  }

  mSymbol->rewind();
  while(mSymbol->next())
  {
    // Skip reference and garbage Symbols
    if(mSymbol->caption().isEmpty()) continue;
    if(mSymbol->owner().isEmpty()) continue;
    if(mSymbol->market().isEmpty()) continue;

    // FIXME: Set Delete date, of the FI
    mFilu->addFi(makeNameNice(mData.value("Name")), mData.value("Type")
               , mSymbol->caption(), mSymbol->market(), mSymbol->owner());

    mHint << mData.value("Name") << mSymbol->caption();
    if(notAdded()) continue;

    if(verboseLevel(eAmple))
    {
      mHint << "Added:" << mSymbol->caption() << mData.value("Name") << mData.value("Type");
      printStatus(eEffectOk, " ");
    }
    break; // Success
  }
}

void Importer::addSymbol()
{
  if(mSymbol->count() == 1)
  {
    mSymbol->rewind(0);
    QString msg = tr("Can't install a single symbol: %1").arg(mSymbol->caption());
    printStatus(eEffectFault, msg);
    return;
  }

  if(!setFiIdByAnySymbol("No RefSymbol found")) return;

  mSymbol->rewind();
  while(mSymbol->next())
  {
    // Skip reference and garbage symbols
    if(mSymbol->caption().isEmpty()) continue;
    if(mSymbol->owner().isEmpty()) continue;
    if(mSymbol->market().isEmpty()) continue;

    mFilu->addSymbol(mSymbol->caption(), mSymbol->market()
                   , mSymbol->owner(), mId.value("Fi"));

    mHint << mData.value("_RefSymbolHint") << mSymbol->owner() << mSymbol->caption() << mSymbol->market();
    if(notAdded()) continue;

    if(verboseLevel(eAmple))
    {
      mHint << mData.value("_RefSymbolHint") << mSymbol->owner() << mSymbol->caption() << mSymbol->market();
      printStatus(eEffectOk, " ");
    }
  }
}

void Importer::addUnderlying()
{
  QString errTxt = tr("Mother not found");
  if(!setFiIdBySymbol(mData.value("Mother"), errTxt)) return;

  errTxt = tr("%1 Underlying not found").arg(mData.value("Mother"));
  if(!setFiIdByAnySymbol(errTxt)) return;

  mFilu->addUnderlying(mData.value("Mother"), mData.value("_LastRefSymbol")
                      , mData.value("Weight").toDouble());

  mHint << mData.value("Mother") << mData.value("_LastRefSymbol");
  if(notAdded()) return;

  if(verboseLevel(eAmple))
  {
    mHint << mData.value("Mother") << mData.value("_LastRefSymbol");
    printStatus(eEffectOk, " ");
  }
}

void Importer::addCompList()
{
  if(mToDo.contains("commitCompList"))
  {
    mImportData.remove(", CompList");
    if(mDataW) printStatus(eEffectOk);

    mImportData = "CompList";
    mDataR = mPendingData.size();
    printStatus();

    mToDo.remove("commitCompList");
    mToDo.remove("compListPending");

    QString mother = mData.value("_CompList");
    mData.remove("_CompList");
    mData.remove("CompList");

    FiTuple* mom = mFilu->getFiBySymbol(mother);
    if(notFound(QString("Mother: %1").arg(mother))) return;

    QString momName = mom->name();
    int momId = mom->id();
    delete mom;

    momName.remove(QRegExp("\\D"));
    int compCount = momName.toInt();
    if(compCount and (compCount != mPendingData.size()))
    {
      printStatus(eEffectFault, tr("Expect %1 underlyings but got %2.")
                                  .arg(compCount).arg(mPendingData.size()));
      return;
    }

    mFilu->setSqlParm(":motherId", momId);
    QSqlQuery* ulys = mFilu->execSql("GetUnderlyings");
    if(!ulys)
    {
      check4FiluError(FUNC);
      return;
    }

    QSet<int> toBeDeleted; // Collect the underlyingId (thats *not* a fiId, its the undl.table primary key)
    while(ulys->next()) toBeDeleted << ulys->value(0).toInt();

    QString txt = QString("Commit %1 components to %2...")
                         .arg(mPendingData.size()).arg(mother);
    printStatus(eEffectPending, txt);

    QTime time;
    time.restart();

    for(int i = 0; i < mPendingData.size(); ++i)
    {
      mFilu->addUnderlying(mother
                         , mPendingData.at(i).at(0)
                         , mPendingData.at(i).at(1).toDouble() );

      if(notAdded(mPendingData.at(i).at(0))) return;
      toBeDeleted.remove(mFilu->lastResult());
      printStatus();
    }

    if(toBeDeleted.size())
    {
      foreach(int id, toBeDeleted)
      {
        mFilu->deleteRecord("underlying", id);
        if(check4FiluError(FUNC)) return;
      }

      int rate = dataPerSecond(mDataW, time.elapsed());
      txt = QString("%1, Components:%2, Changed:%3, %4FIs/s")
                   .arg(mother).arg(mDataW).arg(toBeDeleted.size()).arg(rate);
    }
    else
    {
      int rate = dataPerSecond(mDataW, time.elapsed());
      txt = QString("%1, %2 checked, No changes, %3FIs/s")
                   .arg(mother).arg(mDataW).arg(rate);
    }

    printStatus(eEffectOk, txt);
    return;
  }

  // Save to detect if mother was changing
  QString mother = mData.value("CompList");

  if(!mData.contains("_CompList"))
  {
    // At first call, set them
    mData.insert("_CompList", mother);
    mToDo.insert("compListPending");
  }

  if(mData.value("_CompList") != mother)
  {
    mToDo.insert("commitCompList");
    addCompList(); // Call myself to make the commit
    mPendingData.clear();
    mData.insert("_CompList", mother);
    mToDo.insert("compListPending");
  }

  // Check if FI/Symbol is known
  if(!setFiIdByAnySymbol("FI not found")) return;

  QStringList data;
  data << mData.value("_LastRefSymbol");
  data << mData.value("Weight");

  mPendingData << data;
}

void Importer::addEODBar()
{
  if(mToDo.contains("commitEODBars"))
  {
    mImportData.remove(", EODBarData");
    if(mDataW) printStatus(eEffectOk);

    mImportData = "EODBarData";
    mDataR = mPendingData.size();
    printStatus();

    mToDo.remove("commitEODBars");
    mToDo.remove("eodBarsPending");

    QString symbol = mData.value("_EODSymbol");
    QString market = mData.value("_EODMarket");
    mData.remove("_EODSymbol");
    mData.remove("_EODMarket");

    if(!setMarketId(market, "FaultIfNot")) return;
    if(!setFiIdBySymbol(symbol, "FaultIfNot")) return;

    QTime time;
    time.restart();
    QString txt = QString("Commit %1 at %2, %3 bars...").arg(symbol, market).arg(mDataR);
    mDataR = mPendingData.size();
    printStatus(eEffectPending, txt);

    QString header = "Date;Open;High;Low;Close;Volume;OpenInterest;Quality";
    QStringList barData;
    barData << header;
    // We like to add the data in chunks of 100 bars
    for(int i = 0; i < mPendingData.size(); ++i)
    {
      // Collect data...
      barData << mPendingData.at(i).join(";");
                                            // Continue ...
      if(     (i+1) % 100                   // ...if we have not 100 collected
          and i != mPendingData.size() - 1  // .. as long we have rest data
        ) continue;

      mDataW = i;
      mFilu->addEODBarData(mId.value("Fi"), mId.value("Market"), &barData);
      if(notAdded()) return;

      printStatus();
      barData.clear();
      barData << header;
    }

    int rate = dataPerSecond(mDataW, time.elapsed());
    txt = QString("%1 at %2, %3Bars/s").arg(symbol, market).arg(rate);

    printStatus(eEffectOk, txt);

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
    mToDo.insert("eodBarsPending");
  }

  if(mData.value("_EODSymbol") != symbol or
     mData.value("_EODMarket") != market)
  {
    mToDo.insert("commitEODBars");
    addEODBar(); // Call myself to make the commit
    mPendingData.clear();
    mData.insert("_EODSymbol", symbol);
    mData.insert("_EODMarket", market);
    mToDo.insert("eodBarsPending");
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
  data << QString::number(mId.value("Quality"));

  mPendingData << data;
}

void Importer::addSplit()
{
  QString comment;
  bool    ok = true;
  double  ratio;

  if(mData.contains("SplitPre:Post"))
  {
    comment = mData.value("SplitPre:Post");
    ratio   = mValid->dSplitPrePost(comment);
  }
  else if(mData.contains("SplitPost:Pre"))
  {
    comment = mData.value("SplitPost:Pre");
    ratio   = mValid->dSplitPostPre(comment);
  }
  else if(mData.contains("SplitRatio"))
  {
    bool ok;
    ratio = mData.value("SplitRatio").toDouble(&ok);
    if(!ok)
    {
      printStatus(eEffectFault, QString("SplitRatio not valid, skip: %1, %2")
                                    .arg(mData.value("SplitDate"), mSymbol->caption()));
      return;
    }
  }

  if(mValid->hasError())
  {
    printStatus(eEffectFault, QString("%1, skip: %2, %3")
                                .arg(mValid->formatMessages("%x")
                                   , mData.value("SplitDate"), mSymbol->caption()));
    return;
  }

  if(ratio < 1.0) comment.prepend("Split ");
  else comment.prepend("Reverse Split ");

  if(!setFiIdByAnySymbol("FI not found")) return;

  mFilu->addSplit(mData.value("_LastRefSymbol"), mData.value("SplitDate")
                , ratio, comment, mId.value("Quality"));

  if(notAdded(mData.value("_LastRefSymbol"))) return;

  if(verboseLevel(eAmple))
  {
    mHint << mData.value("_LastRefSymbol") << mData.value("SplitDate") << comment;
    printStatus(eEffectOk, ", ");
  }
}

void Importer::addBroker()
{
  mFilu->addBroker(mData.value("BrokerName"), mData.value("CurrencySymbol"), mData.value("FeeFormula"), mId.value("Quality"));

  if(notAdded(mData.value("BrokerName"))) return;

  if(verboseLevel(eAmple))
  {
    mHint << mData.value("BrokerName") << QString("Fee= %1").arg(mData.value("FeeFormula"));
    printStatus(eEffectOk, ", ");
  }
}

void Importer::addCO()
{
  if(!setFiIdBySymbol(mData.value("RefSymbol0"), "FaultIfNot"))return;
  if(!setMarketId(mData.value("Market0"), "FaultIfNot")) return;

  mFilu->setSqlParm(":id", 0); // Force to insert new CO
  mFilu->setSqlParm(":date", mData.value("CODate"));
  mFilu->setSqlParm(":plot", mData.value("Plot"));
  mFilu->setSqlParm(":type", mData.value("Type"));

  // We have to restore newline and semicolon
  // before do the insert
  mFilu->setSqlParm(":parameters", FTool::lineToTxt(mData.value("ArgV")));

  QSqlQuery* query = mFilu->execSql("PutCOs");
  //mFilu->result(FUNC, query); FIXME We have no insert_co function, so no return value
  //if(notAdded(mData.value("Type"))) return;
  if(check4FiluError(FUNC)) return;
  ++mDataW; // Needed because we could not use notAdded()

  if(verboseLevel(eAmple))
  {
    mHint << mData.value("_LastRefSymbol") << mData.value("Plot") << mData.value("Type");
    printStatus(eEffectOk, ", ");
  }
}

void Importer::addGroup()
{
  if(mToDo.contains("commitGroup"))
  {
    mImportData.remove(", Group");
    if(mDataW) printStatus(eEffectOk);

    mImportData = "Group";
    mDataR = mPendingData.size();
    printStatus();

    mToDo.remove("commitGroup");
    mToDo.remove("groupPending");

    QString path = mData.value("_GroupPath");
    mData.remove("_GroupPath");
    mData.remove("GroupPath");

    int gId = mFilu->addGroup(path);

    mHint << tr("Can't create group '%1', skip").arg(path);
    if(notAdded()) return;

    QTime time;
    time.restart();

    QString txt = QString("Commit %1 FIs to %2...").arg(mPendingData.size()).arg(path);
    printStatus(eEffectPending, txt);
    mDataR = 0;
    mDataW = 0;
    int nf = 0;
    txt = verboseLevel(eAmple) ? "FaultIfNot" : ""; // Ignore error or not
    for(int i = 0; i < mPendingData.size(); ++i)
    {
      ++mDataR;
      QString symbol = mPendingData.at(i).at(0);
      if(!setFiIdBySymbol(symbol, txt))
      {
        ++nf;
        continue;
      }

      mFilu->addToGroup(gId, mId.value("Fi"));
      // FIXME Calling here "notAdded()" need a DB function "add_to_group".
      //       See Filu::execute() about INSERT sql
      ++mDataW;
    }

    if(nf)
    {
      int rate = dataPerSecond(mDataW + nf, time.elapsed());
      txt = QString("%1, Added: %2, Faults: %3, %4FIs/s")
                   .arg(path).arg(mDataW).arg(nf).arg(rate);
    }
    else
    {
      int rate = dataPerSecond(mDataW, time.elapsed());
      txt = QString("%1, %3FIs/s").arg(path).arg(rate);
    }

    printStatus(eEffectOk, txt);
    return;
  }

  // Save path to detect if group was changing
  QString path = mData.value("GroupPath");

  if(!mData.contains("_GroupPath"))
  {
    // At first call, set them
    mData.insert("_GroupPath", path);
    mToDo.insert("groupPending");
  }

  if(mData.value("_GroupPath") != path)
  {
    mToDo.insert("commitGroup");
    addGroup(); // Call myself to make the commit
    mPendingData.clear();
    mData.insert("_GroupPath", path);
    mToDo.insert("groupPending");
  }

  mPendingData << (QStringList() << mData.value("RefSymbol0"));
}

void Importer::addDepot()
{
  setDepotId();

  mFilu->addDepot(mData.value("DepotName"), mData.value("DepotOwner"), mData.value("Trader")
                , mData.value("BrokerName"), mId.value("Depot"));

  if(notAdded()) return;

  if(verboseLevel(eAmple))
  {
    mHint << mData.value("DepotOwner") << mData.value("DepotName");
    printStatus(eEffectOk, ", ");
  }
}

void Importer::addDepotPos()
{
  if(!setDepotId("FaultIfNot")) return;
  if(!setFiIdBySymbol(mData.value("RefSymbol0"), "FaultIfNot")) return;
  if(!setMarketId(mData.value("Market0"), "FaultIfNot")) return;

  mFilu->addDepotPos(mId.value("Depot")
                   , QDate::fromString(mData.value("PDate"), Qt::ISODate)
                   , mId.value("Fi")
                   , mData.value("Pieces").toInt()
                   , mData.value("Price").toDouble()
                   , mId.value("Market")
                   , mData.value("Note") );

  if(notAdded()) return;

  if(verboseLevel(eAmple))
  {
    mHint << mData.value("DepotName") << mData.value("PDate") << QString("%1x %2").arg(mData.value("Pieces"), mData.value("RefSymbol0"));
    printStatus(eEffectOk, ", ");
  }
}

void Importer::addAccountPos()
{
  if(!setDepotId("FaultIfNot")) return;
  int postType = mFilu->accPostingType(mData.value("APType"));
  if(notFound("PrintError")) return;

  mFilu->addAccPosting(mId.value("Depot")
                     , QDate::fromString(mData.value("APDate"), Qt::ISODate)
                     , postType
                     , mData.value("Text")
                     , mData.value("Value").toDouble() );

  if(notAdded()) return;

  if(verboseLevel(eAmple))
  {
    mHint << mData.value("DepotName") << mData.value("APDate") << mData.value("Text") << mData.value("Value");
    printStatus(eEffectOk, ", ");
  }
}

void Importer::addOrder()
{
  if(!setDepotId("FaultIfNot")) return;
  if(!setFiIdBySymbol(mData.value("RefSymbol0"), "FaultIfNot")) return;
  if(!setMarketId(mData.value("Market0"), "FaultIfNot")) return;

  int orderType = mFilu->orderType(mData.value("Type"));
  if(notFound("PrintError")) return;

  int orderStatus = mFilu->orderStatus(mData.value("Status"));
  if(notFound("PrintError")) return;

  mFilu->addOrder(mId.value("Depot")
                , QDate::fromString(mData.value("ODate"), Qt::ISODate)
                , QDate::fromString(mData.value("VDate"), Qt::ISODate)
                , mId.value("Fi")
                , mData.value("Pieces").toInt()
                , mData.value("Limit").toDouble()
                , orderType
                , mId.value("Market")
                , orderStatus
                , mData.value("Note") );

  if(notAdded()) return;

  if(verboseLevel(eAmple))
  {
    mHint << mData.value("DepotName") << mData.value("Status") << mData.value("Type")
          << QString("%1x %2").arg(mData.value("Pieces"), mData.value("RefSymbol0"))
          << mData.value("Limit");
    printStatus(eEffectOk, ", ");
  }
}
