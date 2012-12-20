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

// #include <QDebug>
#include "CmdHelper.h"
#include "FTool.h"

const QString cCommands       = QObject::tr("commands");
const QString cCommand        = QObject::tr("Command");
const QString cNote           = QObject::tr("Note");
const QString cParmIsNot      = QObject::tr("Parameter %1 of %2 is not %3: %4");
const QString cMissingArg     = QObject::tr("%1 is missing %2 argument(s).");

const QString cCmdPCNotValid = "mInqCmd.parmCount is not valid. You have to check isMissingParms().";

QString cmdHeader(const QString& cmds = cCommands)
{
  return QObject::tr("Available %1 are:").arg(cmds);
}

QString optHeader(const QString& name = QObject::tr("options"))
{
  return QObject::tr("Possible %1 (hyphens are omitted) are:").arg(name);
}

QSet<QString> stripOptions(const QString& opts)
{
  return QSet<QString>::fromList(opts.split(" ", QString::SkipEmptyParts));
}

QSet<QString> availPublicSet()
{
  QSet<QString> set;
  set << "PrintCmdBriefCompact";
  return set;
}

QSet<QString> allKnownSet()
{
  QSet<QString> set;
  set << "WantHelp" << "Help+" << "HelpHidden" << "FirstTimeForInst";
  set.unite(availPublicSet());
  return set;
}

CmdHelper::CmdHelper(Newswire* parent)
         : Newswire(parent, FUNC)
         , mAvailPublicSet(availPublicSet())
         , mAllKnownSet(allKnownSet())
         , mHelpLevel(1)
         , mScreenWidth(100)        // FIXME Obtain the real terminal width
{}

CmdHelper::~CmdHelper()
{}

bool CmdHelper::needHelp(int level)
{
  if(mHelpLevel != level) return false;

  initPrinting();
  return true;
}

void CmdHelper::setUp(const QString& opt)
{
  if(!mAvailPublicSet.contains(opt))
    fatal(FUNC, QString("Try to setUp unknown public setting: %1").arg(opt));

  set(opt);
}

void CmdHelper::set(const QString& opt)
{
  if(!mAllKnownSet.contains(opt))
    fatal(FUNC, QString("Try to set unknown setting: %1").arg(opt));

  mSet.insert(opt);
}

bool CmdHelper::isSet(const QString& opt)
{
  if(!mAllKnownSet.contains(opt))
    fatal(FUNC, QString("Asked for unknown setting: %1").arg(opt));

  return mSet.contains(opt);
}

void CmdHelper::unSet(const QString& opt)
{
  if(!mAllKnownSet.contains(opt))
    fatal(FUNC, QString("Try to unset unknown setting: %1").arg(opt));

  mSet.remove(opt);
}

void CmdHelper::aided()
{
  if(isSet("HelpHidden"))
    printNote(tr("Some help was hidden. To show full help use --help+ or -h+"));

  print("");
}

bool CmdHelper::cmdLineLooksBad(const QStringList& cmdLine)
{
  // At first (1st Level) we expect a command without subcommand like:
  //   <Prog> <Command> [<ParmList>] [<OptList>]
  //
  // Returns false if <Command> is Ok or --help was given otherwise true.

  if(!mCmds.size())
  {
    fatal(FUNC, "No commands registered.");
    return true;
  }

  mOptions.clear();
  mSubCmds.clear();
  mMandatory.clear();
  mCmdLine   = cmdLine;
  mHelpLevel = 1;
  unSet("WantHelp");

  // Can't call if(has("verbose")) at this point
  if(mCmdLine.contains("--verbose")) setVerboseLevel(FUNC, mCmdLine);
  setMsgTargetFormat(eFileLog, "%D %T %C: *** %t *** %c: %x");

  // Find any help request and remove it from mCmdLine
  int helpPos = mCmdLine.indexOf("-h+");
  if(helpPos < 0) helpPos = mCmdLine.indexOf("--help+");
  if(helpPos > 0) set("Help+");

  if(helpPos < 0) helpPos = mCmdLine.indexOf("-h");
  if(helpPos < 0) helpPos = mCmdLine.indexOf("--help");

  if(helpPos < 0)
  {
    if(mCmdLine.size() > 1)
    {
      if("help" == mCmdLine.at(1)) helpPos = 1;
      else if("help+" == mCmdLine.at(1))
      {
        helpPos = 1;
        set("Help+");
      }
    }

    if(mCmdLine.size() > 2)
    {
      if("help" == mCmdLine.at(2)) helpPos = 2;
      else if("help+" == mCmdLine.at(2))
      {
        helpPos = 2;
        set("Help+");
      }
    }
  }

  if(helpPos > 0)
  {
    set("WantHelp");
    mCmdLine.removeAt(helpPos);
  }

  if(mCmdLine.size() < 2)
  {
    if(wantHelp()) return false;

    // Called without <Command>, only "<Prog>"
    if(mLabel.contains("<>Greeter<>")) print(mLabel.value("<>Greeter<>"));
    print(tr("Try: %1 -h [<%2>]").arg(mCmdLine.at(0), cCommand));

    printCommands(mCmds);
    return true;
  }

  QString cmd = mCmdLine.at(1);

  if(!cmd.startsWith("--"))
  {
    mCmdLine[1] = "--" + cmd; // Make sure mCmdLine has "--"
  }
  else
  {
    cmd = cmd.mid(2);
  }

  if(!mCmds.contains(cmd))
  {
    if(wantHelp()) return false;

    error(FUNC, tr("Unknown command: %1").arg(cmd));
    print(tr("Try: %1 [<%2>] -h").arg(mCmdLine.at(0), cCommand));
    printCommands(mCmds);
    return true;
  }

  mHelpLevel = 2;

  // Memorize all positions and parameter counts
  Option opt;
  opt.idxPos = 1;
  opt.parmCount = 0;
  opt.name = mCmdLine.at(1).mid(2);
  for(int i = 2; i < mCmdLine.size(); ++i)
  {
    if(mCmdLine.at(i).startsWith("--"))
    {
      mOptions.insert(opt.name, opt);

      opt.idxPos = i;
      opt.name = mCmdLine.at(i).mid(2);
      opt.parmCount = 0;
    }
    else
    {
      ++opt.parmCount;
    }
  }
  mOptions.insert(opt.name, opt);

  return false;
}

bool CmdHelper::has(const QString& opt)
{
  // Don't change mInqOpt, test only if exist

  if(mOptions.has(opt)) return true;
  if(!verboseLevel(eAmple)) return false;

  if(!mOpts.contains(opt) and !mSubCmds.contains(opt))
    fatal(FUNC, QString("Test for unregistered option or subcommand: %1").arg(opt));

  return false;
}

bool CmdHelper::hasCmd(const QString& cmd)
{
  mInqOpt = mOptions.named(cmd);

  if(mInqOpt.idxPos > -1)
  {
    mInqCmd = mInqOpt;
    return true;
  }

  if(!verboseLevel(eAmple)) return false;
  if(!mCmds.contains(cmd)) fatal(FUNC, QString("Test for unregistered command: %1").arg(cmd));

  return false;
}

bool CmdHelper::hasSubCmd(const QString& cmd)
{
  mInqOpt = mOptions.named(cmd);

  if(mInqOpt.idxPos > -1)
  {
    mInqSubCmd = mInqOpt;
    return true;
  }

  if(!verboseLevel(eAmple)) return false;
  if(!mSubCmds.contains(cmd)) fatal(FUNC, QString("Test for unregistered subcommand: %1").arg(cmd));

  return false;
}

bool CmdHelper::hasOpt(const QString& opt)
{
  mInqOpt = mOptions.named(opt);

  if(mInqOpt.idxPos > -1) return true;
  if(!verboseLevel(eAmple)) return false;
  if(!mOpts.contains(opt)) fatal(FUNC, QString("Test for unregistered option: %1").arg(opt));

  return false;
}

bool CmdHelper::isMissingParms(int n/* = 0*/)
{
  // At 2nd level we expect:
  // <Prog> <Command> [<ParmList>] [<OptList>]
  //
  // Returns false if any kind of help is advisable
  // and true if all looks good and nobody want help.

  if(wantHelp())
  {
    initPrinting();
    return true;
  }

  Option saveInqOpt = mInqOpt;

  if(mMandatory.size())
  {
    foreach(StrSet mo, mMandatory)
    {
      bool ok = false;
      QString opts;
      foreach(QString o, mo)
      {
        if(has(o)) { ok = true; break; }

        opts.append(" --" + o);
      }

      if(!ok) error(FUNC, tr("One of these options is mandatory:%1").arg(opts));
    }
  }

  mInqOpt = saveInqOpt;

  if(hasError()) return true;
  if(!n)         return false;

  int missing = n - mInqOpt.parmCount;

  if(missing > 0)
  {
    error(FUNC, cMissingArg.arg(mInqOpt.name).arg(missing));
    initPrinting();
    return true;
  }

  return false;
}

bool CmdHelper::subCmdLooksBad()
{
  // At 3rd level we expect:
  // <Prog> <Command> <SubCommand> [<ParmList>] [<OptList>]
  //
  // All what we do is to check if <SubCommand> is known.
  // Returns false if --help or <SubCommand> is Ok else true.

  if(mCmdLine.size() < 3)
  {
    if(wantHelp()) return false;
    if(!mSubCmds.size()) return false; // Sometimes there are no subcommands needed

    // Called without <SubCommand> only "<Prog> <Command>"
    print(tr("Try: %1 %2 [<%3>] -h").arg(mCmdLine.at(0), cmd(), cCommand));
    printCommands(mSubCmds);
    return true;
  }

  QString subCmd = mCmdLine.at(2);
  if(subCmd.startsWith("--")) subCmd = subCmd.mid(2);

  if(!mSubCmds.contains(subCmd))
  {
    if(wantHelp()) return false;
    if(!mSubCmds.size()) return false; // Sometimes there are no subcommands needed

    error(FUNC, tr("Unknown subcommand: %1").arg(subCmd));
    print(tr("Try: %1 %2 [<%3>] -h").arg(mCmdLine.at(0), cmd(), cCommand));
    printCommands(mSubCmds);
    return true;
  }

  if(!mOptions.has(subCmd))
  {
    // Was not memorized
    mInqSubCmd.name = subCmd;
    mInqSubCmd.idxPos = 2;
    mInqSubCmd.parmCount = mOptions.parmCount(cmd()) - 1;
    mOptions.insert(subCmd, mInqSubCmd);

    // Just for correctness
    mInqCmd = mOptions.named(cmd());
    mInqCmd.parmCount = 0;
    mOptions.insert(cmd(), mInqCmd);
  }

  mInqOpt = mInqSubCmd;
  mHelpLevel = 3;

  return false;
}

void CmdHelper::regCmds(const QString& cmds)
{
  // Register Commands
  StrSet cmdLst = stripOptions(cmds);

  foreach(QString cmd, cmdLst)
  {
    if(mCmds.contains(cmd))
      fatal(FUNC, QString("Command already registered: %1").arg(cmd));

    mCmds.insert(cmd);
  }
}

void CmdHelper::regSubCmds(const QString& sub)
{
  // Register Sub Ccommands
  StrSet cmdLst = stripOptions(sub);

  foreach(QString cmd, cmdLst)
  {
    if(mSubCmds.contains(cmd))
      fatal(FUNC, QString("Subcommand already registered: %1").arg(cmd));

    mSubCmds.insert(cmd);
  }
}

void CmdHelper::regOpts(const QString& opts)
{
  // Register Options
  StrSet optLst = stripOptions(opts);

  foreach(QString opt, optLst)
  {
    if(mOpts.contains(opt))
      fatal(FUNC, QString("Option already registered: %1").arg(opt));

    mOpts.insert(opt);
  }
}

void CmdHelper::regStdOpts(const QString& opts)
{
  // Register Standard Options
  // These are always available at any command and is no need to list explicit
  regOpts(opts);

  StrSet stdOpts = mOptionSet.value("Options");
  if(!stdOpts.size())
  {
    mLabel.insert("Options", "options");
  }
  stdOpts.unite(stripOptions(opts));
  mOptionSet.insert("Options", stdOpts);
}

void CmdHelper::regOptsOneOfIsMandatory(const QString& alias, const QString& opts)
{
  regOpts(opts);
  makeOneOfOptsMandatory(alias, opts);
}

void CmdHelper::makeOneOfOptsMandatory(const QString& alias, const QString& opts)
{
  mMandatory.insert(alias, stripOptions(opts));
}

void CmdHelper::groupOpts(const QString& alias, const QString& opts)
{
  mTempGroup.insert(alias, stripOptions(opts));
}

void CmdHelper::inOptGroup(const QString& alias, const QString& txt, const QString& opts)
{
  inLabel(alias, txt);
  mOptionSet.insert(alias, stripOptions(opts));
}

void CmdHelper::inLabel(const QString& alias, const QString& txt)
{
  if(mLabel.contains(alias)) fatal(FUNC, QString("Alias already used: %1").arg(alias));
  mLabel.insert(alias, txt);
}

void CmdHelper::inBrief(const QString& cmd, const QString& brief, StrSet& cmds)
{
  if(!cmds.contains(cmd))
  {
    fatal(FUNC, QString("Command not registered: %1").arg(cmd));
    return;
  }

  mOptions.inBrief(cmd, brief);
}

void CmdHelper::inOptBrief(const QString& opt, const QString& parms, const QString& brief)
{
  if(!mOpts.contains(opt))
  {
    fatal(FUNC, QString("Option not registered: %1").arg(opt));
    return;
  }

  mOptions.inBrief(opt, parms, brief);
}

void CmdHelper::initPrinting()
{
  set("FirstTimeForInst");
}

void CmdHelper::printBrief(const StrSet& cmds, const QString& header)
{
  if(!cmds.size()) return; // Sometimes happens, nothing to print

  // Print an overview of available commands
  print("");
  print(header);

  QStringList cmdLst = cmds.toList();
  cmdLst.sort();

  if(isSet("PrintCmdBriefCompact"))
  {
    int indent = 2;
    QStringList table = FTool::formatToTable(cmdLst, mScreenWidth - indent);
    foreach(QString l, table)
    {
      print(QString("  %1").arg(l));
    }
  }
  else
  {
    foreach(QString c, cmdLst)
    {
      printDef(c, mOptions.brief(c));
    }
  }
  print("");
}

void CmdHelper::printCommands(const StrSet& cmds)
{
  // To give only a very short info about available commands
  print(cmdHeader());

  QStringList cmdLst = cmds.toList();
  cmdLst.sort();

  if(cmdLst.size() < 6)
  {
    print(QString("  %1").arg(cmdLst.join(" ")));
  }
  else
  {
    int indent = 2;
    QStringList table = FTool::formatToTable(cmdLst, mScreenWidth - indent);
    foreach(QString l, table)
    {
      print(QString("  %1").arg(l));
    }
  }

  print("");
}

void CmdHelper::printOptBrief(const StrSet opts, const QString& header)
{
  if(!opts.size()) return;

  const int cOptWidth = 20;

  // Print an overview of available options
  print("");
  print(header);

  QStringList optLst = opts.toList();
  optLst.sort();

  foreach(QString opt, optLst)
  {
    QString a = mOptions.parms(opt); // a like argument
    QString b = mOptions.brief(opt); // b like brief

    if(b.isEmpty())
    {
      fatal(FUNC, QString("Registered option without brief: %1").arg(opt));
      continue;
    }

    if(isNotSet("Help+"))
    {
      // Print only first sentence of the brief to keep the look nicer
      //   2 spaces indent
      //   1 space between term and brief
      int maxLength = mScreenWidth - (2 + 1 + cOptWidth);
      if(b.size() > maxLength)
      {
        QStringList bs = b.split(". "); // bs like brief sentences
        b = bs.at(0);
        if(bs.size() > 1)
        {
          set("HelpHidden");

          if((b.size() + 4) < maxLength)
          {
            b.append(" ...");
          }
          else if(b.size() > maxLength)
          {
            // Now it's doesn't matter
            b.append(" ...");
          }
        }
      }
    }

    // Check if argument has a default value.
    if(a.contains(":")) b.append(tr(". Default is"));

    QStringList args = a.split(" "); // Now we have all arguments the option takes
    for(int i = 0; i < args.size(); ++i)
    {
      QStringList ad = args.at(i).split(":"); // ad like argument + default
      if(ad.size() > 1)
      {
        QString h = ad.at(1); // h like help
        h.remove(QRegExp("[>\\]]+"));
        b.append(" " + h);

        h = ad.at(1);
        h.remove(QRegExp("\\w+"));
        h.prepend(ad.at(0));
        opt.append(" " + h);

        //FIXME Collect info about opt to print possible values arg can have
        //h.remove(QRegExp("[\\[\\]]+"));
      }
      else
      {
        opt.append(" " + ad.at(0));
      }
    }

    printDef(opt, b, cOptWidth);
  }

  print("");
}

bool CmdHelper::printThisWay(const QString& s)
{
  // Returns !mWantHelp to simplify usage

  // Replace direct listed short hand options with full option
  // and extract the default value, if any, e.g.
  //    '~~I-am-sure'  becomes '--I-am-sure'
  //    '~~checkDay'   becomes first '--checkDay <DayName:Fri>' and than '--checkDay <DayName>'
  QString   cmdLine = s;
  cmdLine.append(" [<Options>]");
  StrSet    extraOpts;
  QRegExp   match("(~~[\\w-]+)");      // Matching words with minus like '~~I-am-sure'
  QRegExp   rxParmDef(":\\w+");        // Matching words with leading colon like ':Fri'
  int       pos = 0;
  while((pos = match.indexIn(cmdLine, pos)) != -1)
  {
    QString o = match.cap(1).mid(2);
    if(!mOpts.contains(o))
    {
      fatal(FUNC, QString("Unknown option specified: ~~%1").arg(o));
      pos += match.matchedLength();
      continue;
    }

    extraOpts.insert(o);
    QString opt = QString("--%1 %2").arg(o, mOptions.parms(o).remove(rxParmDef));
    opt = opt.trimmed(); // In case that option need no parm there is a unwanted space
    cmdLine.replace(pos, match.matchedLength(), opt);
    pos += opt.size();
  }

  // Replace <Options> with its content in case we have some direct listed found
  match.setPattern("\\[<Options>\\]");
  pos = match.indexIn(cmdLine, 0);
  if(extraOpts.size() and pos > 0)
  {
    QStringList opts;
    if(mTempGroup.contains("Options"))
    {
      opts = mTempGroup.value("Options").toList();
      extraOpts.unite(mTempGroup.value("Options"));
    }
    else
    {
      opts = mOptionSet.value("Options").toList();
      extraOpts.unite(mOptionSet.value("Options"));
    }

    opts.sort();
    QString substitude;
    foreach(QString o, opts)
    {
      substitude.append(QString("[--%1 %2] ").arg(o, mOptions.parms(o).remove(rxParmDef)));
    }

    cmdLine.replace(pos, match.matchedLength(), substitude);
  }

  // Replace mandatory option place holders
  pos = 0;
  match.setPattern("<(\\w+)>");
  int found = 0; // Count if we have a place holder found
  while((pos = match.indexIn(cmdLine, pos)) != -1)
  {
    QString mph = match.cap(1); // No, not miles per hour, Mandatory Place Holder
    if(!mMandatory.contains(mph)) { pos += match.matchedLength(); continue; }

    QStringList mol = mMandatory.value(mph).toList(); // Mandatory Option List
    mol.sort();
    QString mo;                                       // Mandatory Option
    if(found) mo.append(" \\ ");                      // Insert a hard wrap mark
    ++found;
    foreach(QString o, mol)
    {
      extraOpts.insert(o);
      QString opt = QString("--%1 %2").arg(o, mOptions.parms(o).remove(rxParmDef));
      opt = opt.trimmed(); // In case that option need no parm there is a unwanted space

      mo.append(opt + " | ");
    }

    mo.chop(3); // Remove last added " | "
    cmdLine.replace(pos, match.matchedLength(), mo);
    pos += mo.size();
  }

  // Build "The Way" and memorize what we have to print
  QString helpOpt(" [--help|-h]");
  bool prtCmdBrief = false;
  bool prtSubCmdBrief = false;
  bool prtOptBrief = false;

  switch(mHelpLevel)
  {
    case 1:
      if(mCmds.size() < 2) helpOpt.clear(); // Should happen very rarely, anyway
      else prtCmdBrief = true;
      mTheWay = QString("  %1").arg(mCmdLine.at(0));
      break;

    case 2:
      if(mSubCmds.size() < 2)
      {
        helpOpt.clear();
        prtOptBrief = true;
      }
      else
      {
        prtSubCmdBrief = true;
      }
      mTheWay = QString("  %1 %2").arg(mCmdLine.at(0), cmd());
      break;

    case 3:
      helpOpt.clear();
      prtOptBrief = true;
      mTheWay = QString("  %1 %2 %3").arg(mCmdLine.at(0), cmd(), subCmd());
      break;

    default: fatal(FUNC, QString("Not supported help level: %1").arg(mHelpLevel)); break;
  }

//   mTheWay.remove("./progs/agentf/"); // Testing only

  cmdLine.append(helpOpt);
  print(tr("Please call me this way:"));

  // Wrap the command call, if needed
  if(1 + mTheWay.size() + cmdLine.size() > mScreenWidth)
  {
    QStringList parts = FTool::breakUpText(cmdLine);
    QString cmd = mTheWay;
    int i = 0;
    // Build the first line
    while(cmd.size() < mScreenWidth and i < parts.size())
    {
      if(parts.at(i) == "\\") { ++i; break; }  // Wrap at hard coded positions
      cmd.append(" " + parts.at(i++));
      if(cmd.size() > mScreenWidth * 0.7 and parts.size() - i > 1)
      {
        if(cmd.endsWith("]") or cmd.endsWith("..")) break;
      }
    }
    print(cmd);

    // Build all following lines
    while(i < parts.size())
    {
      // Ignore hard coded wrap at this point, we have a new line
      if(parts.at(i) == "\\") { ++i; continue; }

      cmd = QString("%1 %2").arg(" ", mTheWay.size()).arg(parts.at(i++));
      while(i < parts.size())
      {
        //qDebug() << mScreenWidth << mScreenWidth * 0.7 << cmd.size() << parts.at(i);
        if(parts.at(i) == "\\") { ++i; break; }  // Wrap at hard coded positions

        // Make an early smart wrap if the next option is coming
        if(cmd.size() > mScreenWidth * 0.7 and parts.size() - i > 1 /*and parts.at(i) != "|"*/)
        {
          if(parts.at(i + 1).startsWith("[--")) { ++i; break; }
          if(parts.at(i + 1).startsWith("--"))  { ++i; break; }
        }

        // Don't wrap an option inside his param list
        if(1 + cmd.size() + parts.at(i).size() > mScreenWidth and !parts.at(i).startsWith("<")) break;
        cmd.append(" " + parts.at(i++));
      }
      print(cmd);
    }
  }
  else
  {
    print(mTheWay + " " + cmdLine);
  }

  if(!wantHelp()) return true;

  // Examine the command call, what descriptions schould be printed
  // For that task strip of anything to become clear words
  QStringList cmdParts = cmdLine.split(" ", QString::SkipEmptyParts);
  cmdParts.replaceInStrings("<", "");
  cmdParts.replaceInStrings(">", "");
  cmdParts.replaceInStrings("[", "");
  cmdParts.replaceInStrings("]", "");
  cmdParts.replaceInStrings("..", "");

  if(prtCmdBrief)    printCmdBrief(cmdHeader(mLabel.value(cmdParts.at(0), cCommands)));
  if(prtSubCmdBrief) printSubCmdBrief(cmdHeader(mLabel.value(cmdParts.at(0), cCommands)));
  if(prtOptBrief)
  {

    foreach(QString p, cmdParts)
    {
      StrSet opts;
      if(mTempGroup.contains(p))  opts = mTempGroup.value(p);
      else if(mLabel.contains(p)) opts = mOptionSet.value(p);
      else continue;

      if(extraOpts.size() and "Options" == p)
      {
        opts.unite(extraOpts);
        extraOpts.clear();
      }

      printOptBrief(opts, optHeader(mLabel.value(p)));
    }

    mTempGroup.clear();

    if(extraOpts.size())
    {
      printOptBrief(extraOpts, optHeader());
    }
  }

  printCommandBrief();
  return false;
}

void CmdHelper::printForInst(const QString& s)
{
  if(s.isEmpty()) return;

  if(isSet("FirstTimeForInst"))
  {
    unSet("FirstTimeForInst");
    print("");
    print(tr("For instance:"));
  }

  print(mTheWay + " " + s);
}

void CmdHelper::prin4Comment(const QString& s)
{
  if(s.isEmpty()) return;

  // Print only when --help+
  if(isNotSet("Help+"))
  {
    set("HelpHidden");
    return;
  }

  printComment(s);
}

void CmdHelper::printCommandBrief()
{
  print(""); // Keep this, anyway if valid command or not
  switch(mHelpLevel)
  {
    case 1: break; // cmd() is always empty
    case 2:
    {
      if(cmd().isEmpty()) break;
      QString brief = mOptions.brief(cmd());
      if(brief.isEmpty()) break;
      print(QString("%1 '%2': %3.").arg(cCommand, cmd(), brief));
      break;
    }
    case 3:
    {
      if(subCmd().isEmpty()) break;
      QString brief = mOptions.brief(subCmd());
      if(brief.isEmpty()) break;
      print(QString("%1 '%2': %3.").arg(cCommand, subCmd(), brief));
      break;
    }
    default: fatal(FUNC, QString("Not supported help level: %1").arg(mHelpLevel)); break;
  }
}

void CmdHelper::printComment(const QString& s)
{
  if(s.isEmpty()) return;

  QStringList wrap = FTool::wrapText(s, mScreenWidth);
  foreach(QString s, wrap)
  {
    print(s);
  }
}

void CmdHelper::prin4Note(const QString& s)
{
  if(s.isEmpty()) return;

  // Print only when --help+
  if(isNotSet("Help+"))
  {
    set("HelpHidden");
    return;
  }

  printNote(s);
}

void CmdHelper::printNote(const QString& s)
{
  if(s.isEmpty()) return;

  // Print a note text as separated block
  QString note = QString("%1: %2").arg(cNote, "%1");
  QStringList txt = FTool::wrapText(s, mScreenWidth - (note.size() - 2));

  print("");

  if(txt.size() > 2)
  {
    // Print first text line alone
    print(QString("%1%2").arg(" ", -(note.size() - 2)).arg(txt.at(0)));
    // ..than a line 'Note' marker...
    print(note.arg(txt.at(1)));
    txt.removeAt(0);
    txt.removeAt(0);
  }
  else
  {
    // Print first line with the 'Note' marker...
    print(note.arg(txt.at(0)));
    txt.removeAt(0);
  }

  // ...and the rest of the text as block
  foreach(QString s, txt)
  {
    print(QString("%1%2").arg(" ", -(note.size() - 2)).arg(s));
  }

  print("");
}

void CmdHelper::printDef(const QString& term, const QString& def
                       , int termWidth/* = 12*/, int defW/* = 999*/)
{
  // Adjust definition width, if needed
  int defWidth = defW < mScreenWidth - termWidth - 2
                      ? defW : mScreenWidth - termWidth - 2;

  // Print Definition List
  if(def.size() > defWidth)
  {
    QStringList wrapDef = FTool::wrapText(def, defWidth);
    if(term.size() > termWidth)
    {
      // Print term alone in a line...
      print(QString("  %1").arg(term));
    }
    else
    {
      // First line with term and part of the definition...
      print(QString("  %1 %2").arg(term, -termWidth).arg(wrapDef.at(0)));
      wrapDef.removeAt(0);
    }

    // ...and the rest of the definition as block below
    foreach(QString s, wrapDef)
    {
      print(QString("  %1 %2").arg(" ", -termWidth).arg(s));
    }
  }
  else
  {
    if(term.size() > termWidth)
    {
      // Print term alone in a line...
      print(QString("  %1").arg(term));
       // ...and the definition in one line below
      print(QString("  %1 %2").arg(" ", -termWidth).arg(def));
    }
    else
    {
      // The easiest way, all on one line
      print(QString("  %1 %2").arg(term, -termWidth).arg(def));
    }
  }
}

QStringList CmdHelper::parmList(const QString& opt)
{
  int idx;
  int parmCount;
  QStringList parms;

  if(opt.isEmpty())
  {
    idx = mInqOpt.idxPos;
    parmCount = mInqOpt.parmCount;
  }
  else
  {
    idx = mOptions.idx(opt);
    parmCount = mOptions.parmCount(opt);;
  }

  if(idx < 0) return parms;

  forever
  {
    ++idx;
    if(!parmCount) break;

    parms << mCmdLine.at(idx);
    --parmCount;
  }

  return parms;
}

QString CmdHelper::argStr(int pos, const QString& def/* = ""*/)
{
  if(mInqOpt.parmCount < 0) fatal(FUNC, cCmdPCNotValid);
  if(mInqOpt.parmCount < pos) return def;

  return mCmdLine.at(pos + mInqOpt.idxPos);
}

int CmdHelper::argInt(int pos, int def/* = 0*/)
{
  QString parm = argStr(pos, "def");

  if("def"  == parm) return def;

  bool ok;
  int ret = parm.toInt(&ok);
  if(ok) return ret;

  error(FUNC, QString(cParmIsNot).arg(pos).arg(mInqOpt.name, tr("an integer"), parm));

  return def;
}

double CmdHelper::argDouble(int pos, double def/* = 0.0*/)
{
  QString parm = argStr(pos, "def");

  if("def"  == parm) return def;

  bool ok;
  double ret = parm.toDouble(&ok);
  if(ok) return ret;

  error(FUNC, QString(cParmIsNot).arg(pos).arg(mInqOpt.name, tr("a double"), parm));

  return def;
}

QDate CmdHelper::argDate(int pos, const QDate& def/* = QDate()*/, const QDate& autoDate/* = QDate()*/)
{
  QDate   ret;
  QDate   dotDotDate(autoDate);
  if(!autoDate.isValid()) dotDotDate = def; // If not given use default date as auto date

  QString parm = argStr(pos, "def");

  if("def"  == parm) ret = def;
  else if("auto" == parm or ".." == parm) ret = dotDotDate;  // We accept auto and .. as valid
  else ret = QDate::fromString(parm, Qt::ISODate);

  if(!ret.isValid())
    error(FUNC, QString(cParmIsNot).arg(pos).arg(mInqOpt.name, tr("a date"), parm));

  return ret;
}

QString CmdHelper::optStr(const QString& opt, const QString& def/* = ""*/, int pos/* = 1*/)
{
  int idx = mOptions.idx(opt);

  if(idx < 0)
  {
    if(!verboseLevel(eAmple)) return def;
    if(!mOpts.contains(opt)) fatal(FUNC, QString("Test for unregistered option: %1").arg(opt));
    return def;
  }

  if(pos > mOptions.parmCount(opt)) return def;

  return mCmdLine.at(idx + pos);
}

int CmdHelper::optInt(const QString& opt, int def/* = 0*/, int pos/* = 1*/)
{
  const QString val = optStr(opt, "def", pos);

  if("def" == val) return def;

  bool ok;
  int ret = val.toInt(&ok);
  if(ok) return ret;

  int idx = mOptions.idx(opt);
  error(FUNC, QString(cParmIsNot).arg(pos)
                                 .arg(mCmdLine.at(idx), tr("an integer"), mCmdLine.at(idx + pos)));

  return def;
}

double CmdHelper::optDouble(const QString& opt, double def/* = 0.0*/, int pos/* = 1*/)
{
  const QString val = optStr(opt, "def", pos);

  if("def" == val) return def;

  bool ok;
  double ret = val.toDouble(&ok);
  if(ok) return ret;

  int idx = mOptions.idx(opt);
  error(FUNC, QString(cParmIsNot).arg(pos)
                                 .arg(mCmdLine.at(idx), tr("a double"), mCmdLine.at(idx + pos)));

  return def;
}

QDate CmdHelper::optDate(const QString& opt, const QDate& def/* = QDate()*/, int pos/* = 1*/)
{
  const QString val = optStr(opt, "def", pos);

  if("def" == val) return def;

  QDate ret = QDate::fromString(val, Qt::ISODate);
  if(ret.isValid()) return ret;

  int idx = mOptions.idx(opt);
  error(FUNC, QString(cParmIsNot).arg(pos)
                                 .arg(mCmdLine.at(idx), tr("a date"), mCmdLine.at(idx + pos)));

  return def;
}

QString CmdHelper::strParmBool(int pos)
{
  return parmBool(pos) ? "true" : "false";
}

QString CmdHelper::strParmDate(int pos)
{
  return parmDate(pos).toString(Qt::ISODate);
}

QString CmdHelper::strParmDouble(int pos)
{
  return QString::number(parmDouble(pos), 'f', 2);
}

QString CmdHelper::strParmInt(int pos)
{
  return QString::number(parmDouble(pos));
}

QString CmdHelper::parmStr(int pos)
{
  if(mInqOpt.parmCount < pos)
  {
    error(FUNC, cMissingArg.arg(mInqOpt.name).arg(pos - mInqOpt.parmCount));
    return "";
  }

  return mCmdLine.at(mInqOpt.idxPos + pos);
}

bool CmdHelper::parmBool(int pos)
{
  QString parm = parmStr(pos).toLower();

  if("1" == parm or "true" == parm) return true;
  if("0" == parm or "false" == parm) return false;

  error(FUNC, cParmIsNot.arg(pos).arg(mInqOpt.name, "bool", parm));
  return false;
}

QDate CmdHelper::parmDate(int pos)
{
  QString parm = parmStr(pos);
  QDate ret = QDate::fromString(parm, Qt::ISODate);

  if(!ret.isValid())
    error(FUNC, QString(cParmIsNot).arg(pos).arg(mInqOpt.name, tr("a date"), parm));

  return ret;
}

double CmdHelper::parmDouble(int pos)
{
  QString parm = parmStr(pos);

  bool ok;
  double ret = parm.toDouble(&ok);
  if(!ok) error(FUNC, QString(cParmIsNot).arg(pos).arg(mInqOpt.name, tr("a double"), parm));
  return ret;
}

int CmdHelper::parmInt(int pos)
{
  QString parm = parmStr(pos);

  bool ok;
  double ret = parm.toInt(&ok);
  if(!ok) error(FUNC, QString(cParmIsNot).arg(pos).arg(mInqOpt.name, tr("a integer"), parm));
  return ret;
}
