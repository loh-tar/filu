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

#ifndef CMDHELPER_HPP
#define CMDHELPER_HPP

#include <QDate>
#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>

#include "Newswire.h"

/***********************************************************************
*
*   Command Helper
*
*   A collection of funtions to handle command line arguments
*   and to print help messages.
*
*   The command line looks like:
*     <Prog> [<ParmList>] [OptList] [-h|--help]
*
*   Furthermore can be <ParmList> interpreted as:.
*     <Command> <ParmList> or
*     <Command> <SubCommand> <ParmList>
*
*   So far, so clear. Whereas..
*     <ParmList>   looks like: <Parm> [<Parm>]..
*     <OptList>    looks like: <Opt> [<Opt>]..
*     <Parm>       looks like: Any character but not space
*     <Opt>        looks like: --<Parm> [<ParmList>]
*
*   Therefore looks <Command> and <SubCommand> like:  [--]<Parm>
*
*   For instance:
*     agentf this <Symbol> <Market> <Provider> [<FromDate> [<ToDate>]]
*     agentf scan [--group <GrpName>] [--indi <IndiName>] [--verbose <Level>]
*     agentf filu create [--db <DBName>] [--schema <Schema>]
*
*   These grouped program calls are all do the same:
*     agentf this AAPL NewYork Yahoo
*     agentf --this AAPL NewYork Yahoo
*
*     agentf scan --group all --indi MyNewIdea --verbose Info
*     agentf scan --verbose Info --indi MyNewIdea --group all
*     agentf --scan --verbose Info --indi MyNewIdea --group all
*
*     agentf filu create --db testdb --schema foobar
*     agentf filu --create --schema foobar --db testdb
*     agentf --filu create --db testdb --schema foobar
*
*   Theoretically possible would be...
*     agentf --verbose Info --indi MyNewIdea --group all --scan
*     agentf --create --filu --db testdb  --schema foobar
*
*   ...but that looks confusing an would be a performance issue. Therefore
*   is it not supported.
*
*   In summary, that means that the program call looks like:
*     <Prog> <Command> [<ParmList>] [<SubCommand> [<ParmList>]].. [OptList] [-h|--help]
*
************************************************************************/

class CmdHelper : public Newswire
{
  protected:
    typedef QSet<QString>             StrSet;
    typedef QHash<QString, StrSet>    HashStrStrSet;
    typedef QHash<QString, QString>   HashStrStr;


    struct Option
    {
      QString     name;                 // The option name itself
      QString     parms;                // The parm brief, not the parms given on the cmd line
      QString     brief;                // The brief of the option
      int         idxPos;               // The position in mCmdLine
      int         parmCount;            // The count of given parms on the cmd line

                  Option()
                  : idxPos(-1)          // Default ctor, make sure the option is "not used"...
                  , parmCount(-1) {}    // ...and "not valid"
    };

    class OptionList
    {
    public:
      bool        insert(const QString& o, const Option& opt) {
                      if(mOptions.contains(o)) {
                        mOptions.insert(o, opt);
                        return false;
                      } else {
                        mOptions.insert(o, opt);
                        return true;
                      } }

      void        inBrief(const QString& o, const QString& b) {
                      Option opt = mOptions.value(o);
                      opt.brief = b;
                      mOptions.insert(o, opt); }

      void        inBrief(const QString& o, const QString& p, const QString& b) {
                      Option opt = mOptions.value(o);
                      opt.parms = p;
                      opt.brief = b;
                      mOptions.insert(o, opt); }

      QString     brief(const QString& o)     { return mOptions.value(o).brief; }
      QString     parms(const QString& o)     { return mOptions.value(o).parms; }
      bool        has(const QString& o)       { return mOptions.contains(o); }
      int         idx(const QString& o)       { return mOptions.value(o).idxPos; }
      int         parmCount(const QString& o) { return mOptions.value(o).parmCount; }
      Option      named(const QString& o)     { return mOptions.value(o); }
      void        clear()                     { mOptions.clear(); }
    protected:
      QHash<QString, Option>  mOptions;
    };

  public:
                  CmdHelper(Newswire* parent);
    virtual      ~CmdHelper();

    // 1st level stuff
    void          setUp(const QString& opt);
    void          regCmds(const QString& cmds);
    void          regOpts(const QString& opts);
    void          regStdOpts(const QString& opts);
    void          regOptsOneOfIsMandatory(const QString& alias, const QString& opts);
    void          makeOneOfOptsMandatory(const QString& alias, const QString& opts);
    bool          cmdLineLooksBad(const QStringList& cmdLine);
    void          inGreeter(const QString& txt) { inLabel("<>Greeter<>", txt); }
    void          inLabel(const QString& alias, const QString& txt);
    void          inCmdBrief(const QString& cmd, const QString& brief) { inBrief(cmd, brief, mCmds); }
    void          inOptBrief(const QString& opt, const QString& parms, const QString& brief);
    void          inOptGroup(const QString& alias, const QString& txt, const QString& opts);
    void          groupOpts(const QString& alias, const QString& opts);

    // 2nd level stuff
    bool          isMissingParms(int n = 0);

    // 3rd level stuff
    void          regSubCmds(const QString& sub);
    bool          subCmdLooksBad();
    void          inSubBrief(const QString& cmd, const QString& brief) { inBrief(cmd, brief, mSubCmds); }
    bool          needHelp(int level);
    bool          wantHelp() { return isSet("WantHelp"); }
    void          aided();

    // Obtain any info about command line stuff
    bool          has(const QString& opt);        // Don't change mInqOpt
    // All these change mInqOpt
    bool          hasCmd(const QString& cmd);
    bool          hasSubCmd(const QString& cmd);
    bool          hasOpt(const QString& opt);
    int           parmCount() { return mInqOpt.parmCount; }

    QString       cmd() { return mInqCmd.name; }
    QString       subCmd() { return mInqSubCmd.name; }
    QStringList   cmdLine() { return mCmdLine; }
    QStringList   parmList(const QString& opt = "");  // Ship all parms of given opt or last asked opt

    // All these functions are lazy. They ship a default value if parm is not avaiable
    // but thrown an error if given parm is not valid
    QString       optStr(const QString& opt, const QString& def = "", int pos = 1);
    int           optInt(const QString& opt, int def = 0, int pos = 1);
    double        optDouble(const QString& opt, double def = 0.0, int pos = 1);
    QDate         optDate(const QString& opt, const QDate& def = QDate(), int pos = 1);

    QString       argStr(int pos, const QString& def = "");
    int           argInt(int pos, int def = 0);
    double        argDouble(int pos, double def = 0.0);
    QDate         argDate(int pos, const QDate& def = QDate(), const QDate& autoDate = QDate());

    // Here we have strict functions.
    // They thrown an error if parm at pos if not avaiable or not valid.
    // They work on last asked option by hasFoo()
    QString       strParm(int pos) { return parmStr(pos); }
    QString       strParmBool(int pos);
    QString       strParmDate(int pos);
    QString       strParmDouble(int pos);
    QString       strParmInt(int pos);

    QString       parmStr(int pos);
    bool          parmBool(int pos);
    QDate         parmDate(int pos);
    double        parmDouble(int pos);
    int           parmInt(int pos);

    // Printing stuff
    bool          printThisWay(const QString& s);
    void          printForInst(const QString& s);
    void          printComment(const QString& s);
    void          prin4Comment(const QString& s);  // Only print if --help+. The 4 looks like t :-)
    void          printNote(const QString& s);
    void          prin4Note(const QString& s);     // Only print if --help+. The 4 looks like t :-)

  protected:
    void          inBrief(const QString& cmd, const QString& brief, StrSet& cmds);
    void          initPrinting();
    void          printCommandBrief();
    void          printBrief(const StrSet& cmds, const QString& header);
    void          printCmdBrief(const QString& header) { printBrief(mCmds, header); }
    void          printSubCmdBrief(const QString& header) { printBrief(mSubCmds, header); }
    void          printOptBrief(const StrSet opts, const QString& header);
    void          printCommands(const StrSet& cmds);
    void          printDef(const QString& term, const QString& def, int termWidth = 12, int defW = 999);

    void          set(const QString& opt);
    void          unSet(const QString& opt);
    bool          isSet(const QString& opt);
    bool          isNotSet(const QString& opt) { return !isSet(opt); }

    QStringList       mCmdLine;             // Copy of the gotten command line
    OptionList        mOptions;             // Hold the parsed cmd line. All stuff of Cmd/SubCmd/Opt
    StrSet            mCmds;                // Hold registered command names
    StrSet            mSubCmds;             //   "      "      subcommand names
    StrSet            mOpts;                //   "      "      option names
    HashStrStrSet     mMandatory;           //   "      "      options where one of is mandatory
    HashStrStrSet     mOptionSet;           // Hold grouped options for help printing
    HashStrStrSet     mTempGroup;           // Hold temporary regrouped options for help printing
    HashStrStr        mLabel;               // Hold a very short description of something

    QString           mTheWay;
    StrSet            mSet;                 // Hold status flags and CmdHelper options
    const StrSet      mAvailPublicSet;      // Hold possible public settings
    const StrSet      mAllKnownSet;         // Hold all possible settings
    int               mHelpLevel;           // A number to indikate at wich point of execution the trouble occured

    Option            mInqCmd;              // Inquired command by hasCmd()
    Option            mInqSubCmd;           // Inquired subcommand by hasSubCmd()
    Option            mInqOpt;              // Inquired option by hasFoo()

    int               mScreenWidth;

  private:

};

#endif
