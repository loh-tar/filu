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

#include "Exporter.h"
#include "FTool.h"

using namespace std;

Exporter::Exporter(FClass* parent)
        : FClass(parent, FUNC)
        , mBuffer(&mBufferStr/*, QIODevice::WriteOnly*/)
        , mConsole(stderr)
        , mOutFile(0)
        , mFis(0)
{}

Exporter::~Exporter()
{
  mOutput.flush();

  if(mOutFile)
  {
    mOutFile->close();
    delete mOutFile;
  }
}

void Exporter::printStatus(Effect effectEnum, const QString& extraTxt)
{
  static const QStringList effectList = QStringList() << "" << "Ok" << "Note"<< "FAULT";
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
                        .arg(mDataText, -20, '.')
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

bool Exporter::noData(const QString& what/* = ""*/)
{
  if(mFilu->lastResult() > Filu::eNoData)
  {
    mDataR += mFilu->lastQuery()->size();
    mHint.clear();
    return false;
  }

  if(what.isEmpty()) return true;

  if(mDataW) printStatus(eEffectOk);

  QString txt;
  if(what == "Info") txt = tr("No data found to export.");
  else txt = what;

  mHint.clear();
  printStatus(eEffectNote, txt);
  return true;
}

void Exporter::writeToFile()
{
  QStringList lines = mBufferStr.split('\n');
  lines.removeLast();
  foreach(QString line, lines)
  {
    mByteCount += line.size();
    ++mLineNo;
    if(!line.startsWith('*') and !line.startsWith('[') and !line.isEmpty())
    {
      ++mDataW;
      mDataLineNo.enqueue(mLineNo);
      if(mDataLineNo.size() > 1) mDataLineNo.dequeue(); // Store max one line number(!?)
    }
  }

  mOutput << mBufferStr << flush;;
  mBufferStr.clear();
  printStatus();
}

bool Exporter::selectFis()
{
  if(mFis)
  {
    mFis->seek(-1); // Rewind
    if(mFis->size()) return true;

    return false;
  }

  mFis = mFilu->execSql("GetAllFi");

  if(check4FiluError(FUNC)) return false;

  if(noData("No FIs found.")) return false;

  return true;
}

bool Exporter::exxport(QStringList& command)
{
  mCmdLine = command;

  if(FTool::getParameter(mCmdLine, "--verbose", mParm) > 0) setVerboseLevel(FUNC, mParm.at(0));

  // Open output file, if needed
  if(FTool::getParameter(mCmdLine, "--toFile", mParm) > 0)
  {
    mOutFile = new QFile(mParm.at(0));

    QFlags<QIODevice::OpenModeFlag> mode = QIODevice::Text | QIODevice::WriteOnly;

    if(mParm.contains("append"))
    {
      mode = QIODevice::Text | QIODevice::WriteOnly | QIODevice::Append;
    }
    else
    {
      mode = QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate;
    }

    if(!mOutFile->open(mode))
    {
      error(FUNC, tr("Can't open file '%1'.").arg(mParm.at(0)));
      return false;
    }
  }
  else
  {
    //no outfile spezified, we write to stdout
    mOutFile = new QFile;
    if(!mOutFile->open(stdout, QIODevice::Text | QIODevice::WriteOnly))
    {
      error(FUNC, "Can't open stdout."); // Could that happens? don't think so -> not tr()
      return false;
    }

    if(!mCmdLine.contains("--verbose")) setVerboseLevel(eNoVerbose);
  }

  mOutput.setDevice(mOutFile);
  mLineNo = 0;
  mByteCount = 0;
  mDataR = 0;
  mDataW = 0;

  bool extraInfo = false;
  if(mCmdLine.contains("--all"))
  {
    mCmdLine << "--fiTypes" << "--symbolTypes" << "--markets" << "--offdays"
             << "--marketSymbols" << "--fiNames" << "--symbols" << "--eodRaw"
             << "--splits" << "--underlyings"
             << "--co" << "--groups" << "--depots";

    extraInfo = true;
  }

  if(mCmdLine.contains("--noBars"))
  {
    mCmdLine.removeAll("--eodRaw");
//     mCmdLine.removeAll();

    extraInfo = true;
  }

  if(mCmdLine.contains("--noUser"))
  {
    mCmdLine.removeAll("--co");
    mCmdLine.removeAll("--groups");
    mCmdLine.removeAll("--depots");
//     mCmdLine.removeAll();

    extraInfo = true;
  }

  // Look for filter settings
  if(FTool::getParameter(mCmdLine, "--fiType", mParm) > 0) mFilu->setSqlParm(":ftype", mParm.at(0));
  else mFilu->setSqlParm(":ftype", "");

  if(FTool::getParameter(mCmdLine, "--group", mParm) > 0) mFilu->setSqlParm(":group", mParm.at(0));
  else mFilu->setSqlParm(":group", "");

  if(FTool::getParameter(mCmdLine, "--market", mParm) > 0) mFilu->setSqlParm(":market", mParm.at(0));
  else mFilu->setSqlParm(":market", "");

  if(FTool::getParameter(mCmdLine, "--from", mParm) > 0) mFilu->setSqlParm(":fromDate", mParm.at(0));
  else mFilu->setSqlParm(":fromDate", "1000-01-01");

  if(FTool::getParameter(mCmdLine, "--to", mParm) > 0) mFilu->setSqlParm(":toDate", mParm.at(0));
  else mFilu->setSqlParm(":toDate", "3000-01-01");

  // Let us print some info at the beginning
  mBuffer << "***\n";
  mBuffer << "*\n";
  mBuffer << "* This file was generated by Filu at " << QDate::currentDate().toString(Qt::ISODate) << "\n*\n" ;
  mBuffer << "* Given export paramaters were: " << command.join(" ") << "\n";
  if(extraInfo)
  {
    mBuffer << "* Results in full parm list: " << mCmdLine.join(" ") << "\n";
  }
  mBuffer << "*\n";
  mBuffer << "* Data are filtered and grouped to given FI type and/or symbol type\n";
  mBuffer << "* and/or market and/or group membership.\n";
  mBuffer << "*" << endl << endl;

  // Look for export commands, and execute them if found
  if(mCmdLine.contains("--fiTypes"))      if(!expFiTypes()) return false;
  if(mCmdLine.contains("--symbolTypes"))  if(!expSymbolTypes()) return false; // Aka provider
  if(mCmdLine.contains("--markets"))      if(!expMarkets()) return false;
//   if(mCmdLine.contains("--offdays")) ; // Should always done when -markets given
//   if(mCmdLine.contains("--marketSymbols")) ;
  if(mCmdLine.contains("--fiNames"))      if(!expFiNames()) return false;
  if(mCmdLine.contains("--symbols"))      if(!expSymbols()) return false;
//   if(mCmdLine.contains("--eodAdjusted")) ;
  if(mCmdLine.contains("--splits"))       if(!expSplits())  return false; // What if -eodAdjusted? after reimport we have a problem
//   if(mCmdLine.contains("--underlyings")) ;
  if(mCmdLine.contains("--co"))           if(!expCOs())     return false;
  if(mCmdLine.contains("--groups"))       if(!expGroups())  return false;
//   if(mCmdLine.contains("--depots")) ;
//   if(mCmdLine.contains("--tstrategy")) ;
//   if(mCmdLine.contains("")) ;
  if(mCmdLine.contains("--eodRaw"))       if(!expEODRaw())  return false; // Because it takes the most time, do it last

  return true;
}

bool Exporter::expFiTypes()
{
  mDataText = "FiTypes";

  QSqlQuery* query = mFilu->execSql("GetAllFiTypes");

  if(!query)
  {
    printStatus(eEffectFault);
    check4FiluError(FUNC);
    return false;
  }

  if(noData("Info")) return true;

  // The query result has the format
  // FiType
  // See also GetAllFiTypes.sql

  mBuffer << "***\n";
  mBuffer << "*\n";
  mBuffer << "* All FI types.\n";
  mBuffer << "*\n";

  mBuffer << "[Header]FiType\n\n";
  writeToFile();

  while(query->next())
  {
    mBuffer << query->value(0).toString() << "\n";
    writeToFile();
  }

  mBuffer << endl;

  printStatus(eEffectOk);

  return true;
}

bool Exporter::expSymbolTypes()
{
  mDataText = "SymbolTypes";

  mFilu->setSqlParm(":all", true);
  QSqlQuery* query = mFilu->execSql("GetSymbolTypes");

  if(!query)
  {
    printStatus(eEffectFault);
    check4FiluError(FUNC);
    return false;
  }

  if(noData("Info")) return true;

  // The query result has the format
  // SymbolType, SEQ, IsProvider
  // See also GetAllSymbolTypes.sql

  mBuffer << "***\n";
  mBuffer << "*\n";
  mBuffer << "* All symbol types.\n";
  mBuffer << "*\n";

  mBuffer << "[Header]SymbolType;SEQ;IsProvider\n\n";
  writeToFile();

  while(query->next())
  {
    mBuffer << query->value(1).toString() << ";";
    mBuffer << query->value(2).toString() << ";";
    mBuffer << query->value(3).toBool() << "\n";
    writeToFile();
  }

  mBuffer << endl;

  printStatus(eEffectOk);

  return true;
}

bool Exporter::expMarkets()
{
  mDataText = "Markets";

  QSqlQuery* query = mFilu->execSql("GetAllMarkets");

  if(!query)
  {
    printStatus(eEffectFault);
    check4FiluError(FUNC);
    return false;
  }

  if(noData("Info")) return true;

  // The query result has the format
  // MarketId, Market, OpenTime, CloseTime, Currency, CurrencySymbol
  // See also GetAllMarkets.sql

  mBuffer << "***\n";
  mBuffer << "*\n";
  mBuffer << "* All markets with currency and currency symbol.\n";
  mBuffer << "*\n";

  mBuffer << "[Header]Market;OpenTime;CloseTime;Currency;CurrencySymbol\n\n";
  writeToFile();
  while(query->next())
  {
    // Skip marketId, query->value(0).toString();
    mBuffer << query->value(1).toString() << ";";
    mBuffer << query->value(2).toTime().toString(Qt::TextDate) << ";";
    mBuffer << query->value(3).toTime().toString(Qt::ISODate) << ";";
    mBuffer << query->value(4).toString() << ";";
    mBuffer << query->value(5).toString() << ";\n";
    writeToFile();
  }

  mBuffer << endl;

  printStatus(eEffectOk);

  return true;
}

bool Exporter::expFiNames()
{
  mDataText = "FiNames";

  if(!selectFis())
  {
    if(hasError()) return false;
    return true;
  }

  // The query result has the format
  // Name, Type, DDate, Symbol, Provider, Market, IDate, MDate
  // See also GetAllFi.sql

  mBuffer << "***\n";
  mBuffer << "*\n";
  mBuffer << "* FI names with full lovely symbol set as reference.\n";
  mBuffer << "*\n";

  mBuffer << "[Header]Name;DDate;Symbol;Provider;Market;IDate;MDate\n\n";
  writeToFile();

  QString type;
  while(mFis->next())
  {
    if(type != mFis->value(1).toString())
    {
      mBuffer << "*\n";
      mBuffer << "* FiType: " <<  mFis->value(1).toString() << "\n";
      mBuffer << "*\n";
      mBuffer << "[Type]" << mFis->value(1).toString() << "\n";
    }

    type = mFis->value(1).toString();

    mBuffer << mFis->value(0).toString() << ";";
    mBuffer << mFis->value(2).toDate().toString(Qt::ISODate) << ";";
    mBuffer << mFis->value(3).toString() << ";";
    mBuffer << mFis->value(4).toString() << ";";
    mBuffer << mFis->value(5).toString() << ";";
    mBuffer << mFis->value(6).toDate().toString(Qt::ISODate) << ";";
    mBuffer << mFis->value(7).toDate().toString(Qt::ISODate) << ";";
    mBuffer << "\n";
    writeToFile();
  }

  mBuffer << endl;

  printStatus(eEffectOk);

  return true;
}

bool Exporter::expSymbols()
{
  mDataText = "Symbols";

  QSqlQuery* query = mFilu->execSql("GetAllSymbols");

  if(!query)
  {
    printStatus(eEffectFault);
    check4FiluError(FUNC);
    return false;
  }

  if(noData("Info")) return true;

  // The query result has the format
  // RefSymbol, FiType, Symbol, Provider, Market, IDate, MDate
  // See also GetAllSymbols.sql.sql

  mBuffer << "***\n";
  mBuffer << "*\n";
  mBuffer << "* All symbols with leading lovely symbol as reference.\n";
  mBuffer << "*\n";

  mBuffer << "[Header]RefSymbol;Symbol;IDate;MDate\n"; // One \n less...
  writeToFile();

  FTool::getParameter(mCmdLine, "--symbols", mParm);

  QString refSymbol, type, symbol, provider, market, idate, mdate;

  bool separator = false;

  while(query->next())
  {
    if(mParm.size())
    {
      if(!mParm.contains(query->value(3).toString())) continue;
    }

    if(type != query->value(1).toString()) separator = true;
    else if(provider != query->value(3).toString()) separator = true;
    else if(market != query->value(4).toString()) separator = true;

    int i = 0;
    refSymbol = query->value(i++).toString();
    type      = query->value(i++).toString();
    symbol    = query->value(i++).toString();
    provider  = query->value(i++).toString();
    market    = query->value(i++).toString();
    idate     = query->value(i++).toDate().toString(Qt::ISODate);
    mdate     = query->value(i++).toDate().toString(Qt::ISODate);

    if(separator)
    {
      mBuffer << "\n"; // ...because we need it here
      mBuffer << "*\n";
      mBuffer << "* Type:" << type << "  Provider:" << provider << "  Market:" << market << "\n";
      mBuffer << "*\n";
      mBuffer << "[Type]" <<  type << "\n";
      mBuffer << "[Provider]" << provider  << "\n";
      mBuffer << "[Market]" << market  << "\n";
      writeToFile();
      separator = false;
    }

    mBuffer << refSymbol << ";" << symbol << ";" << idate << ";" << mdate << "\n";
    writeToFile();
  }

  mBuffer << endl;

  printStatus(eEffectOk);

  return true;
}

bool Exporter::expEODRaw()
{
  mDataText = "EODBars";

  if(!selectFis())
  {
    if(hasError()) return false;
    return true;
  }

  mBuffer << "***\n";
  mBuffer << "*\n";
  mBuffer << "* EOD raw data\n";
  mBuffer << "*\n";

  mBuffer << "[Header]Date;Open;High;Low;Close;Volume;OpenInterest;Quality\n\n";
  writeToFile();

  QString fromFi, toFi;
  while(mFis->next())
  {
    QSqlRecord fi = mFis->record();

    if(fromFi.isEmpty()) fromFi = fi.value("Symbol").toString();

    mFilu->setSqlParm(":fiId", fi.value("FiId").toInt());
    QSqlQuery* query = mFilu->execSql("GetAllEODRawData");

    if(!query)
    {
      printStatus(eEffectFault);
      check4FiluError(FUNC);
      return false;
    }

    if(noData(tr("No bars for %1").arg(fi.value("Name").toString())))
    {
      fromFi.clear();
      toFi.clear();
      continue;
    }

    if(toFi.isEmpty()) printStatus(eEffectPending, fi.value("Name").toString());
    else printStatus(eEffectPending, QString("%1...%2").arg(fromFi, toFi));

    toFi = fi.value("Symbol").toString();

    QString market;
    while(query->next())
    {
      if(market != query->value(0).toString())
      {
        market = query->value(0).toString();

        mBuffer << "*\n";
        mBuffer << "* " << fi.value("Name").toString() << " at " << market << "\n";
        mBuffer << "*\n";
        mBuffer << "[RefSymbol]" << toFi << "\n";
        mBuffer << "[Market]" << query->value(0).toString() << "\n";
      }

      mBuffer << query->value(1).toDate().toString(Qt::ISODate) << ";";
      mBuffer << query->value(2).toDouble() << ";"; // open
      mBuffer << query->value(3).toDouble() << ";";
      mBuffer << query->value(4).toDouble() << ";";
      mBuffer << query->value(5).toDouble() << ";";
      mBuffer << query->value(6).toDouble() << ";"; // volume
      mBuffer << query->value(7).toDouble() << ";"; // open interest
      mBuffer << query->value(8).toInt() << "\n";   // quality

      writeToFile();
    }

    mBuffer << endl;

    if(verboseLevel(eAmple))
    {
      printStatus(eEffectOk);
      fromFi.clear();
      toFi.clear();
    }
  }

  if(verboseLevel() < eAmple and !fromFi.isEmpty())
  {
    if(toFi.isEmpty()) printStatus(eEffectOk, fromFi);
    else printStatus(eEffectOk, QString("%1...%2").arg(fromFi, toFi));
  }

  return true;
}

bool Exporter::expSplits()
{
  mDataText = "Splits";

  QSqlQuery* query = mFilu->execSql("GetAllSplits");

  if(!query)
  {
    printStatus(eEffectFault);
    check4FiluError(FUNC);
    return false;
  }

  if(noData("Info")) return true;

  // The query result has the format
  // RefSymbol, SpliDate, SplitComment, SplitRatio
  // See also GetAllSplits.sql

  mBuffer << "***\n";
  mBuffer << "*\n";
  mBuffer << "* Split data\n";
  mBuffer << "*\n";

  mBuffer << "[Header]RefSymbol;SplitDate;SplitComment;SplitRatio\n\n";
  writeToFile();

  while(query->next())
  {
    mBuffer << query->value(0).toString() << ";"; //
    mBuffer << query->value(1).toDate().toString(Qt::ISODate) << ";";
    mBuffer << query->value(2).toString() << ";"; //
    mBuffer << query->value(3).toDouble() << "\n"; //
    writeToFile();
  }

  mBuffer << endl;

  printStatus(eEffectOk);

  return true;
}

bool Exporter::expCOs()
{
  mDataText = "COs";

  QSqlQuery* query = mFilu->execSql("GetAllCOs");

  if(!query)
  {
    printStatus(eEffectFault);
    check4FiluError(FUNC);
    return false;
  }

  if(noData("Info")) return true;

  // The query result has the format
  // RefSymbol, Market, CODate, Plot, Type, ArgV

  mBuffer << "***\n";
  mBuffer << "*\n";
  mBuffer << "* Chart object data\n";
  mBuffer << "*\n";

  mBuffer << "[Header]RefSymbol;Market;CODate;Plot;Type;ArgV\n\n";
  writeToFile();

  while(query->next())
  {
    int i = 0;
    mBuffer << query->value(i++).toString() << ";";  // RefSymbol
    mBuffer << query->value(i++).toString() << ";";  // Market
    mBuffer << query->value(i++).toDate().toString(Qt::ISODate) << ";";
    mBuffer << query->value(i++).toString() << ";";  // Plot
    mBuffer << query->value(i++).toString() << ";";  // Type

    // Convert before export
    mBuffer << FTool::txtToLine(query->value(i++).toString()) << "\n";
    writeToFile();
  }

  mBuffer << endl;

  printStatus(eEffectOk);

  return true;
}

bool Exporter::expGroups()
{
  if(verboseLevel(eAmple)) mDataText = "Group";
  else mDataText = "Group members";

  QStringList buffer;
  buffer << "***";
  buffer << "*";
  buffer << "* Group data";
  buffer << "*";
  buffer << "[Header]RefSymbol\n\n";

  if(FTool::getParameter(mCmdLine, "--group", mParm) > 0)
  {
    mDataText = "Group";
    mBuffer << buffer.join("\n");
    writeToFile();
    if(!expGroup(mFilu->getGroupId(mParm.at(0)), false)) return false;
  }
  else
  {
    mFilu->setSqlParm(":motherId", 0);
    QSqlQuery* query = mFilu->execSql("GetAllGroups");

    if(!query)
    {
      printStatus(eEffectFault);
      check4FiluError(FUNC);
      return false;
    }

    if(noData("Info")) return true;

    mDataR = 0; // Don't count the group name, only entries

    mBuffer << buffer.join("\n");
    writeToFile();

    while(query->next())
    {
      if(!expGroup(query->value(0).toInt())) return false;
    }
  }

  if(!verboseLevel(eAmple)) printStatus(eEffectOk);

  return true;
}

bool Exporter::expGroup(int gid, bool ignoreIfEmpty/* = true*/)
{
  // FIXME Looks ugly that 'ignoreIfEmpty'. Do it smarter at all

  if(verboseLevel(eAmple)) ignoreIfEmpty = false;

  mFilu->setSqlParm(":groupId", gid);
  QSqlQuery* query = mFilu->execSql("GetGroupPathById");
  query->next();
  QString path = query->value(0).toString();

  query = mFilu->execSql("GetGMembers");

  if(!query)
  {
    check4FiluError(FUNC);
    return false;
  }

  if(ignoreIfEmpty)
  {
    if(noData()) return true;
  }
  else
  {
    if(noData(QString("%1 has no member.").arg(path))) return true;
  }

  mBuffer << "*\n";
  mBuffer << "* Group members of " << path << "\n";
  mBuffer << "*\n";
  mBuffer << "[GroupPath]" << path << "\n";
  writeToFile();

  while(query->next())
  {
    mBuffer << query->value(2).toString() << "\n"; // Symbol
    writeToFile();
  }

  mBuffer << endl;

  if(verboseLevel(eAmple)) printStatus(eEffectOk, path);

  return true;
}