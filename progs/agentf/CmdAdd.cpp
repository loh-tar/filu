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

#include "CmdAdd.h"

#include "Importer.h"
#include "FTool.h"
#include "CmdHelper.h"

CmdAdd::CmdAdd(FClass* parent)
      : FClass(parent, FUNC)
      , mImporter(new Importer(this))
{}

CmdAdd::~CmdAdd()
{
  delete mImporter;
}

void CmdAdd::briefIn(CmdHelper* cmd)
{
  if(!cmd) return;

  static const QString cCmd1 = "add";
  static const QString cCmd1Brief = tr("Allows you add a single dataset to the database by using Importer");

  cmd->inCmdBrief(cCmd1, cCmd1Brief);
}

bool CmdAdd::exec(CmdHelper* ch)
{
  if(!ch)
  {
    fatal(FUNC, "Called with NULL pointer.");
    return false;
  }

  mCmd = ch;
  mCmd->setUp("PrintCmdBriefCompact");
  mCmd->regSubCmds("broker eodBar fi market split symbol symbolType depot depotPos post order underlying");
  mCmd->regOpts("dp dpid");

  if(mCmd->subCmdLooksBad()) return false;

  if(mCmd->wantHelp())
  {
    mCmd->inOptBrief("dpid", "<DepotId>"
                   , tr("The serial number of the depot is shown by 'depots lsd' command"));
    mCmd->inOptBrief("dp", "<DepotName> <DepotOwner>"
                   , tr("Pretty human friedly to read"));

    mCmd->inLabel("DataType", tr("data types"));
  }

  if(mCmd->needHelp(2))
  {
    if(mCmd->printThisWay("<DataType> <ParmList>")) return !hasError();

    mCmd->printComment(tr("See also doc/importer-file-format.txt for background info."));
    mCmd->printNote(tr("The quality of added data is always considered as Platinum."));
    mCmd->printForInst("fi \"Apple Inc.\" Stock AAPL NYSE Yahoo US0378331005 NoMarket ISIN");
    mCmd->aided();
    return !hasError();
  }

  mImporter->setVerboseLevel(verboseLevel());

  // Look for each known command and call the related function
  if(mCmd->hasSubCmd("broker"))            addBroker();
  else if(mCmd->hasSubCmd("eodBar"))       addEodBar();
  else if(mCmd->hasSubCmd("fi"))           addFi();
  else if(mCmd->hasSubCmd("market"))       addMarket();
  else if(mCmd->hasSubCmd("split"))        addSplit();
  else if(mCmd->hasSubCmd("symbol"))       addSymbol();
  else if(mCmd->hasSubCmd("symbolType"))   addSymbolType();
  else if(mCmd->hasSubCmd("depot"))        addDepot();
  else if(mCmd->hasSubCmd("depotPos"))     addDepotPos();
  else if(mCmd->hasSubCmd("post"))         addAccPosting();
  else if(mCmd->hasSubCmd("order"))        addOrder();
  else if(mCmd->hasSubCmd("underlying"))   addUnderlyg();
  else
  {
    fatal(FUNC, QString("Unsupported command: %1").arg(mCmd->cmd()));
  }

  return !hasError();
}
/*
void CmdAdd::printDataTypes()
{
  QStringList cmds;
  cmds << "broker"<< "depot" << "depotPos" << "eodBar" << "fi" << "market"
       << "order" << "post" << "split" << "symbol" << "symbolType"
       << "underlying";

  print(tr("\nPossible data types are:\n"));
  int cols = 3;
  for(int i = 0; i < cmds.size(); i += cols)
  {
    QString line;
    for(int j = 0; (j < cols) and ((i + j) < cmds.size()); ++j)
    {
      line.append(QString("%1").arg(cmds.at(i + j), -20));
    }
    print(QString("  %1").arg(line));
  }

  print("");
}*/

void CmdAdd::import()
{
  addErrors(mCmd->errors());

  mHeader << "Quality";
  mData   << "Platinum";

  if(!hasError())
  {
    const QString header = mHeader.join(";");
    const QString data   = mData.join(";");

    verbose(FUNC, header, eAmple);
    if(mImporter->import(header))
    {
      verbose(FUNC, data, eAmple);
      mImporter->import(data);
    }

    addErrors(mImporter->errors());
  }

  mHeader.clear();
  mData.clear();
}

void CmdAdd::addEodBar()
{
  mCmd->regOpts("vol oi");
  mCmd->regOptsOneOfIsMandatory("BarData", "close ohlc");

  if(mCmd->isMissingParms(3))
  {
    mCmd->inOptBrief("close", "<Close>", tr("To use where only a close price is available"));
    mCmd->inOptBrief("ohlc", "<Open> <High> <Low> <Close>", tr("The prefered full eodBar set"));
    mCmd->inOptBrief("vol", "<Volume>", tr("Should always given when available"));
    mCmd->inOptBrief("oi", "<OpenInterest>", tr("Currently useless because bad supported by Filu"));

    if(mCmd->printThisWay("<Date> <RefSymbol> <Market> <BarData>"
                          "[~~vol] [~~oi]")) return;

    mCmd->printForInst("2010-04-01 AAPL NYSE --close 237.41 --vol 21540900");
    mCmd->printForInst("2010-04-01 AAPL NYSE --ohlc 237.41 238.73 232.75 235.97");
    mCmd->aided();
    return;
  }

  mHeader << "[Header]Date" << "RefSymbol" << "Market";
  mData   << mCmd->strParmDate(1) << mCmd->strParm(2) << mCmd->strParm(3);

  mHeader << "Open;High;Low;Close";

  if(mCmd->has("close"))
  {
    // We have to use 4 times the close price
    // otherwise the database will set the quality to Tin
    mData   << mCmd->strParmDouble(1) << mCmd->strParmDouble(1)
            << mCmd->strParmDouble(1) << mCmd->strParmDouble(1);
  }
  else //if(mCmd->has("ohlc"))
  {
    mData   << mCmd->strParmDouble(1) << mCmd->strParmDouble(2)
            << mCmd->strParmDouble(3) << mCmd->strParmDouble(4);
  }

  if(mCmd->has("vol"))
  {
    mHeader << "Volume";
    mData   << mCmd->strParmDouble(1);
  }

  if(mCmd->has("oi"))
  {
    mHeader << "OpenInterest";
    mData   << mCmd->strParmInt(1);
  }

  import();
}

void CmdAdd::addBroker()
{
  if(mCmd->isMissingParms(3))
  {
    if(mCmd->printThisWay("<BrokerName> <CurrencySymbol> <FeeFormula>")) return;

    mCmd->printNote("Please encapsulate the fee formula in double quotes."); // FIXME Should be done better
    mCmd->printForInst("MyBank USD \"5.95 + OV * 0.001\"");
    mCmd->aided();
    return;
  }

  mHeader << "[Header]BrokerName" << "CurrencySymbol" << "FeeFormula";
  mData   << mCmd->strParm(1) << mCmd->strParm(2) << mCmd->strParm(3);

  import();
}

void CmdAdd::addFi()
{
  if(!mCmd->wantHelp() and (mCmd->parmCount() - 2) % 3)
  {
    error(FUNC, tr("Wrong count of <Symbol> <Market> <Provider>."));
  }

  if(mCmd->isMissingParms(5) or hasError())
  {
    if(mCmd->printThisWay("<Name> <Type> <Symbol> <Market> <Provider> \\ "
                          "[<Symbol> <Market> <Provider>]..")) return;

    mCmd->printForInst("\"Apple Inc.\" Stock AAPL NYSE Yahoo US0378331005 NoMarket ISIN");
    mCmd->aided();
    return;
  }

  mHeader << "[Header]Name" << "Type";
  mData   << mCmd->parmList();

  int symbolCount = (mCmd->parmCount() - 2) / 3;
  for(int i = 0; i < symbolCount; ++i) mHeader << "Symbol" << "Market" << "Provider";

  import();
}

void CmdAdd::addMarket()
{
  mCmd->regOpts("curr");

  if(mCmd->isMissingParms(2))
  {
    mCmd->inOptBrief("curr", "<CurrencyName>"
                   , tr("The currency name is *not* updated in case that she is already known"));

    if(mCmd->printThisWay("<Market> <CurrencySymbol> [~~curr]")) return;

    mCmd->printForInst("NYSE USD --curr \"US Dollar\"");
    mCmd->aided();
    return;
  }

  mHeader << "[Header]Market" << "CurrencySymbol";
  mData   << mCmd->strParm(1) << mCmd->strParm(2);

  if(mCmd->has("curr"))
  {
    mHeader << "Currency";
    mData   << mCmd->strParm(1);
  }

  import();
}

void CmdAdd::addSplit()
{
  if(mCmd->isMissingParms(3))
  {
    if(mCmd->printThisWay("<RefSymbol> <SplitDate> <SplitPre:Post>")) return;

    mCmd->printForInst("AAPL 2005-02-28 1:2");
    mCmd->aided();
    return;
  }

  mHeader << "[Header]RefSymbol;SplitDate;SplitPre:Post";
  mData   << mCmd->parmList(); // FIXME check split

  import();
}

void CmdAdd::addSymbol()
{
  if(mCmd->isMissingParms(4))
  {
    if(mCmd->printThisWay("<RefSymbol> <Symbol> <Market> <Provider>")) return;

    mCmd->printForInst("AAPL US0378331005 NoMarket ISIN");
    mCmd->aided();
    return;
  }

  mHeader << "[Header]RefSymbol" << "Symbol" << "Market" << "Provider";
  mData   << mCmd->parmList();

  import();
}

void CmdAdd::addSymbolType()
{
  if(mCmd->isMissingParms(3))
  {
    if(mCmd->printThisWay("<SymbolType> <IsProvider> <SEQ>")) return;

    mCmd->printForInst("ISIN false 1100");
    mCmd->aided();
    return;
  }

  mHeader << "[Header]SymbolType" << "IsProvider" << "SEQ";
  mData   << mCmd->strParm(1) << mCmd->strParmBool(2) << mCmd->strParmInt(3);

  import();
}

void CmdAdd::addOrder()
{
  mCmd->makeOneOfOptsMandatory("Depot", "dpid dp");

  if(mCmd->isMissingParms(9))
  {
    if(mCmd->printThisWay("<ODate> <VDate> <RefSymbol> <Market> <Pieces> <Limit> <Type> "
                          "<Status> <Note> \\ <Depot>")) return;

    mCmd->printForInst("2010-09-01 2010-09-01 AAPL NYSE 10 Best Buy "
                       "Active \"It looks so good\" --dp SlowHand Me");
    mCmd->aided();
    return;
  }

  mHeader << "[Header]ODate" << "VDate" << "RefSymbol" << "Market";
  mData   << mCmd->strParmDate(1) << mCmd->strParmDate(2) << mCmd->strParm(3) << mCmd->strParm(4);

  mHeader << "Pieces" << "Limit" << "Type" << "Status" << "Note";
  mData   << mCmd->strParmInt(5) << mCmd->strParmDouble(6) << mCmd->strParm(7)
          << mCmd->strParm(8) << mCmd->strParm(9);

  takeDepotOptions();

  import();
}

void CmdAdd::addDepot()
{
  if(mCmd->isMissingParms(4))
  {
    if(mCmd->printThisWay("<DepotName> <DepotOwner> <Trader> <BrokerName>")) return;

    mCmd->printForInst("SlowHand Me Watchdog MyBank");
    mCmd->aided();
    return;
  }

  mHeader << "[Header]DepotName" << "DepotOwner" << "Trader" << "BrokerName";
  mData   << mCmd->parmList();

  import();
}

void CmdAdd::addDepotPos()
{
  mCmd->makeOneOfOptsMandatory("Depot", "dpid dp");

  if(mCmd->isMissingParms(6))
  {
    if(mCmd->printThisWay("<PDate> <RefSymbol> <Market> <Pieces> <Price> <Note> \\ "
                          "<Depot>")) return;

    mCmd->printForInst("2010-09-01 AAPL NYSE 10 247.47 \"It looked so good\" --dp SlowHand Me");
    mCmd->aided();
    return;
  }

  mHeader << "[Header]PDate" << "RefSymbol" << "Market";
  mData   << mCmd->strParmDate(1) << mCmd->strParm(2) << mCmd->strParm(3);

  mHeader << "Pieces" << "Price" << "Note";
  mData   << mCmd->strParmInt(4) << mCmd->strParmDouble(5) << mCmd->strParm(6);

  takeDepotOptions();

  import();
}

void CmdAdd::addAccPosting()
{
  mCmd->makeOneOfOptsMandatory("Depot", "dpid dp");

  if(mCmd->isMissingParms(4))
  {
    if(mCmd->printThisWay("<APDate> <APType> <Text> <Value> \\ <Depot>")) return;

    mCmd->printForInst("2010-09-01 FiBuy \"10x Apple at 247.47\" 2474.70 --dp SlowHand Me");
    mCmd->aided();
    return;
  }

  mHeader << "[Header]APDate" << "APType" << "Text" << "Value";
  mData   << mCmd->strParmDate(1) << mCmd->strParm(2) << mCmd->strParm(3) << mCmd->strParmDouble(4);

  takeDepotOptions();

  import();
}

void CmdAdd::addUnderlyg()
{
  if(mCmd->isMissingParms(3))
  {
    if(mCmd->printThisWay("<Mother> <RefSymbol> <Weight>")) return;

    mCmd->printForInst("^NDX AAPL 1");
    mCmd->aided();
    return;
  }

  mHeader << "[Header]Mother" << "RefSymbol" << "Weight";
  mData   << mCmd->strParm(1) << mCmd->strParm(2) << mCmd->strParmDouble(3);

  import();
}

void CmdAdd::takeDepotOptions()
{
  if(mCmd->has("dpid"))
  {
    mHeader << "DepotId";
    mData   << mCmd->strParmInt(1);
  }
  else //if(mCmd->has("dp"))
  {
    if(mCmd->parmCount() > 1)
    {
      mHeader << "DepotName" << "DepotOwner";
      mData   << mCmd->strParm(1) << mCmd->strParm(2);
    }
    else
    {
      error(FUNC, tr("Option --dp need 2 parameter but has %1").arg(mCmd->parmCount()));
    }
  }
}
