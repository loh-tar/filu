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

#include "Validator.h"

#include "FiluU.h"

Validator::Validator(FClass* parent)
         : FClass(parent, FUNC)
{}

Validator::~Validator()
{}

QString Validator::sOrderLimit(const QString& limitTxt)
{
  double limit = dOrderLimit(limitTxt);

  if(0.0 == limit) return "Best";

  return limitTxt;
}

double Validator::dOrderLimit(const QString& limitTxt)
{
  bool   ok;
  double limit = limitTxt.toDouble(&ok);

  if(ok)
  {
    if(limit < 0.0) error(FUNC, tr("Order limit can't be negative."));
  }
  else
  {
    if(limitTxt.compare("Best", Qt::CaseInsensitive) == 0) return 0.0;

    error(FUNC, tr("Order limit '%1' is not valid").arg(limitTxt));
  }

  return -1.0; // Not valid
}

QString Validator::sOrderType(const QString& type)
{
  int id = iOrderType(type);

  if(id < 0) return type; // Not valid

  return mFilu->orderType(id);
}

int Validator::iOrderType(const QString& type)
{
  int id = mFilu->orderType(type);

  addErrors(mFilu->errors());

  return id;
}

QString Validator::sOrderStatus(const QString& status)
{
  int id = iOrderStatus(status);

  if(id < 0) return status; // Not valid

  return mFilu->orderStatus(id);
}

int Validator::iOrderStatus(const QString& status)
{
  int id = mFilu->orderStatus(status);

  addErrors(mFilu->errors());

  return id;
}

QString Validator::sAccPostType(const QString& type)
{
  int id = iAccPostType(type);

  if(id < 0) return type; // Not valid

  return mFilu->accPostingType(id);
}

int Validator::iAccPostType(const QString& type)
{
  int id = mFilu->accPostingType(type);

  addErrors(mFilu->errors());

  return id;
}

QString Validator::sSplitPrePost(const QString& split)
{
  dSplitPrePost(split);

  return split;
}

double Validator::dSplitPrePost(const QString& split)
{
  double  pre;
  double  post;
  double  ratio = 0.0;
  bool    ok;

  QStringList sl = split.split(":", QString::SkipEmptyParts);
  if(sl.size() < 2)
  {
    error(FUNC, "Ratio must be SplitPre:Post.");
    return ratio;
  }

  pre = sl[0].toDouble(&ok);
  if(pre == 0.0)
  {
    error(FUNC, "SplitPre:Post, Pre must not 0.");
    return ratio;
  };

  post = sl[1].toDouble(&ok);
  if(post == 0.0)
  {
    error(FUNC, "SplitPre:Post, Post must not 0.");
    return ratio;
  }

  ratio = pre / post;

  if(ratio < 0)
  {
    error(FUNC, "Ratio must not negative.");
    return 0.0;
  }

  ratio = pre / post;
  return ratio;
}

QString Validator::sSplitPostPre(const QString& split)
{
  dSplitPostPre(split);

  return split;
}

double Validator::dSplitPostPre(const QString& split)
{
  double  pre;
  double  post;
  double  ratio = 0.0;
  bool    ok;

  QStringList sl = split.split(":", QString::SkipEmptyParts);
  if(sl.size() < 2)
  {
    error(FUNC, "Ratio must be SplitPost:Pre.");
    return ratio;
  }

  pre = sl[0].toDouble(&ok);
  if(pre == 0.0)
  {
    error(FUNC, "SplitPost:Pre, Post must not 0.");
    return ratio;
  };

  post = sl[1].toDouble(&ok);
  if(post == 0.0)
  {
    error(FUNC, "SplitPost:Pre, Pre must not 0.");
    return ratio;
  };

  ratio = pre / post;

  if(ratio < 0)
  {
    error(FUNC, "Ratio must not negative.");
    return 0.0;
  }

  return ratio;
}

QString Validator::sQuality(const QString& quality)
{
  int id = iQuality(quality);

  if(id < 0) return quality; // Not valid

  return mFilu->quality(id);
}

int Validator::iQuality(const QString& quality)
{
  int id = mFilu->quality(quality);

  addErrors(mFilu->errors());

  return id;
}
