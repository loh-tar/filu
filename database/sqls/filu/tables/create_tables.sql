/*
 *   This file is part of Filu.
 *
 *   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
 *
 *   Filu is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Filu is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Filu. If not, see <http://www.gnu.org/licenses/>.
 */

CREATE TABLE :filu.error(
  error_id     serial4       PRIMARY KEY,
  caption      varchar(30)   NOT NULL,
  etext        varchar(100)  NOT NULL
);

CREATE UNIQUE INDEX error_unique_caption ON :filu.error(
  lower(caption)
);
--
-- END OF CREATE TABLE :filu.error
--
CREATE TABLE :filu.ftype(
  ftype_id     serial4       PRIMARY KEY,
  caption      varchar(30)   NOT NULL,
  quality      int2          NOT NULL DEFAULT 2 -- bronze, as tempo classified data
);

CREATE UNIQUE INDEX ftype_unique_caption ON :filu.ftype(
  lower(caption)
);
--
-- END OF CREATE TABLE :filu.ftype
--
CREATE TABLE :filu.stype(
  stype_id     serial4      PRIMARY KEY,
  caption      varchar(30)  NOT NULL,
  seq          int4         NOT NULL,
  isprovider   boolean      NOT NULL DEFAULT false,
  quality      int2         NOT NULL DEFAULT 2 -- bronze, as tempo classified data
);

CREATE UNIQUE INDEX stype_unique_caption ON :filu.stype(
  lower(caption)
);
--
-- END OF CREATE TABLE :filu.stype
--
CREATE TABLE :filu.fi(
  fi_id         serial4         PRIMARY KEY,
  caption       varchar(100)    NOT NULL,
  deletedate    date            NOT NULL DEFAULT '3000-01-01',
  ftype_id      int4            NOT NULL,
  quality       int2            NOT NULL DEFAULT 2, -- bronze, as tempo classified data

  FOREIGN KEY(ftype_id) REFERENCES :filu.ftype(ftype_id) ON DELETE CASCADE
);

CREATE UNIQUE INDEX fi_unique_caption ON :filu.fi(
  lower(caption)
);
--
-- END OF CREATE TABLE :filu.fi
--
CREATE TABLE :filu.underlying(
  underlying_id     serial4     PRIMARY KEY,
  fi_id             int4        NOT NULL,
  weight            float4      NOT NULL DEFAULT 1,
  underlying_fi_id  int4        NOT NULL,
  quality           int2        NOT NULL DEFAULT 2, -- bronze, as tempo classified data

  FOREIGN KEY(underlying_fi_id) REFERENCES :filu.fi(fi_id) ON DELETE CASCADE,
  FOREIGN KEY(fi_id) REFERENCES :filu.fi(fi_id) ON DELETE CASCADE,
  UNIQUE(underlying_fi_id, fi_id)
);
--
-- END OF CREATE TABLE :filu.underlying
--
CREATE TABLE :filu.market(
  market_id         serial4     PRIMARY KEY,
  caption           varchar(30) NOT NULL,
  currency_fi_id    int4        NOT NULL,
  opentime          time        NOT NULL DEFAULT '00:00:00',
  closetime         time        NOT NULL DEFAULT '23:59:59',
  quality           int2        NOT NULL DEFAULT 2, -- bronze, as tempo classified data

  FOREIGN KEY(currency_fi_id) REFERENCES :filu.fi(fi_id) ON DELETE RESTRICT
);

CREATE UNIQUE INDEX market_unique_caption ON :filu.market(
  lower(caption)
);
--
-- END OF CREATE TABLE :filu.market
--
CREATE TABLE :filu.symbol(
  symbol_id        serial4      PRIMARY KEY,
  market_id        int4         NOT NULL,
  stype_id         int4         NOT NULL,
  fi_id            int4         NOT NULL,
  caption          varchar(30)  NOT NULL,
  issuedate        date         NOT NULL DEFAULT '1000-01-01',
  maturitydate     date         NOT NULL DEFAULT '3000-01-01',
  quality          int2         NOT NULL DEFAULT 2, -- bronze, as tempo classified data

  FOREIGN KEY(fi_id) REFERENCES :filu.fi(fi_id) ON DELETE CASCADE,
  FOREIGN KEY(market_id) REFERENCES :filu.market(market_id) ON DELETE CASCADE,
  FOREIGN KEY(stype_id) REFERENCES :filu.stype(stype_id) ON DELETE CASCADE,
-- It is possible that the same caption is used by different providers, and
-- theoretical is it possible that these same caption is than used for different
-- FIs in the rare case that both use own running ID numbers like www.onvista.de
  UNIQUE(caption, market_id, stype_id),
  UNIQUE(fi_id, market_id, stype_id)
);

-- Here not UNIQUE, because of above UNIQUE(caption, market_id, stype_id).
CREATE INDEX symbol_lower_caption ON :filu.symbol(
  lower(caption)
);
--
-- END OF CREATE TABLE :filu.symbol
--
CREATE TABLE :filu.offday(
  offday_id      serial4    PRIMARY KEY,
  market_id      int4       NOT NULL,
  offday         date       NOT NULL,
  is_openday     boolean    NOT NULL DEFAULT false, -- to prevent to insert an offday by script
  quality        int2       NOT NULL DEFAULT 2,     -- bronze, as tempo classified data

  FOREIGN KEY(market_id) REFERENCES :filu.market(market_id) ON DELETE CASCADE,
  UNIQUE(market_id, offday)
);
--
-- END OF CREATE TABLE :filu.offday
--
-- Here is how a provider call a market
CREATE TABLE :filu.msymbol(
  msymbol_id       serial4      PRIMARY KEY,
  stype_id         int4         NOT NULL,
  market_id        int4         NOT NULL,
  caption          varchar(30)  NOT NULL,
  quality          int2         NOT NULL DEFAULT 2, -- bronze, as tempo classified data

  FOREIGN KEY(stype_id) REFERENCES :filu.stype(stype_id) ON DELETE CASCADE,
  FOREIGN KEY(market_id) REFERENCES :filu.market(market_id) ON DELETE CASCADE
);

CREATE INDEX msymbol_lower_caption ON :filu.msymbol(
  lower(caption)
);
--
-- END OF CREATE TABLE :filu.msymbol
--
CREATE TABLE :filu.eodbar(
  eodbar_id     serial4     PRIMARY KEY,
  fi_id         int4        NOT NULL,
  market_id     int4        NOT NULL,
  qdate         date        NOT NULL,
  qopen         float4      NOT NULL,
  qhigh         float4      NOT NULL,
  qlow          float4      NOT NULL,
  qclose        float4      NOT NULL,
  qvol          float8      NOT NULL DEFAULT 0.0,
  qoi           int4        NOT NULL DEFAULT 0,
  quality       int2        NOT NULL DEFAULT 2, -- bronze, as tempo classified data

  FOREIGN KEY(fi_id) REFERENCES :filu.fi(fi_id) ON DELETE CASCADE,
  FOREIGN KEY(market_id) REFERENCES :filu.market(market_id) ON DELETE CASCADE,
  UNIQUE(fi_id, market_id, qdate)
);

CREATE INDEX eodbar_fi_market_date ON :filu.eodbar( fi_id, market_id, qdate );

--
-- END OF CREATE TABLE :filu.eodbar
--
CREATE TABLE :filu.split(
  split_id  serial4         PRIMARY KEY,
  fi_id     int4            NOT NULL,
  sdate     date            NOT NULL,
  sratio    float4          NOT NULL,
  scomment  varchar(30),
  quality   int2            NOT NULL DEFAULT 2, -- bronze, as tempo classified data

  FOREIGN KEY(fi_id) REFERENCES :filu.fi(fi_id) ON DELETE CASCADE,
  UNIQUE(fi_id, sdate)
);
--
-- END OF CREATE TABLE :filu.dividend
--
CREATE TABLE :filu.dividend(
  dividend_id  serial4      PRIMARY KEY,
  fi_id        int4         NOT NULL,
  ddate        date         NOT NULL,
  dpayout      float4       NOT NULL,
  dcomment     varchar(30),
  quality      int2         NOT NULL DEFAULT 2, -- bronze, as tempo classified data

  FOREIGN KEY(fi_id) REFERENCES :filu.fi(fi_id) ON DELETE CASCADE,
  UNIQUE(fi_id, ddate)
);
--
-- END OF CREATE TABLE :filu.dividend
--
CREATE TABLE :filu.broker(
  broker_id       serial4       PRIMARY KEY,
  caption         varchar(50)   NOT NULL,
  currency_fi_id  int4          NOT NULL,
  feeformula      varchar(100)  NOT NULL,
  quality         int2          NOT NULL DEFAULT 2, -- bronze, as tempo classified data

  FOREIGN KEY(currency_fi_id) REFERENCES :filu.fi(fi_id) ON DELETE RESTRICT
);

CREATE UNIQUE INDEX broker_unique_caption ON :filu.broker(
  lower(caption)
);
--
-- END OF CREATE TABLE :filu.broker
--
