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

#ifndef IMPORTER_HPP
#define IMPORTER_HPP

#include <QQueue>
#include <QSet>
#include <QTextStream>
#include <QTime>

#include "FClass.h"
class FiTuple;
class SymbolTuple;
class Validator;

/***********************************************************************
*
*   Yes, he imports stuff into the DB.
*   "Stuff" are FIs with symbols and index membership. The data is in an
*   *.csv (semicolon as delimeter) format expected with header and some
*   more optional tags.
*
*   For details about the file format look at
*   FiluSource/doc/import-file-format.txt
*
************************************************************************/


class Importer : public FClass
{
  public:
                  Importer(FClass* parent);
    virtual      ~Importer();

    void          reset();
    bool          import(const QString& line);
    void          getPreparedHeaderData(QStringList& header, QHash<QString, QString>& data);
    QString       makeNameNice(const QString& name);
    void          makeNameNice(QString& name);

  protected:
    enum Effect
    {
      eEffectPending = 0,
      eEffectOk      = 1,
      eEffectFault   = 2
    };

    void          printStatus(Effect effect = eEffectPending, const QString& extraTxt = "");
    bool          notAdded(const QString& what = "");
    void          checkIfAdded(const QString& what = "") { notAdded(what); };
    bool          notFound(const QString& what = "");
    bool          noSuccess() { return notFound(); };
    bool          handleTag(QStringList& row);
    QString       makeUnique(const QString& key);
    void          buildPair(QString& key, QString& value, const QString& line);
    void          prepare();

    void          setSymbolTuple(); // mSymbol
    bool          setFiIdByAnySymbol(const QString faultTxt = "");
    bool          setFiIdBySymbol(const QString& symbol, const QString faultTxt = "");
    bool          setMarketId(const QString& market, const QString faultTxt = "");
    bool          setCurrencyId(const QString& curr);
    bool          setDepotId(const QString faultTxt = "");
    bool          setQualityId(); // Well, its not realy an ID, but could

    void          addFiType();
    void          addSymbolType();
    void          addMarket();
    void          addFi();
    void          addSymbol();
    void          addUnderlying();
    void          addCompList();
    void          addEODBar();
    void          addSplit();
    void          addBroker();

    // User data imports
    void          addCO();
    void          addGroup();
    void          addDepot();
    void          addDepotPos();
    void          addAccountPos();
    void          addOrder();

    QString       mOrigData;           // Copy of the data at import(), FIXME:unused
    QList<QStringList> mPendingData;   // Collect data till committed
    QHash<QString, QString> mData;     // The split mOrigData associated to the [Header]
    QStringList   mHeader;             // Must be a stringlist, we need the positions of header data
    QStringList   mHeaderExpanded;     // Is mHeader + auto added keys, e.g. when a KnownSymbolType is used
    QSet<QString> mToDo;               // Holds notes, which job is todo to avoid redundant if()

    QString       mMustBeUnique;
    QStringList   mAllSymbolTypes;     // All possible symbol types
    QStringList   mKnownSymbolTypes;
    QHash<QString, bool> mKnownSTisProvider; // SymbolType is...?
    int           mTotalSymbolCount;
    QStringList   mUsedRefSymbols;     // Reference symbols, don't install
    QStringList   mUsedKnownSymbols;   // None provider symbols which has to be installed
    int           mSymbolXCount;       // How many SymbolX: Symbol0, Symbol1, Symbol2...
    QStringList   mAllUsedSymbols;     // mUsedRefSymbols + mUsedKnownSymbols + mUsedSymbols
    SymbolTuple*  mSymbol;
    QTextStream   mConsole;
    QHash<QString, int> mId;           // Hold Fi, Market etc IDs
    QTime         mRolex;
    bool          mMakeNameNice;
    QStringList   mNiceSearch;
    QStringList   mNiceReplace;

    int           mLineNo;              // Hold the line number which was actual read
    QQueue<int>   mDataLineNo;          // Is cut to max 2 numbers
    int           mByteCount;
    int           mDataR;               // Data Read
    int           mDataW;               // Data Written
    QString       mImportData;
    QStringList   mHint;

    Validator*    mValid;

  private:

};

#endif
