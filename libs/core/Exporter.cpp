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

#include <QFile>
#include <QFile>
#include <QSqlQuery>
#include <QSqlRecord>

#include "FiluU.h"

#include "CmdHelper.h"
#include "Exporter.h"
#include "FTool.h"

const QString cCmd1 = "exp";
const QString cCmd1Brief = QObject::tr("Export data from the database");

const QString cOptFilter = "fiType listed market from to noBars noUser symbols "
                           "owner dpid withSim";
const QString cOptHeader = QObject::tr("Possible filter options (hyphens are omitted) are:");

using namespace std;

#define NewLine "\n"

Exporter::Exporter(FClass* parent)
        : FClass(parent, FUNC)
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

void Exporter::briefIn(CmdHelper* cmd)
{
  if(!cmd) return;

  cmd->inCmdBrief(cCmd1, cCmd1Brief);
}

void Exporter::printStatus(Effect effectEnum/* = eEffectPending*/, const QString& extraTxt/* = ""*/)
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

bool Exporter::noData(const QString& what/* = ""*/, const VerboseLevel when/* = eVerbose*/)
{
  if(mFilu->lastResult() > Filu::eNoData)
  {
    mDataR += mFilu->lastQuery()->size();
    mHint.clear();
    return false;
  }

  if(what.isEmpty()) return true;
  if(!verboseLevel(when)) return true;

  if(mDataW) printStatus(eEffectOk);

  QString txt;
  if(what == "Info") txt = tr("No data found to export.");
  else txt = what;

  mHint.clear();
  printStatus(eEffectNote, txt);
  return true;
}

bool Exporter::badQuery(QSqlQuery* query)
{
  if(query) return false;

  printStatus(eEffectFault);
  check4FiluError(FUNC);
  return true;
}

void Exporter::writeToFile()
{
  if(mOLine.size())
  {
    mOLine.replaceInStrings(QRegExp("^\n$"), "");
    const QString buffer = mOLine.join("\n");
    mLineNo    += mOLine.size();
    mByteCount += buffer.size() + 1;  // One more for next comming 'endl'
    mOutput << buffer << endl;
    mOLine.clear();
  }

  if(mDLine.size())
  {
    const QString buffer = mDLine.join(";");
    mByteCount += buffer.size() + 1;  // One more for next comming 'endl'
    mOutput << buffer << endl;
    ++mLineNo;
    ++mDataW;
    mDataLineNo.enqueue(mLineNo);
    if(mDataLineNo.size() > 1) mDataLineNo.dequeue(); // Store max one line number(!?)
    mDLine.clear();
  }

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

bool Exporter::exec(CmdHelper* ch)
{
   if(!ch)
  {
    fatal(FUNC, "Called with NULL pointer.");
    return false;
  }

  mCmd = ch;
  mCmd->regSubCmds("all core user data");
  mCmd->regStdOpts("into");
  mCmd->regOpts("fiType from listed market noBars noUser to "/*"reset "*/
                "symbols owner dpid withSim");

  if(mCmd->subCmdLooksBad()) return false;

  if(mCmd->wantHelp())
  {
    mCmd->inSubBrief("all", tr("To export all kind of data"));
    mCmd->inSubBrief("core", tr("To export all but no user data"));
    mCmd->inSubBrief("user", tr("To export only user data"));
    mCmd->inSubBrief("data", tr("To export explicit noted data"));
    //mCmd->inSubBrief("", tr(""));

    mCmd->inOptBrief("into", "<FileName> [+]"
                    , tr("Write the exported data into <FileName>. If '+' is given the data will "
                          "appended to the existing file. If no '--into' is given, the data will be "
                          "written to stdout. The verbose level is in that case set to 'Quiet' unless "
                          "verbose is given"));

    mCmd->inOptBrief("fiType", "<FiType>", tr("Only this type, e.g. 'Stock'"));
    mCmd->inOptBrief("from", "<Date>", tr("Should be given as YYYY-MM-DD"));
    mCmd->inOptBrief("listed", "<GroupPath>", tr("Only FIs in this PerformerF group createt by the user"));
    mCmd->inOptBrief("market", "<Market>", tr("Only this market, e.g. 'NYSE'"));
    mCmd->inOptBrief("noBars", "", tr("Don't export eodBar data"));
    mCmd->inOptBrief("noUser", "", tr("Don't export user data"));
    mCmd->inOptBrief("to", "<Date>", tr("Should be given as YYYY-MM-DD"));

    mCmd->inOptBrief("symbols", "<Type>.."
                    , tr("Only these kind of symbols. But note that <Type> can not be your "
                        "lovely symbol, this is always used as RefSymbol. When given is no need "
                        "to note also 'symbol' explicit to export as data type"));

    mCmd->inOptBrief("owner", "<Name>", tr("Only depots by named owner"));
    mCmd->inOptBrief("dpid", "<Id>", tr("Only this depot"));
    mCmd->inOptBrief("withSim", "", tr("Don't suppress Simulator depots"));
    //mCmd->inOptBrief("reset", "", "Only used by ");
    //mCmd->inOptBrief("", "", "");

    mCmd->inLabel("DataSet", tr("data sets"));
    mCmd->inOptGroup("Filter", tr("filter options")
                    , "from to owner dpid withSim symbols market fiType listed");
  }

  if(mCmd->needHelp(2))
  {
    if(mCmd->printThisWay("<DataSet> [<Filter>]")) return true;

    mCmd->aided();
    return true;
  }

  // Look for each command, and execute them if was given.
  // The order of look up is important.
  if(mCmd->has("verbose")) setVerboseLevel(FUNC, mCmd->cmdLine());

  if(mCmd->hasSubCmd("all")) cmdAll();
  else if(mCmd->hasSubCmd("core")) cmdCore();
  else if(mCmd->hasSubCmd("user")) cmdUser();
  else if(mCmd->hasSubCmd("data")) cmdData();
  else
  {
    fatal(FUNC, QString("Unsupported command: %1").arg(mCmd->cmd()));
    return false;
  }

  return exxport();
}

void Exporter::cmdAll()
{
  QStringList data;
  data << "fiTypes" << "symbolTypes" << "markets" << "offdays"
       /*<< "marketSymbols" */<< "fiNames" << "symbols" << "eodRaw"
       << "splits" << "broker" << "ulys"
       << "co" << "groups" << "depots";

  if(mCmd->isMissingParms())
  {
    mCmd->groupOpts("Filter", "from to noBars noUser fiType listed market symbols");

    if(mCmd->printThisWay("[<Filter>]")) return;

    data.sort();
    mCmd->printComment(tr("Exported dates are: %1").arg(data.join(" ")));
    mCmd->printNote(tr("There will only user data of the current user exported."));
    mCmd->printForInst("--noBars --noUser");
    mCmd->aided();
    return;
  }

  mExport << data;
}

void Exporter::cmdCore()
{
  QStringList data;
  data << "fiTypes" << "symbolTypes" << "markets" << "offdays"
       /*<< "marketSymbols" */<< "fiNames" << "symbols" << "eodRaw"
       << "splits" << "broker" << "ulys";

  if(mCmd->isMissingParms())
  {
    mCmd->groupOpts("Filter", "from to noBars fiType listed market symbols");

    if(mCmd->printThisWay("[<Filter>]")) return;

    data.sort();
    mCmd->printComment(tr("Exported dates are: %1").arg(data.join(" ")));
    mCmd->printForInst("--fiType Currency");
    mCmd->aided();
    return;
  }

  mExport << data;
}

void Exporter::cmdUser()
{
  QStringList data;
  data << "co" << "groups" << "depots";

  if(mCmd->isMissingParms())
  {
    mCmd->groupOpts("Filter", "from to owner dpid withSim");

    if(mCmd->printThisWay("[<Filter>]")) return;

    mCmd->printComment(tr("There will only data of the current user exported. "
                          "The date filter affect the chart objects but not "
                          "the depot data."));
    data.sort();
    mCmd->printComment(tr("Exported dates are: %1").arg(data.join(" ")));
    mCmd->aided();
    return;
  }

  mExport << data;
}

void Exporter::cmdData()
{
  QStringList data;
  data << "fiTypes" << "symbolTypes" << "markets" << "offdays"
       /*<< "marketSymbols" */<< "fiNames" << "symbols" << "eodRaw"
       << "splits" << "broker" << "ulys"
       << "co" << "groups" << "depots";

  if(mCmd->isMissingParms())
  {
    mCmd->groupOpts("Filter", "from to owner dpid withSim symbols market fiType listed");

    if(mCmd->printThisWay("<Data>.. [<Filter>]")) return;

    data.sort();
    mCmd->printComment(tr("Possible dates are: %1").arg(data.join(" ")));
    mCmd->printNote(tr("There will only user data of the current user exported, if some given."));

    mCmd->printForInst(QString("fiNames eodRaw splits --from %1").arg(QDate::currentDate().addDays(-20).toString(Qt::ISODate)));
    mCmd->printForInst("fiNames --fiType Index");
    mCmd->printForInst("depots --owner Me");
    mCmd->aided();
    return;
  }

  mExport << mCmd->parmList("data");
}

bool Exporter::exxport()
{
  if(hasError()) return false;

  // Open output file, if needed
  if(mCmd->has("into"))
  {
    QString fileName = mCmd->optStr("into");
    if(fileName.isEmpty())
    {
      error(FUNC, tr("Missing file name at '--to'."));
      return false;
    }

    mOutFile = new QFile(fileName);

    QFlags<QIODevice::OpenModeFlag> mode = QIODevice::Text | QIODevice::WriteOnly;

    if(mCmd->optStr("into", "", 2) == "+")
    {
      mode = QIODevice::Text | QIODevice::WriteOnly | QIODevice::Append;
    }
    else
    {
      mode = QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate;
    }

    if(!mOutFile->open(mode))
    {
      error(FUNC, tr("Can't open file '%1'.").arg(fileName));
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

    if(!mCmd->has("verbose")) setVerboseLevel(eNoVerbose);
  }

  mOutput.setDevice(mOutFile);
  mLineNo = 0;
  mByteCount = 0;
  mDataR = 0;
  mDataW = 0;

  if(mCmd->has("noBars"))
  {
    mExport.removeAll("eodRaw");
//     mExport.removeAll();
  }

  if(mCmd->has("noUser"))
  {
    mExport.removeAll("co");
    mExport.removeAll("groups");
    mExport.removeAll("depots");
//     mExport.removeAll();
  }

  if(mCmd->has("symbols"))
  {
    mExport << "symbols";
  }

  // Look for filter settings
  mFilu->setSqlParm(":ftype", mCmd->optStr("fiType"));
  mFilu->setSqlParm(":group", mCmd->optStr("listed"));
  mFilu->setSqlParm(":market", mCmd->optStr("market"));
  mFilu->setSqlParm(":fromDate", mCmd->optDate("from", QDate(1000, 1, 1)));
  mFilu->setSqlParm(":toDate", mCmd->optDate("to", QDate(3000, 1, 1)));

  if(mCmd->hasError()) return false; // Perhaps bad date

  // Let us print some info at the beginning
  mOLine << "***";
  mOLine << "*";
  mOLine << "* This file was generated by Filu at " + QDate::currentDate().toString(Qt::ISODate);
  mOLine << "* Command was:";
  mOLine << "*     " + mCmd->cmdLine().join(" ");
  mOLine << "* Results in data list:";
  mOLine << "*     " + mExport.join(" ");
  mOLine << "*";
  mOLine << "* Data are filtered and grouped to given FI type and/or symbol type";
  mOLine << "* and/or market and/or group membership";
  mOLine << "*";

  // Look for export commands, and execute them if found
  if(mExport.contains("fiTypes"))      if(!expFiTypes()) return false;
  if(mExport.contains("symbolTypes"))  if(!expSymbolTypes()) return false; // Aka provider
  if(mExport.contains("markets"))      if(!expMarkets()) return false;
//   if(mExport.contains("offdays")) ; // Should always done when -markets given
//   if(mExport.contains("marketSymbols")) ;
  if(mExport.contains("fiNames"))      if(!expFiNames()) return false;
  if(mExport.contains("ulys"))         if(!expUnderlyings()) return false;
  if(mExport.contains("symbols"))      if(!expSymbols()) return false;
//   if(mExport.contains("eodAdjusted")) ;
  if(mExport.contains("splits"))       if(!expSplits())  return false; // What if -eodAdjusted? after reimport we have a problem
  if(mExport.contains("broker"))       if(!expBroker())  return false;
  if(mExport.contains("co"))           if(!expCOs())     return false;
  if(mExport.contains("groups"))       if(!expGroups())  return false;
  if(mExport.contains("depots"))       if(!expDepots())  return false;
//   if(mExport.contains("tstrategy")) ;
//   if(mExport.contains("")) ;
  if(mExport.contains("eodRaw"))       if(!expEODRaw())  return false; // Because it takes the most time, do it last

  return true;
}

bool Exporter::expFiTypes()
{
  mDataText = "FiTypes";

  QSqlQuery* query = mFilu->execSql("GetAllFiTypes");

  if(badQuery(query)) return false;
  if(noData("Info"))  return true;

  mOLine << "***";
  mOLine << "*";
  mOLine << "* All FI types";
  mOLine << "*";
  mOLine << "[Header]FiType";
  mOLine << NewLine;

  writeToFile();

  while(query->next())
  {
    mDLine << query->value(0).toString();
    writeToFile();
  }

  mOLine << NewLine;
  printStatus(eEffectOk);

  return true;
}

bool Exporter::expSymbolTypes()
{
  mDataText = "SymbolTypes";

  mFilu->setSqlParm(":all", true);
  QSqlQuery* query = mFilu->execSql("GetSymbolTypes");

  if(badQuery(query)) return false;
  if(noData("Info"))  return true;

  mOLine << "***";
  mOLine << "*";
  mOLine << "* All symbol types";
  mOLine << "*";
  mOLine << "[Header]SymbolType;SEQ;IsProvider";
  mOLine << NewLine;
  writeToFile();

  while(query->next())
  {
    QSqlRecord st = query->record();
    mDLine << st.value("SymbolType").toString();
    mDLine << st.value("SEQ").toString();
    mDLine << st.value("IsProvider").toString();
    writeToFile();
  }

  mOLine << NewLine;
  printStatus(eEffectOk);

  return true;
}

bool Exporter::expMarkets()
{
  mDataText = "Markets";

  mFilu->setSqlParm(":marketId", 0);
  QSqlQuery* query = mFilu->execSql("GetMarket");

  if(badQuery(query)) return false;
  if(noData("Info"))  return true;

  mOLine << "***";
  mOLine << "*";
  mOLine << "* All markets with currency and currency symbol";
  mOLine << "*";
  mOLine << "[Header]Market;OpenTime;CloseTime;CurrencySymbol;Currency";
  mOLine << NewLine;

  writeToFile();
  while(query->next())
  {
    QSqlRecord m = query->record();
    mDLine << m.value("Market").toString();
    mDLine << m.value("OpenTime").toTime().toString(Qt::ISODate);
    mDLine << m.value("CloseTime").toTime().toString(Qt::ISODate);
    mDLine << m.value("CurrencySymbol").toString();
    mDLine << m.value("Currency").toString();
    writeToFile();
  }

  mOLine << NewLine;
  printStatus(eEffectOk);

  return true;
}

bool Exporter::expFiNames()
{
  mDataText = "FiNames";

  if(!selectFis()) return !hasError();

  mOLine << "***";
  mOLine << "*";
  mOLine << "* FI names with full lovely symbol set as reference";
  mOLine << "*";
  mOLine << "[Header]Name;DDate;Symbol;Provider;Market;IDate;MDate";
  writeToFile();

  QString type;
  while(mFis->next())
  {
    QSqlRecord fi = mFis->record();

    if(type != fi.value("Type").toString())
    {
      type = fi.value("Type").toString();

      mOLine << NewLine;
      mOLine << "*";
      mOLine << "* FiType: " + type;
      mOLine << "*";
      mOLine << "[Type]" + type;
      mOLine << NewLine;
    }

    mDLine << fi.value("Name").toString();
    mDLine << fi.value("DDate").toDate().toString(Qt::ISODate);
    mDLine << fi.value("Symbol").toString();
    mDLine << fi.value("Provider").toString();
    mDLine << fi.value("Market").toString();
    mDLine << fi.value("IDate").toDate().toString(Qt::ISODate);
    mDLine << fi.value("MDate").toDate().toString(Qt::ISODate);
    writeToFile();
  }

  mOLine << NewLine;
  printStatus(eEffectOk);

  return true;
}

bool Exporter::expUnderlyings()
{
  mDataText = "CompList";

  if(!selectFis()) return !hasError();

  mDataR = 0; // We are not interested in the total amound of FIs
  bool intro = true;

  while(mFis->next())
  {
    QSqlRecord mfi = mFis->record();
    if(mfi.value("Type").toString() == "Stock") continue; //

    mFilu->setSqlParm(":motherId", mfi.value("FiId").toInt());
    QSqlQuery* query = mFilu->execSql("GetUnderlyings");

    if(badQuery(query)) return false;
    if(noData()) continue;

    //++mDataR; // Count the mother
    --mDataW; // Sub the mother

    if(intro)
    {
      mOLine << "***";
      mOLine << "*";
      mOLine << "* Underlyings with full lovely symbol set as reference";
      mOLine << "*";
      intro = false;
    }

    mOLine << "*";
    mOLine << "* " + mfi.value("Name").toString();
    mOLine << "*";
    mOLine << "[Header]Name;DDate;Symbol;Provider;Market;IDate;MDate";
    mOLine << NewLine;

    mDLine << mfi.value("Name").toString();
    mDLine << mfi.value("DDate").toDate().toString(Qt::ISODate);
    mDLine << mfi.value("Symbol").toString();
    mDLine << mfi.value("Provider").toString();
    mDLine << mfi.value("Market").toString();
    mDLine << mfi.value("IDate").toDate().toString(Qt::ISODate);
    mDLine << mfi.value("MDate").toDate().toString(Qt::ISODate);
    writeToFile();

    mOLine << NewLine;
    mOLine << "[Header]Weight;Name;Type;Symbol;Provider;Market";
    mOLine << "[CompList]" + mfi.value("Symbol").toString();
    mOLine << NewLine;
    writeToFile();

    while(query->next())
    {
      QSqlRecord ufi = query->record();
      mDLine << ufi.value("Weight").toString();
      mDLine << ufi.value("CompName").toString();
      mDLine << ufi.value("CompType").toString();
      mDLine << ufi.value("Comp").toString();
      mDLine << ufi.value("CompSymbolType").toString();
      mDLine << ufi.value("CompSymbolMarket").toString();
      writeToFile();
    }

    mOLine << NewLine;
    mOLine << "[CompListEnd]";
    mOLine << NewLine;

    if(verboseLevel(eAmple)) printStatus(eEffectOk, mfi.value("Name").toString());
  }

  mOLine << NewLine;
  writeToFile();

  if(intro) printStatus(eEffectNote, tr("No underlyings found."));
  else if(verboseLevel() < eAmple) printStatus(eEffectOk);

  return true;
}

bool Exporter::expSymbols()
{
  mDataText = "Symbols";

  QSqlQuery* query = mFilu->execSql("GetAllSymbols");

  if(badQuery(query)) return false;
  if(noData("Info"))  return true;

  // The query result has the format
  // RefSymbol, FiType, Symbol, Provider, Market, IDate, MDate
  // See also GetAllSymbols.sql

  mOLine << "***";
  mOLine << "*";
  mOLine << "* All symbols with leading lovely symbol as reference";
  mOLine << "*";
  mOLine << "[Header]RefSymbol;Symbol;IDate;MDate";
  // One NewLine less...
  writeToFile();

  QString refSymbol, type, symbol, provider, market, idate, mdate;

  bool separator = false;

  QStringList onlySymbols = mCmd->parmList("symbols");

  while(query->next())
  {
    if(onlySymbols.size() and !onlySymbols.contains(query->value(3).toString())) continue;

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
      mOLine << NewLine;   // ...because we need it here
      mOLine << "*";
      mOLine << "* Type:" + type + "  Provider:" + provider + "  Market:" + market;
      mOLine << "*";
      mOLine << "[Type]" +  type;
      mOLine << "[Provider]" + provider ;
      mOLine << "[Market]" + market ;
      mOLine << NewLine;
      writeToFile();
      separator = false;
    }

    mDLine << refSymbol << symbol << idate << mdate;
    writeToFile();
  }

  mOLine << NewLine;
  printStatus(eEffectOk);

  return true;
}

bool Exporter::expEODRaw()
{
  mDataText = "EODBars";

  if(!selectFis()) return !hasError();

  mOLine << "***";
  mOLine << "*";
  mOLine << "* EOD raw data";
  mOLine << "*";
  mOLine << "[Header]Date;Open;High;Low;Close;Volume;OpenInterest;Quality";
  mOLine << NewLine;
  writeToFile();

  QString fromFi, toFi;
  while(mFis->next())
  {
    QSqlRecord fi = mFis->record();

    if(fromFi.isEmpty()) fromFi = fi.value("Symbol").toString();

    mFilu->setSqlParm(":fiId", fi.value("FiId").toInt());
    QSqlQuery* query = mFilu->execSql("GetAllEODRawData");

    if(badQuery(query)) return false;

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
      QSqlRecord b = query->record();

      if(market != b.value("Market").toString())
      {
        market = b.value("Market").toString();

        mOLine << "*";
        mOLine << "* " + fi.value("Name").toString() + " at " + market;
        mOLine << "*";
        mOLine << "[RefSymbol]" + toFi;
        mOLine << "[Market]" + market;
        mOLine << NewLine;
      }

      mDLine << b.value("Date").toDate().toString(Qt::ISODate);
      mDLine << b.value("Open").toString();
      mDLine << b.value("High").toString();
      mDLine << b.value("Low").toString();
      mDLine << b.value("Close").toString();
      mDLine << b.value("Volume").toString();
      mDLine << b.value("OpenInterest").toString();
      mDLine << b.value("Quality").toString();
      writeToFile();
    }

    mOLine << NewLine;

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

  if(badQuery(query)) return false;
  if(noData("Info"))  return true;

  mOLine << "***";
  mOLine << "*";
  mOLine << "* Split data";
  mOLine << "*";
  mOLine << "[Header]RefSymbol;SplitDate;SplitRatio;SplitComment";
  mOLine << NewLine;
  writeToFile();

  while(query->next())
  {
    QSqlRecord s = query->record();
    mDLine << s.value("Symbol").toString();
    mDLine << s.value("Date").toDate().toString(Qt::ISODate);
    mDLine << s.value("Ratio").toString();
    mDLine << s.value("Comment").toString();
    writeToFile();
  }

  mOLine << NewLine;
  printStatus(eEffectOk);

  return true;
}

bool Exporter::expBroker()
{
  mDataText = "Broker";

  mFilu->setSqlParm(":brokerId", 0);
  QSqlQuery* query = mFilu->execSql("GetBroker");

  if(badQuery(query)) return false;
  if(noData("Info"))  return true;

  mOLine << "***";
  mOLine << "*";
  mOLine << "* Broker";
  mOLine << "*";
  mOLine << "[Header]BrokerName;CurrencySymbol;FeeFormula";
  mOLine << NewLine;
  writeToFile();

  while(query->next())
  {
    QSqlRecord r = query->record();
    mDLine << r.value("Name").toString();
    mDLine << r.value("Currency").toString();
    mDLine << r.value("FeeFormula").toString();
    writeToFile();
  }

  mOLine << NewLine;
  printStatus(eEffectOk);

  return true;
}

bool Exporter::expCOs()
{
  mDataText = "COs";

  QSqlQuery* query = mFilu->execSql("GetAllCOs");

  if(badQuery(query)) return false;
  if(noData("Info"))  return true;

  mOLine << "***";
  mOLine << "*";
  mOLine << "* Chart object data";
  mOLine << "*";
  mOLine << "[Header]RefSymbol;Market;CODate;Plot;Type;ArgV";
  mOLine << NewLine;
  writeToFile();

  while(query->next())
  {
    QSqlRecord co = query->record();
    mDLine << co.value("RefSymbol").toString();
    mDLine << co.value("Market").toString();
    mDLine << co.value("CODate").toDate().toString(Qt::ISODate);
    mDLine << co.value("Plot").toString();
    mDLine << co.value("Type").toString();
    // Convert before export
    mDLine << FTool::txtToLine(co.value("ArgV").toString());
    writeToFile();
  }

  mOLine << NewLine;
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
  buffer << "[Header]RefSymbol\n";

  if(mCmd->has("listed"))
  {
    mDataText = "Group";
    mOLine << buffer.join("\n");
    writeToFile();
    if(!expGroup(mFilu->getGroupId(mCmd->optStr("listed")), false)) return false;
  }
  else
  {
    mFilu->setSqlParm(":motherId", 0);
    QSqlQuery* query = mFilu->execSql("GetAllGroups");

    if(badQuery(query)) return false;
    if(noData("Info"))  return true;

    mDataR = 0; // Don't count the group name, only entries

    mOLine << buffer.join("\n");
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

  if(badQuery(query)) return false;

  if(ignoreIfEmpty)
  {
    if(noData()) return true;
  }
  else
  {
    if(noData(QString("%1 has no member.").arg(path))) return true;
  }

  mOLine << "*";
  mOLine << "* Group members of " + path;
  mOLine << "*";
  mOLine << "[GroupPath]" + path;
  mOLine << NewLine;
  writeToFile();

  while(query->next())
  {
    mDLine << query->value(2).toString(); // Symbol
    writeToFile();
  }

  mOLine << NewLine;

  if(verboseLevel(eAmple)) printStatus(eEffectOk, path);

  return true;
}

bool Exporter::expDepots()
{
  bool noSimulator = true;

  if(  mCmd->has("withSim")
    or mCmd->has("owner")
    or mCmd->has("dpid")   ) noSimulator = false;

  mFilu->setSqlParm(":owner", mCmd->optStr("owner"));
  mFilu->setSqlParm(":depotId", mCmd->optInt("dpid", -1));

  mDataText = "Depots";

  QSqlQuery* query = mFilu->execSql("GetDepots");

  if(badQuery(query)) return false;
  if(noData("No depots found.")) return true;

  // Filter out Simulator, or not
  QList<QSqlRecord> depots;
  while(query->next())
  {
    if(query->value(3).toString() == "Simulator" and noSimulator) continue;
    depots << query->record();
  }

  mDataR = depots.size();

  if(!depots.size())
  {
    printStatus(eEffectNote, "No depots found.");
    return true;
  }

  if(verboseLevel() != eInfo) // Prefer the detailed format
  {
    mOLine << "***";
    mOLine << "*";
    mOLine << "*";
    mOLine << "* Depot data - All data belonging to a depot are grouped";
    mOLine << "*";

    for(int i = 0; i < depots.size(); ++i)
    {
      mDataText    = "Depots";
      mDataR       = 1;
      QSqlRecord d = depots.at(i);
      QString depotInfo = d.value("Owner").toString() + " - " + d.value("Name").toString();

      mOLine << NewLine;
      mOLine << "[DepotOwner]" + d.value("Owner").toString();
      mOLine << "[DepotName]"  + d.value("Name").toString();
      mOLine << NewLine;
      mOLine << "*";
      mOLine << "* Depot: " + depotInfo;
      mOLine << "*";
      mOLine << "[Header]Trader;BrokerName";
      mOLine << NewLine;

      mDLine << d.value("Trader").toString();
      mDLine << d.value("Broker").toString();
      writeToFile();
      mOLine << NewLine;

      printStatus(eEffectOk, d.value("Owner").toString() + " - " + d.value("Name").toString());

      // positions
      mOLine << "*";
      mOLine << "* Position adds and outs for: " + depotInfo;
      mOLine << "*";
      mOLine << "[Header]PDate;RefSymbol;Market;Pieces;Price;Note";
      mOLine << NewLine;

      mDataText = "  Positions";
      mFilu->setSqlParm(":fiId", -1);
      mFilu->setSqlParm(":depotId", d.value("DepotId").toInt());

      query = mFilu->execSql("GetDepotPositions");
      if(!noData("No positions found."))
      {
        expPositions(query);
        printStatus(eEffectOk);
      }

      // account
      mOLine << "*";
      mOLine << "* Account postings for: " + depotInfo;
      mOLine << "*";
      mOLine << "[Header]APDate;APType;Value;Text";
      mOLine << NewLine;

      mDataText = "  Account";
      mFilu->setSqlParm(":accountId", -1);
      mFilu->setSqlParm(":depotId", d.value("DepotId").toInt());

      query = mFilu->execSql("GetAccount");
      if(!noData("No account postings found."))
      {
        expAccount(query);
        printStatus(eEffectOk);
      }

      // orders
      mOLine << "*";
      mOLine << "* Orders for: " + depotInfo;
      mOLine << "*";
      mOLine << "[Header]ODate;VDate;RefSymbol;Market;Pieces;Limit;Type;Status;Note";
      mOLine << NewLine;

      mDataText = "  Orders";
      mFilu->setSqlParm(":orderId", -1);
      mFilu->setSqlParm(":fiId", -1);
      mFilu->setSqlParm(":status", FiluU::eOrderActive);
      mFilu->setSqlParm(":depotId", d.value("DepotId").toInt());

      query = mFilu->execSql("GetDepotOrders");
      if(!noData("No orders found."))
      {
        expOrders(query);
        printStatus(eEffectOk);
      }
    }
  }
  else
  {
    // depot
    mOLine << "***";
    mOLine << "*";
    mOLine << "* Depot - Depot";
    mOLine << "*";
    mOLine << "[Header]DepotOwner;DepotName;Trader;BrokerName";
    mOLine << NewLine;

    for(int i = 0; i < depots.size(); ++i)
    {
      QSqlRecord d = depots.at(i);
      mDLine << d.value("Owner").toString();
      mDLine << d.value("Name").toString();
      mDLine << d.value("Trader").toString();
      mDLine << d.value("Broker").toString();
      writeToFile();
    }

    mOLine << NewLine;
    printStatus(eEffectOk);

    // positions
    mOLine << "***";
    mOLine << "*";
    mOLine << "* Depot - Position adds and outs";
    mOLine << "*";
    mOLine << "[Header]PDate;RefSymbol;Market;Pieces;Price;Note";
    mOLine << NewLine;

    mDataText = "Positions";
    mFilu->setSqlParm(":fiId", -1);
    bool noDataFound = true;
    for(int i = 0; i < depots.size(); ++i)
    {
      QSqlRecord d = depots.at(i);
      mFilu->setSqlParm(":depotId", d.value("DepotId").toInt());

      mOLine << "[DepotOwner]" + d.value("Owner").toString();
      mOLine << "[DepotName]"  + d.value("Name").toString();
      mOLine << NewLine;

      query = mFilu->execSql("GetDepotPositions");
      if(noData())
      {
        mOLine << "* No positions found";
        mOLine << NewLine;
        continue;
      }

      noDataFound = false;
      expPositions(query);
    }

    if(noDataFound)
    {
      printStatus(eEffectNote, "No positions found.");
    }
    else
    {
      printStatus(eEffectOk);
    }

    // account
    mOLine << "***";
    mOLine << "*";
    mOLine << "* Depot - Account postings";
    mOLine << "*";
    mOLine << "[Header]APDate;APType;Value;Text";
    mOLine << NewLine;

    mDataText = "Account";
    mFilu->setSqlParm(":accountId", -1);
    noDataFound = true;
    for(int i = 0; i < depots.size(); ++i)
    {
      QSqlRecord d = depots.at(i);
      mFilu->setSqlParm(":depotId", d.value("DepotId").toInt());

      mOLine << "[DepotOwner]" + d.value("Owner").toString();
      mOLine << "[DepotName]"  + d.value("Name").toString();
      mOLine << NewLine;

      query = mFilu->execSql("GetAccount");
      if(noData())
      {
        mOLine << "* No account positions found";
        mOLine << NewLine;
        continue;
      }

      noDataFound = false;
      expAccount(query);
    }

    if(noDataFound)
    {
      printStatus(eEffectNote, "No account postings found.");
    }
    else
    {
      printStatus(eEffectOk);
    }

    // orders
    mOLine << "***";
    mOLine << "*";
    mOLine << "* Depot - Orders";
    mOLine << "* (.)+";
    mOLine << "*";
    mOLine << "[Header]ODate;VDate;RefSymbol;Market;Pieces;Limit;Type;Status;Note";
    mOLine << NewLine;

    mDataText = "Orders";
    mFilu->setSqlParm(":orderId", -1);
    mFilu->setSqlParm(":fiId", -1);
    mFilu->setSqlParm(":status", FiluU::eOrderActive);
    noDataFound = true;
    for(int i = 0; i < depots.size(); ++i)
    {
      QSqlRecord d = depots.at(i);
      mFilu->setSqlParm(":depotId", d.value("DepotId").toInt());

      mOLine << "[DepotOwner]" + d.value("Owner").toString();
      mOLine << "[DepotName]"  + d.value("Name").toString();
      mOLine << NewLine;

      query = mFilu->execSql("GetDepotOrders");
      if(noData())
      {
        mOLine <<  "* No orders found";
        mOLine << NewLine;
        continue;
      }

      noDataFound = false;
      expOrders(query);
    }

    if(noDataFound)
    {
      printStatus(eEffectNote, "No orders found.");
    }
    else
    {
      printStatus(eEffectOk);
    }
  }

  return true;
}

void Exporter::expPositions(QSqlQuery* pos)
{
  while(pos->next())
  {
    QSqlRecord p = pos->record();
    mDLine << p.value("Date").toDate().toString(Qt::ISODate);
    mDLine << p.value("Symbol").toString();
    mDLine << p.value("Market").toString();
    mDLine << p.value("Pieces").toString();
    mDLine << p.value("Price").toString();
    mDLine << p.value("Note").toString();
    writeToFile();
  }

  mOLine << NewLine;
}

void Exporter::expAccount(QSqlQuery* acc)
{
  while(acc->next())
  {
    QSqlRecord a = acc->record();
    mDLine << a.value("Date").toDate().toString(Qt::ISODate);
    mDLine << mFilu->accPostingType(a.value("Type").toInt());
    mDLine << a.value("Value").toString();
    mDLine << a.value("Text").toString();
    writeToFile();
  }

  mOLine << NewLine;
}

void Exporter::expOrders(QSqlQuery* orders)
{
  while(orders->next())
  {
    QSqlRecord o = orders->record();
    mDLine << o.value("ODate").toDate().toString(Qt::ISODate);
    mDLine << o.value("VDate").toDate().toString(Qt::ISODate);
    mDLine << o.value("Symbol").toString();
    mDLine << o.value("Market").toString();
    mDLine << o.value("Pieces").toString();
    mDLine << o.value("Limit").toString();
    mDLine << mFilu->orderType(o.value("Buy").toInt());
    mDLine << mFilu->orderStatus(o.value("Status").toInt());
    mDLine << o.value("Note").toString();
    writeToFile();
  }

  mOLine << NewLine;
}
