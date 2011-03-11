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

#include "CmdAdd.h"

#include "Importer.h"
#include "FTool.h"

CmdAdd::CmdAdd(FClass* parent)
      : FClass(parent, FUNC)
      , mImporter(new Importer(this))
      , mWantHelp(false)
{
  mInfoTxt.insert("WrongParmCount", tr("Wrong parameter count."));
  mInfoTxt.insert("TooLessArg", tr("Too less arguments."));
  mInfoTxt.insert("ThisWay", tr("Please call me this way:\n"));
  mInfoTxt.insert("ForInst", tr("\nFor instance:\n"));
}

CmdAdd::~CmdAdd()
{
  delete mImporter;
}

bool CmdAdd::exec(const QStringList& cmdLine)
{
  QStringList parm = cmdLine;
  if(parm.size() == 2) parm.append("help");     // Was only 'agentf add'
  if(parm.at(2) != "--help")
  {
    parm[2] = "--" + parm.at(2);                // Now we can use FTool::getParameter(...)
    if(parm.size() == 3) parm.append("--help"); // Was only 'agentf add foo' but no arguments
  }

  setVerboseLevel(FUNC, parm);
  //if(verboseLevel(eAmple)) mImporter->setVerboseLevel(eInfo);
  mImporter->setVerboseLevel(verboseLevel());

  mInfoTxt.insert("CmdPrefix", QString("  %1 add %2").arg(parm.at(0), "%1"));
  QString cmdPref2(mInfoTxt.value("CmdPrefix").size() - 2, ' ');
  cmdPref2.append("%1");
  mInfoTxt.insert("CmdPref++", cmdPref2);

  mWantHelp = parm.contains("--help") ? true : false;
  bool  needHelp = false;
  const QString cmd = parm.at(2);

  // Look for each known command and call the related function
  if(cmd == "--broker")            addBroker(parm);
  else if(cmd == "--eodBar")       addEodBar(parm);
  else if(cmd == "--fi")           addFi(parm);
  else if(cmd == "--market")       addMarket(parm);
  else if(cmd == "--split")        addSplit(parm);
  else if(cmd == "--symbol")       addSymbol(parm);
  else if(cmd == "--symbolType")   addSymbolType(parm);
  else if(cmd == "--depot")        addDepot(parm);
  else if(cmd == "--depotPos")     addDepotPos(parm);
  else if(cmd == "--post")         addAccPosting(parm);
  else if(cmd == "--order")        addOrder(parm);
  else if(cmd == "--underlying")   addUnderlyg(parm);
  else if(cmd == "--help")         needHelp = true;
  else
  {
    error(FUNC, tr("Unknown data type: %1").arg(cmd.right(cmd.size() - 2)));
    printDataTypes();
    return false;
  }

  if(needHelp)
  {
    print(mInfoTxt.value("ThisWay"));
    print(mInfoTxt.value("CmdPrefix").arg("<dataType> <parameterList> [--help] [--verbose <level>]"));
    printDataTypes();
    print(tr("Calling a data type without any parameter tells you more (the same as give --help)."));
    print(mInfoTxt.value("ForInst"));
    print(mInfoTxt.value("CmdPrefix").arg("fi"));
    print(tr("\nThe 'add' command uses Importer to add a single dataset to the database,"
             "\nso take a look at doc/importer-file-format.txt for background info."));
  }

  return !hasError();
}

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
}

void CmdAdd::import(const QString& header, const QString& data)
{
  verbose(FUNC, header, eAmple);
  if(!mImporter->import(header))
  {
    addErrors(mImporter->errors());
    return;
  }

  verbose(FUNC, data, eAmple);
  if(!mImporter->import(data))
  {
    addErrors(mImporter->errors());
    return;
  }
}

void CmdAdd::addEodBar(const QStringList& parm)
{
  if(!mWantHelp)
  {
    if(FTool::getParameter(parm, "--eodBar", mCmdArg) < 2)
    {
      error(FUNC, mInfoTxt.value("TooLessArg"));
      mWantHelp = true;
    }
  }

  if(mWantHelp)
  {
    print(mInfoTxt.value("ThisWay"));
    print(mInfoTxt.value("CmdPrefix").arg("eodBar <Date> <RefSymbol> <Market> [<Quality>]"));
    print(mInfoTxt.value("CmdPref++").arg("       --close <Close> | --ohlc <Open> <High> <Low> <Close>"));
    print(mInfoTxt.value("CmdPref++").arg("       [--vol <Volume>] [--oi <OpenInterest>]"));
    print(mInfoTxt.value("ForInst"));
    print(mInfoTxt.value("CmdPrefix").arg("eodBar 2010-04-01 AAPL NYSE --close 237.41 --vol 21540900"));
    print(mInfoTxt.value("CmdPrefix").arg("eodBar 2010-04-01 AAPL NYSE --ohlc 237.41 238.73 232.75 235.97"));
    return;
  }

  QString header = "[Header]Date;RefSymbol;Market";

  if(mCmdArg.size() > 3) header.append(";Quality");

  QString data = mCmdArg.join(";");

  if(FTool::getParameter(parm, "--close", mCmdArg) > 0)
  {
    header.append(";Close");
    data.append(";" + mCmdArg.at(0));
  }

  if(FTool::getParameter(parm, "--ohlc", mCmdArg) > 0)
  {
    header.append(";Open;High;Low;Close");
    data.append(";" + mCmdArg.join(";"));
  }

  if(FTool::getParameter(parm, "--vol", mCmdArg) > 0)
  {
    header.append(";Volume");
    data.append(";" + mCmdArg.at(0));
  }

  if(FTool::getParameter(parm, "--oi", mCmdArg) > 0)
  {
    header.append(";OpenInterest");
    data.append(";" + mCmdArg.at(0));
  }

  import(header, data);
}

void CmdAdd::addBroker(const QStringList& parm)
{
  if(!mWantHelp)
  {
    if(FTool::getParameter(parm, "--broker", mCmdArg) < 2)
    {
      error(FUNC, mInfoTxt.value("TooLessArg"));
      mWantHelp = true;
    }
  }

  if(mWantHelp)
  {
    print(mInfoTxt.value("ThisWay"));
    print(mInfoTxt.value("CmdPrefix").arg("broker <BrokerName> <FeeFormula> [<Quality>]"));
    print(mInfoTxt.value("ForInst"));
    print(mInfoTxt.value("CmdPrefix").arg("broker MyBank \"5.95 + OV * 0.001\""));
    return;
  }

  QString header = "[Header]BrokerName;FeeFormula";

  if(mCmdArg.size() > 2) header.append(";Quality");

  import(header, mCmdArg.join(";"));
}

void CmdAdd::addFi(const QStringList& parm)
{
  if(!mWantHelp)
  {
    if(((FTool::getParameter(parm, "--fi", mCmdArg) - 2) % 3 > 0) or parm.count() < 7)
    {
      error(FUNC, mInfoTxt.value("WrongParmCount"));
      mWantHelp = true;
    }
  }

  if(mWantHelp)
  {
    print(mInfoTxt.value("ThisWay"));
    print(mInfoTxt.value("CmdPrefix").arg("fi <Name> <Type> <Symbol> <Market> <Provider> [<Symbol> <Market> <Provider> ...]"));
    print(mInfoTxt.value("ForInst"));
    print(mInfoTxt.value("CmdPrefix").arg("fi \"Apple Inc.\" Stock AAPL NYSE Yahoo US0378331005 NoMarket ISIN"));
    return;
  }

  QString header = "[Header]Name;Type";
  int symbolCount = (mCmdArg.count() - 2) / 3;
  for(int i = 0; i < symbolCount; ++i) header.append(";Symbol;Market;Provider");

  import(header, mCmdArg.join(";"));
}

void CmdAdd::addMarket(const QStringList& parm)
{
  if(!mWantHelp)
  {
    if(FTool::getParameter(parm, "--market", mCmdArg) < 2)
    {
      error(FUNC, mInfoTxt.value("TooLessArg"));
      mWantHelp = true;
    }
  }

  if(mWantHelp)
  {
    print(mInfoTxt.value("ThisWay"));
    print(mInfoTxt.value("CmdPrefix").arg("market <Market> <CurrencySymbol> [<Quality>] [--curr <CurrencyName>]"));
    print(mInfoTxt.value("ForInst"));
    print(mInfoTxt.value("CmdPrefix").arg("market NYSE USD --curr \"US Dollar\""));
    return;
  }

  QString header = "[Header]Market;CurrencySymbol";

  if(mCmdArg.size() > 2) header.append(";Quality");

  QString data = mCmdArg.join(";");

  if(FTool::getParameter(parm, "--curr", mCmdArg) > 0)
  {
    header.append(";Currency");
    data.append(";" + mCmdArg.at(0));
  }

  import(header, data);
}

void CmdAdd::addSplit(const QStringList& parm)
{
  if(!mWantHelp)
  {
    if(FTool::getParameter(parm, "--split", mCmdArg) < 3)
    {
      error(FUNC, mInfoTxt.value("TooLessArg"));
      mWantHelp = true;
    }
  }

  if(mWantHelp)
  {
    print(mInfoTxt.value("ThisWay"));
    print(mInfoTxt.value("CmdPrefix").arg("split <RefSymbol> <SplitDate> <SplitPre:Post> [<Quality>]"));
    print(mInfoTxt.value("ForInst"));
    print(mInfoTxt.value("CmdPrefix").arg("split AAPL 2005-02-28 1:2"));
    return;
  }

  QString header = "[Header]RefSymbol;SplitDate;SplitPre:Post";

  if(mCmdArg.size() > 3) header.append(";Quality");

  import(header, mCmdArg.join(";"));
}

void CmdAdd::addSymbol(const QStringList& parm)
{
  if(!mWantHelp)
  {
    if(FTool::getParameter(parm, "--symbol", mCmdArg) < 4)
    {
      error(FUNC, mInfoTxt.value("TooLessArg"));
      mWantHelp = true;
    }
  }

  if(mWantHelp)
  {
    print(mInfoTxt.value("ThisWay"));
    print(mInfoTxt.value("CmdPrefix").arg("symbol <RefSymbol> <Symbol> <Market> <Provider> [<Quality>]"));
    print(mInfoTxt.value("ForInst"));
    print(mInfoTxt.value("CmdPrefix").arg("symbol AAPL US0378331005 NoMarket ISIN"));
    return;
  }

  QString header = "[Header]RefSymbol;Symbol;Market;Provider";

  if(mCmdArg.size() > 4) header.append(";Quality");

  import(header, mCmdArg.join(";"));
}

void CmdAdd::addSymbolType(const QStringList& parm)
{
  if(!mWantHelp)
  {
    if(FTool::getParameter(parm, "--symbolType", mCmdArg) < 3)
    {
      error(FUNC, mInfoTxt.value("TooLessArg"));
      mWantHelp = true;
    }
  }

  if(mWantHelp)
  {
    print(mInfoTxt.value("ThisWay"));
    print(mInfoTxt.value("CmdPrefix").arg("symbolType <SymbolType> <IsProvider> <SEQ> [<Quality>]"));
    print(mInfoTxt.value("ForInst"));
    print(mInfoTxt.value("CmdPrefix").arg("symbolType ISIN false 1100"));
    return;
  }

  QString header = "[Header]SymbolType;IsProvider;SEQ";

  if(mCmdArg.size() > 3) header.append(";Quality");

  import(header, mCmdArg.join(";"));
}

void CmdAdd::addOrder(const QStringList& parm)
{
  QStringList dp;

  if(!mWantHelp)
  {
    if(FTool::getParameter(parm, "--order", mCmdArg) < 9)
    {
      error(FUNC, mInfoTxt.value("TooLessArg"));
      mWantHelp = true;
    }
    else if(FTool::getParameter(parm, "--dp", dp) < 1)
    {
      error(FUNC, mInfoTxt.value("TooLessArg"));
      mWantHelp = true;
    }
  }

  if(mWantHelp)
  {
    print(mInfoTxt.value("ThisWay"));
    print(mInfoTxt.value("CmdPrefix").arg("order <ODate> <VDate> <RefSymbol> <Market> <Pieces> <Limit> <Type> <Status> <Note> [<Quality>]"));
    print(mInfoTxt.value("CmdPref++").arg("     --dp <DepotId> | <DepotName> <DepotOwner>"));
    print(mInfoTxt.value("ForInst"));
    print(mInfoTxt.value("CmdPrefix").arg("order 2010-09-01 2010-09-01 AAPL NYSE 10 Best Buy Active \"It looks so good\" --dp SlowHand Me"));
    return;
  }

  QString header = "[Header]ODate;VDate;RefSymbol;Market;Pieces;Limit;Type;Status;Note";

  if(mCmdArg.size() > 9) header.append(";Quality");

  if(dp.size() > 1) header.append(";DepotName;DepotOwner");
  else header.append(";DepotId");

  mCmdArg << dp;
  import(header, mCmdArg.join(";"));
}

void CmdAdd::addDepot(const QStringList& parm)
{
  if(!mWantHelp)
  {
    if(FTool::getParameter(parm, "--depot", mCmdArg) < 5)
    {
      error(FUNC, mInfoTxt.value("TooLessArg"));
      mWantHelp = true;
    }
  }

  if(mWantHelp)
  {
    print(mInfoTxt.value("ThisWay"));
    print(mInfoTxt.value("CmdPrefix").arg("depot <DepotName> <DepotOwner> <Trader> <CurrencySymbol> <BrokerName> [<Quality>]"));
    print(mInfoTxt.value("ForInst"));
    print(mInfoTxt.value("CmdPrefix").arg("depot SlowHand Me Watchdog USD MyBank"));
    return;
  }

  QString header = "[Header]DepotName;DepotOwner;Trader;CurrencySymbol;BrokerName";

  if(mCmdArg.size() > 5) header.append(";Quality");

  import(header, mCmdArg.join(";"));
}

void CmdAdd::addDepotPos(const QStringList& parm)
{
  QStringList dp;

  if(!mWantHelp)
  {
    if(FTool::getParameter(parm, "--depotPos", mCmdArg) < 6)
    {
      error(FUNC, mInfoTxt.value("TooLessArg"));
      mWantHelp = true;
    }
    else if(FTool::getParameter(parm, "--dp", dp) < 1)
    {
      error(FUNC, mInfoTxt.value("TooLessArg"));
      mWantHelp = true;
    }
  }

  if(mWantHelp)
  {
    print(mInfoTxt.value("ThisWay"));
    print(mInfoTxt.value("CmdPrefix").arg("depotPos <PDate> <RefSymbol> <Market> <Pieces> <Price> <Note> [<Quality>]"));
    print(mInfoTxt.value("CmdPref++").arg("         --dp <DepotId> | <DepotName> <DepotOwner>"));
    print(mInfoTxt.value("ForInst"));
    print(mInfoTxt.value("CmdPrefix").arg("depotPos 2010-09-01 AAPL NYSE 10 247.47 \"It looked so good\" --dp SlowHand Me"));
    return;
  }

  QString header = "[Header]PDate;RefSymbol;Market;Pieces;Price;Note";

  if(mCmdArg.size() > 6) header.append(";Quality");

  if(dp.size() > 1) header.append(";DepotName;DepotOwner");
  else header.append(";DepotId");

  mCmdArg << dp;
  import(header, mCmdArg.join(";"));
}

void CmdAdd::addAccPosting(const QStringList& parm)
{
  QStringList dp;

  if(!mWantHelp)
  {
    if(FTool::getParameter(parm, "--post", mCmdArg) < 4)
    {
      error(FUNC, mInfoTxt.value("TooLessArg"));
      mWantHelp = true;
    }
    else if(FTool::getParameter(parm, "--dp", dp) < 1)
    {
      error(FUNC, mInfoTxt.value("TooLessArg"));
      mWantHelp = true;
    }
  }

  if(mWantHelp)
  {
    print(mInfoTxt.value("ThisWay"));
    print(mInfoTxt.value("CmdPrefix").arg("post <APDate> <APType> <Text> <Value> [<Quality>]"));
    print(mInfoTxt.value("CmdPref++").arg("     --dp <DepotId> | <DepotName> <DepotOwner>"));
    print(mInfoTxt.value("ForInst"));
    print(mInfoTxt.value("CmdPrefix").arg("post 2010-09-01 FiBuy \"10x Apple at 247.47\" 2474.70 --dp SlowHand Me"));
    return;
  }

  QString header = "[Header]APDate;APType;Text;Value";

  if(mCmdArg.size() > 4) header.append(";Quality");

  if(dp.size() > 1) header.append(";DepotName;DepotOwner");
  else header.append(";DepotId");

  mCmdArg << dp;
  import(header, mCmdArg.join(";"));
}

void CmdAdd::addUnderlyg(const QStringList& parm)
{
  if(!mWantHelp)
  {
    if(FTool::getParameter(parm, "--underlying", mCmdArg) < 3)
    {
      error(FUNC, mInfoTxt.value("TooLessArg"));
      mWantHelp = true;
    }
  }

  if(mWantHelp)
  {
    print(mInfoTxt.value("ThisWay"));
    print(mInfoTxt.value("CmdPrefix").arg("underlying <Mother> <RefSymbol> <Weight> [<Quality>]"));
    print(mInfoTxt.value("ForInst"));
    print(mInfoTxt.value("CmdPrefix").arg("underlying ^NDX AAPL 1"));
    return;
  }

  QString header = "[Header]Mother;RefSymbol;Weight";

  if(mCmdArg.size() > 3) header.append(";Quality");

  import(header, mCmdArg.join(";"));
}
