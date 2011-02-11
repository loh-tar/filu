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

#include "DataTupleSet.h"
#include <QDebug>


DataTupleSet::DataTupleSet()
{
  Index      = -1;
  MaxIndex   = -1;
  RangeFrom  = -1;
  RangeTo    =  MaxIndex;
  mThisBars  =  0;
}

DataTupleSet::~DataTupleSet()
{
  QHashIterator<QString, DataTuple*> i(DataSet);
  while (i.hasNext())
  {
    i.next();
    delete i.value();
  }
}

bool DataTupleSet::next()
{
  if(Index < RangeTo)
  {
    ++Index;
    return true;
  }

  return false;
}

void DataTupleSet::rewind(int start /* = -1 */)
{
  if((start < 0) or (start > MaxIndex)) Index = RangeFrom - 1;
  else Index = start;
}

int DataTupleSet::findDate(const QDate& date)
{
  return mThisBars->findDate(date);
}

bool DataTupleSet::append(const QString& key)
{
  if(DataSet.contains(key))
  {
    //qDebug() << "DataTupleSet::append() key already exist:" << key;
    return false;
  }

  DataTuple* dt = new DataTuple(dataTupleSize(), key);

  while(dt->next()) dt->setValue(0.0); // Needed e.g. by CalcWatchDogs.cpp

  return append(dt);
}

bool DataTupleSet::append(DataTuple* dt)
{
  if(DataSet.contains(dt->Name))
  {
    //qDebug() << "DataTupleSet::append() key already exist:" << dt->Name;
    return false;
  }

  if(DataSet.isEmpty())
  {
    MaxIndex = dt->size() - 1;
    Index = 0;
    setRange();
  }

  DataSet.insert(dt->Name, dt);
  dt->quitAutonomy(this);

  return true;
}

bool DataTupleSet::appendBarTuple(BarTuple* bars, const QString& fiName /* = "THIS"*/)
{
  QString prefix;
  if(!fiName.compare("THIS"))
  {
    mThisBars = bars;
    prefix = "";
  }
  else prefix = fiName + ".";

  if(DataSet.contains(prefix + "OPEN")) return false;

  if(!bars) return false;
  int size = bars->count();

  if(DataSet.isEmpty())
  {
    MaxIndex = size - 1;
    Index = 0;
    setRange();
  }

  DataTuple* dt;

  dt = new DataTuple(prefix + "OPEN", bars->mOpen, this);
  DataSet.insert(prefix + "OPEN", dt);

  dt = new DataTuple(prefix + "HIGH", bars->mHigh, this);
  DataSet.insert(prefix + "HIGH", dt);

  dt = new DataTuple(prefix + "LOW", bars->mLow, this);
  DataSet.insert(prefix + "LOW", dt);

  dt = new DataTuple(prefix + "CLOSE", bars->mClose, this);
  DataSet.insert(prefix + "CLOSE", dt);

  dt = new DataTuple(prefix + "VOLUME", bars->mVolume, this);
  DataSet.insert(prefix + "VOLUME", dt);

  dt = new DataTuple(prefix + "OPINT", bars->mOpenInterest, this);
  DataSet.insert(prefix + "OPINT", dt);

  rewind();

  mFiIds.insert(fiName, bars->mFiId);
  mMarketIds.insert(fiName, bars->mMarketId);

  return true;
}

bool DataTupleSet::getValue(const QString& name, double& value)
{
  if(!DataSet.contains(name))
  {
    qDebug() << "DataTupleSet::getValue: Key name not found:" << name;
    return false;
  }
  return DataSet.value(name)->getValue(value);
}

bool DataTupleSet::setValue(const QString& name, double value)
{
  if(!DataSet.contains(name)) return false;
  DataSet.value(name)->setValue(value);
  return true;
}

bool DataTupleSet::getColor(const QString& name, QColor& color)
{
  if(!DataSet.contains(name)) return false;
  return DataSet.value(name)->getColor(color);
}

void DataTupleSet::setColor(const QString& name, const QString& color)
{
  if(!DataSet.contains(name)) return;
  DataSet.value(name)->setColor(color);
}

void DataTupleSet::setEvenColor(const QString& name, const QString& color)
{
  if(!DataSet.contains(name)) return;
  DataSet.value(name)->setEvenColor(color);
}

void DataTupleSet::getDate(QDate& date)
{
  if((Index < 0) or (Index > MaxIndex))
  {
    date = QDate();
    return;
  }

  mThisBars->rewind(Index);
  date = mThisBars->date();
}

void DataTupleSet::getTime(QTime& time)
{
  mThisBars->rewind(Index);
  time = mThisBars->time();
}

void DataTupleSet::getDateRange(QDate& fromDate, QDate& toDate)
{
  mThisBars->rewind(RangeFrom);
  fromDate = mThisBars->date();

  mThisBars->rewind(MaxIndex);
  toDate = mThisBars->date();
}

double* DataTupleSet::valueArray(const QString& name)
{
  return DataSet.value(name)->Value;
}

void DataTupleSet::setValidRange(const QString& name, int first, int count)
{
  DataSet.value(name)->firstValid = first;
  DataSet.value(name)->countValid = count;
}

void DataTupleSet::getValidRange(const QString& name, int& first, int& count)
{
  first = DataSet.value(name)->firstValid;
  count = DataSet.value(name)->countValid;
}

void DataTupleSet::setRange(int from /* = -1 */, int count /* = -1 */)
{
  if(from < 0 or from > MaxIndex) RangeFrom = 0;
  else RangeFrom = from;

  int to = count + from;
  if(count < 0 or to > MaxIndex) RangeTo = MaxIndex;
  else RangeTo = to;

  rewind();
}

void DataTupleSet::getExtrema(const QString& name, double& min, double& max)
{
  if(!DataSet.contains(name)) return;

  DataSet.value(name)->getExtrema(min, max);
}

int DataTupleSet::getVariableNames(QStringList& list)
{
  list = DataSet.keys();
  return DataSet.size();
}

int DataTupleSet::dataTupleSize()
{
  return MaxIndex + 1;
}

void DataTupleSet::setNeededBars(const QString& name, int value)
{
  if(!DataSet.contains(name)) return; // In case name is a constant (number)

  DataSet.value(name)->mNeedsBars = value;
}

int DataTupleSet::neededBars(const QString& name)
{
  if(!DataSet.contains(name)) return 1; // In case name is a constant (number)

  return DataSet.value(name)->mNeedsBars;
}

bool DataTupleSet::getIDs(const QString& alias, int& fiId, int& marketId)
{
  if(!mFiIds.contains(alias))     return false;
  if(!mMarketIds.contains(alias)) return false;

  fiId     = mFiIds.value(alias);
  marketId = mMarketIds.value(alias);

  return true;
}
