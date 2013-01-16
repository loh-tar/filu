--
--   This file is part of Filu.
--
--   Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
--
--   Filu is free software: you can redistribute it and/or modify
--   it under the terms of the GNU General Public License as published by
--   the Free Software Foundation, either version 2 of the License, or
--   (at your option) any later version.
--
--   Filu is distributed in the hope that it will be useful,
--   but WITHOUT ANY WARRANTY; without even the implied warranty of
--   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--   GNU General Public License for more details.
--
--   You should have received a copy of the GNU General Public License
--   along with Filu. If not, see <http://www.gnu.org/licenses/>.
--
--
--
CREATE SCHEMA :user AUTHORIZATION :dbuser;
-- GRANT ALL ON SCHEMA :user TO :dbuser;
--
--
--
CREATE TABLE :user.co(
  co_id      serial4         PRIMARY KEY,
  fi_id      int4            NOT NULL,
  market_id  int4            NOT NULL,
  co_plot    varchar(30)     NOT NULL,
  co_date    date            NOT NULL,
  co_type    varchar(20)     NOT NULL,
  co_argv    text,

  FOREIGN KEY(fi_id) REFERENCES :filu.fi(fi_id)  ON DELETE CASCADE,
  FOREIGN KEY(market_id) REFERENCES :filu.market(market_id) ON DELETE CASCADE,
  UNIQUE(fi_id, market_id, co_plot, co_date, co_type)
);
--
--
--
CREATE TABLE :user.group(
  group_id          serial4         PRIMARY KEY,
  caption           varchar(20)     NOT NULL,
  mothergroup_id    int4            NOT NULL DEFAULT 0,

  UNIQUE(mothergroup_id, caption)
);

--ALTER TABLE :user.group OWNER TO :dbuser;
--
--
--
CREATE TABLE :user.gmember(
  gmember_id        serial4         PRIMARY KEY,
  group_id          int4            NOT NULL,
  fi_id             int4            NOT NULL,

  FOREIGN KEY(group_id) REFERENCES :user.group ON DELETE CASCADE,
  UNIQUE(group_id, fi_id)
);

--ALTER TABLE :user.gmember OWNER TO :dbuser;
--
--
--
CREATE TABLE :user.ts
(
  ts_id         serial4     PRIMARY KEY,
  ts_strategy   char(32)    NOT NULL,
  ts_fdate      date        NOT NULL,
  ts_tdate      date        NOT NULL,
  ts_rule       text        NOT NULL,
  ts_indicator  text        NOT NULL
);

--ALTER TABLE :user.ts OWNER TO :dbuser;
--
--
--
CREATE TABLE :user.tsr
(
  tsr_id         serial4     PRIMARY KEY,
  ts_id          int4        NOT NULL,
  fi_id          int4        NOT NULL,
  market_id      int4        NOT NULL,
  wltp           float4      NOT NULL, -- Won long trades %
  lltp           float4      NOT NULL, -- Lost long trades %
  agwlp          float4      NOT NULL, -- Avg gain of won lt %
  alltp          float4      NOT NULL, -- Avg lost of lost lt %
 -- FIXME: add short stuff
  tpp            float4      NOT NULL, -- Total Performance %
  score          float4      NOT NULL, -- ln(exp(won long trades * Avg gain of won lt%)/exp(lost long trades * Avg lost of lost lt%))

  FOREIGN KEY(ts_id) REFERENCES :user.ts(ts_id) ON DELETE CASCADE,
  FOREIGN KEY(fi_id) REFERENCES :filu.fi(fi_id) ON DELETE CASCADE,
  FOREIGN KEY(market_id) REFERENCES :filu.market(market_id) ON DELETE CASCADE
);

--ALTER TABLE :user.tsr OWNER TO :dbuser;
--
--
--
CREATE TABLE :user.depot(
  depot_id   serial4         PRIMARY KEY,
  caption    varchar(50)     NOT NULL,
  trader     varchar(50)     NOT NULL,
  owner      varchar(50)     NOT NULL,
  broker_id  int4            NOT NULL,

FOREIGN KEY(broker_id) REFERENCES :filu.broker(broker_id) ON DELETE RESTRICT
);

CREATE UNIQUE INDEX depot_unique_caption ON :user.depot(
  lower(caption), lower(owner)
);
--
--
--
CREATE TABLE :user.depotpos(
  depotpos_id     serial4   PRIMARY KEY,
  depot_id        int4      NOT NULL,
  pdate           date      NOT NULL,
  fi_id           int4      NOT NULL,
  pieces          int4      NOT NULL,
  price           float4    NOT NULL,
  market_id       int4      NOT NULL,  -- Looks stupid but 'currency' would make things pretty complicate.
                                       -- How should you decide WHERE is the right place to sell?
  note            text,

FOREIGN KEY(depot_id) REFERENCES :user.depot(depot_id) ON DELETE CASCADE,
FOREIGN KEY(fi_id) REFERENCES :filu.fi(fi_id) ON DELETE RESTRICT,
FOREIGN KEY(market_id) REFERENCES :filu.market(market_id) ON DELETE RESTRICT
);
--
--
--
CREATE TABLE :user.order(
  order_id        serial4   PRIMARY KEY,
  depot_id        int4      NOT NULL,
  odate           date      NOT NULL, -- order date
  vdate           date      NOT NULL, -- valid date
  fi_id           int4      NOT NULL,
  pieces          int4      NOT NULL, -- ordered pieces
  olimit          float4    NOT NULL, -- order limit
  buy             bool      NOT NULL, -- buy=true, sell=false
  market_id       int4      NOT NULL,
  status          int2      NOT NULL, -- see FiluU.h
  note            varchar(100),

FOREIGN KEY(depot_id) REFERENCES :user.depot(depot_id) ON DELETE CASCADE,
FOREIGN KEY(fi_id) REFERENCES :filu.fi(fi_id) ON DELETE RESTRICT,
FOREIGN KEY(market_id) REFERENCES :filu.market(market_id) ON DELETE RESTRICT
);
--
--
--
CREATE TABLE :user.account(
  account_id      serial4      PRIMARY KEY,
  depot_id        int4         NOT NULL,
  bdate           date         NOT NULL, -- booking date
  btype           int2         NOT NULL, -- see FiluU.h
  btext           varchar(100) NOT NULL, -- valid date
  bvalue          float4       NOT NULL, -- booking value/amount posted

FOREIGN KEY(depot_id) REFERENCES :user.depot(depot_id) ON DELETE CASCADE
);
--
--
--
