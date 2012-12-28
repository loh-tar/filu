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

#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include "FClass.h"

class Validator : public FClass
{
  public:
                  Validator(FClass* parent);
    virtual      ~Validator();

    // String returning functions always return a named value
    QString       sOrderLimit(const QString& limit);
    double        dOrderLimit(const QString& limit);
    QString       sOrderType(const QString& type);
    int           iOrderType(const QString& type);
    QString       sOrderStatus(const QString& status);
    int           iOrderStatus(const QString& status);
    QString       sAccPostType(const QString& type);
    int           iAccPostType(const QString& type);

    QString       sSplitPrePost(const QString& split);
    double        dSplitPrePost(const QString& split);
    QString       sSplitPostPre(const QString& split);
    double        dSplitPostPre(const QString& split);

    QString       sQuality(const QString& quality);
    int           iQuality(const QString& quality);
  protected:

  private:

};

#endif
