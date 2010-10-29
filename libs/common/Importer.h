//
//   This file is part of Filu.
//
//   Copyright (C) 2007, 2010  loh.tar@googlemail.com
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

#ifndef IMPORTER_HPP
#define IMPORTER_HPP

#include "FClass.h"

class FiTuple;
class SymbolTuple;

/***********************************************************************
*
*  Yes, he imports stuff into the DB.
*  "Stuff" are FIs with symbols and index membership. The data is in an
*  *.csv (semicolon as delimeter) format expected with header and some
*  more optional tags.
*
*  For details about the file format look at
*  FiluSource/doc/import-file-format.txt
*
************************************************************************/


class Importer : public FClass
{
  public:
                  Importer(FClass* parent);
    virtual      ~Importer();

    void          reset();
    bool          import(QString& data);

  protected:
    void          init();
    void          printDot();
    bool          handleTag(QStringList& row);
    QString       makeUnique(const QString& key);
    void          buildPair(QString& key, QString& value, const QString& line);
    void          prepare();
    void          setFi();             // mFi;
    void          setSymbol();         // mSymbol
    bool          setSymbol(const QString& symbol);
    bool          setMarket(const QString& market);

    void          addSymbolType();
    void          addMarket();
    void          addFi();
    void          addSymbol();
    void          addUnderlying();
    void          addEODBar();
    void          addSplit();

    // user data imports
    void          addCO();
    void          addGroup();

    QString       mOrigData;           // copy of the data at import(), FIXME:unused
    QStringList   mPendingData;        // collect data till committed
    QHash<QString, QString> mData;     // the split mOrigData associated to the [Header]
    QStringList   mHeader;             // must be a stringlist, we need the positions of header data
    QSet<QString> mToDo;               // holds notes, which job is todo to avoid redundant if()
    bool          mPrepared;

    QString       mMustBeUnique;
    QStringList   mAllSymbolTypes;     // all possible symbol types
    QStringList   mKnownSymbolTypes;
    QHash<QString, bool> mKnownSTisProvider; // SymbolType is...?
    int           mTotalSymbolCount;
    QStringList   mUsedRefSymbols;     // reference symbols, don't install
    QStringList   mUsedKnownSymbols;   // none provider symbols which has to be installed
    int           mUsedSymbols;        // provider symbols which has to be installed
    QStringList   mAllUsedSymbols;     // mUsedRefSymbols + mUsedKnownSymbols + mUsedSymbols
    FiTuple*      mFi;
    SymbolTuple*  mSymbol;
    QTextStream   mConsole;
    int           mFiId;
    int           mMarketId;
    QTime         mRolex;

  private:

};

#endif
