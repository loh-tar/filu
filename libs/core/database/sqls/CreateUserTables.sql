--
--   This file is part of Filu.
--
--   Copyright (C) 2007, 2010  loh.tar@googlemail.com
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

-- ***********************************************************************
-- *
-- *          All stuff related to user data
-- *
-- ***********************************************************************
--
--
-- *** create the user schema ***
CREATE SCHEMA :user
  AUTHORIZATION :filu;
GRANT ALL ON SCHEMA :user TO :filu;
--
-- ***********************************************************************
-- *
-- *          All stuff related to chart objects
-- *
-- ***********************************************************************
--
--
-- *** create the chart object table ***
----DROP TABLE :user.co;
CREATE TABLE :user.co(
  co_id      SERIAL8         NOT NULL,
  fi_id      INT8            NOT NULL,
  market_id  INT8            NOT NULL,
  co_plot    VARCHAR(30)     NOT NULL,
  co_date    DATE            NOT NULL,
  co_type    VARCHAR(20)     NOT NULL,
  co_argv    TEXT,
  PRIMARY KEY(co_id),
  FOREIGN KEY(fi_id) REFERENCES :filu.fi(fi_id) ON DELETE NO ACTION,
  FOREIGN KEY(market_id) REFERENCES :filu.market(market_id),
  UNIQUE  (fi_id, market_id, co_plot, co_date, co_type)
);


CREATE OR REPLACE FUNCTION :user.co_duplicate()
  RETURNS "trigger" AS
$BODY$

DECLARE
isExist  int4;

BEGIN

  IF new.co_id < 1
  THEN
    -- check if already exist
    SELECT INTO isExist co_id
      FROM :user.co
    WHERE
          fi_id      = new.fi_id
      AND market_id  = new.market_id
      AND co_plot    = new.co_plot
      AND co_date    = new.co_date
      AND co_type    = new.co_type
    LIMIT 1; -- Just to be on the safe side

    IF isExist IS NULL
    THEN
      new.co_id := nextval(':user.co_co_id_seq');
      RETURN new;
    END IF;

  ELSE
   -- check if already exist
    SELECT INTO isExist co_id
      FROM :user.co
    WHERE co_id = new.co_id;

    IF isExist IS NULL THEN RETURN new; END IF;
  END IF;

  UPDATE :user.co
  SET
    co_plot    = new.co_plot,
    co_date    = new.co_date,
    co_type    = new.co_type,
    co_argv    = new.co_argv
  WHERE
    co_id = new.co_id;

  RETURN NULL;

END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
ALTER FUNCTION :user.co_duplicate() OWNER TO :filu;


CREATE TRIGGER :user_co_duplicate BEFORE INSERT
  ON :user.co FOR EACH ROW
  EXECUTE PROCEDURE :user.co_duplicate();
--
--
-- ***********************************************************************
-- *
-- *          All stuff related to groups
-- *
-- ***********************************************************************
--
--
-- *** create the group table ***
--
-- DROP TABLE :user.group;
CREATE TABLE :user.group
(
  group_id bigserial NOT NULL,
  caption varchar(20) NOT NULL,
  mothergroup_id int8 DEFAULT 0,
  CONSTRAINT group_pkey PRIMARY KEY (group_id),
  CONSTRAINT group_mid_caption_key UNIQUE (mothergroup_id, caption)
)
WITHOUT OIDS;
ALTER TABLE :user.group OWNER TO :filu;

--
--
-- *** create the group member table ***
--
-- DROP TABLE :user.gmember;
CREATE TABLE :user.gmember
(
  gmember_id bigserial NOT NULL,
  group_id int8 NOT NULL,
  fi_id int8 NOT NULL,
  CONSTRAINT gmember_pkey PRIMARY KEY (gmember_id),
  CONSTRAINT group_fkey FOREIGN KEY (group_id) REFERENCES :user.group,
  CONSTRAINT gmember_gid_fid_key UNIQUE (group_id, fi_id)
)
WITHOUT OIDS;
ALTER TABLE :user.gmember OWNER TO :filu;

-- Function: :user.gmember_duplicate()
-- DROP FUNCTION :user.gmember_duplicate();
CREATE OR REPLACE FUNCTION :user.gmember_duplicate()
  RETURNS "trigger" AS
$BODY$

DECLARE
isExist  int4;

BEGIN

  -- check if already exist
  SELECT INTO isExist gmember_id
    FROM :user.gmember
  WHERE group_id = new.group_id
    AND fi_id    = new.fi_id
    LIMIT 1; -- Just to be on the safe side

  -- if yes, don't insert
  IF isExist IS NOT NULL
  THEN
    RETURN NULL;
  END IF;

  RETURN new;

END;

$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
ALTER FUNCTION :user.gmember_duplicate() OWNER TO :filu;

-- Trigger: :user_gmember_duplicate on :user.gmember
-- DROP TRIGGER :user_gmember_duplicate ON :user.gmember;
CREATE TRIGGER :user_gmember_duplicate
  BEFORE INSERT
  ON :user.gmember
  FOR EACH ROW
  EXECUTE PROCEDURE :user.gmember_duplicate();


-- Function: :user.group_duplicate()
-- DROP FUNCTION :user.group_duplicate();
CREATE OR REPLACE FUNCTION :user.group_duplicate()
  RETURNS "trigger" AS
$BODY$
DECLARE
rc int;

   BEGIN

      IF new.group_id = 0 THEN
          new.group_id := nextval(':user.group_group_id_seq');
          RETURN new;
      ELSE
         update :user.group set
           caption           = new.caption,
           mothergroup_id    = new.mothergroup_id
         where group_id      = new.group_id;

         GET DIAGNOSTICS rc = ROW_COUNT;

         IF rc > 0
         THEN RETURN NULL;
         ELSE RETURN new;
         END IF;

      END IF;

   END;

$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
ALTER FUNCTION :user.group_duplicate() OWNER TO :filu;

-- Trigger: :user_group_duplicate on :user.group
-- DROP TRIGGER :user_group_duplicate ON :user.group;
CREATE TRIGGER :user_group_duplicate
  BEFORE INSERT
  ON :user.group
  FOR EACH ROW
  EXECUTE PROCEDURE :user.group_duplicate();

--
--
-- Function: :user.group_insert(character varying)

-- DROP FUNCTION :user.group_insert(character varying);

CREATE OR REPLACE FUNCTION :user.group_insert(path character varying)
  RETURNS bigint AS
$BODY$
DECLARE
  gid     :user.group.group_id%TYPE;       --group id
  mid     :user.group.mothergroup_id%TYPE; --mother group id
  query   text;
  groups  text[];
  rec     RECORD;
  i       int;

BEGIN
  groups := regexp_split_to_array(path, '/');
  i   := 0;
  mid := 0;

LOOP
  i := i + 1;

  EXIT WHEN groups[i] IS NULL;
  CONTINUE WHEN groups[i]='';

  query := $$select * from :user.group g
            where lower(g.caption) = lower($$ || quote_literal(groups[i]) || $$)
            and mothergroup_id = $$ || quote_literal(mid);

  EXECUTE query INTO rec;
  gid := rec.group_id;

  IF gid IS NULL
  THEN
    gid := nextval(':user.group_group_id_seq');
    --RAISE INFO 'add new group %, g=%, m=%', groups[i], gid, mid;
    INSERT INTO :user.group(group_id, caption, mothergroup_id)
          VALUES(gid, groups[i], mid);
  END IF;

  mid := gid;

END LOOP;

RETURN mid;

END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE
  COST 100;

ALTER FUNCTION :user.group_insert(character varying) OWNER TO :filu;

-- Function: :user.group_delete()
-- DROP FUNCTION :user.group_delete();
CREATE OR REPLACE FUNCTION :user.group_delete()
  RETURNS "trigger" AS
$BODY$

BEGIN
      delete from :user.gmember
      where group_id = old.group_id;

      delete from :user.group
      where mothergroup_id = old.group_id;

      RETURN old;
END;

$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
ALTER FUNCTION :user.group_delete() OWNER TO :filu;


-- Trigger: :user_group_delete on :user.group
-- DROP TRIGGER :user_group_delete ON :user.group;
CREATE TRIGGER :user_group_delete
  BEFORE DELETE
  ON :user.group
  FOR EACH ROW
  EXECUTE PROCEDURE :user.group_delete();


-- Function: :user.group_id_to_path(bigint)
-- DROP FUNCTION :user.group_id_to_path(bigint);
CREATE OR REPLACE FUNCTION :user.group_id_to_path(id :user.group.group_id%TYPE)
  RETURNS CHARACTER VARYING AS
$BODY$
DECLARE
  gid     :user.group.group_id%TYPE; --mother group id
  query   TEXT;
  path    VARCHAR;
  rec     RECORD;

BEGIN
  path := $$$$; -- clear the path
  gid := id;

  WHILE (gid > 0)
  LOOP
    query := $$SELECT * FROM :user.group g WHERE g.group_id = $$ || quote_literal(gid);
    --RAISE INFO $$%$$, query;
    EXECUTE query INTO rec;
    path := $$/$$ || rec.caption || path;
    gid := rec.mothergroup_id;
    --RAISE INFO $$mid:% path:% gid:%$$, mid, path;
  END LOOP;

RETURN path;

END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE
  COST 100;

ALTER FUNCTION :user.group_id_to_path(:user.group.group_id%TYPE) OWNER TO :filu;

-- Function: :user.group_childs
-- DROP FUNCTION :user.group_childs;
CREATE OR REPLACE FUNCTION :user.group_childs(id bigint) RETURNS SETOF :user.group AS
$BODY$
DECLARE
    r :user.group%rowtype;
BEGIN
  FOR r IN select group_id, caption, mothergroup_id
              from :user.group
              where mothergroup_id = id
              order by group_id
  LOOP
    RETURN NEXT r;
    RETURN QUERY select * from :user.group_childs(r.group_id);
  END LOOP;

  RETURN;
END
$BODY$
LANGUAGE 'plpgsql' ;


--
--
-- ***********************************************************************
-- *
-- *          All stuff related to backtesting
-- *
-- ***********************************************************************
--
--
-- *** create the master table ts = trading strategy ***
--
-- Table: :user.ts
-- DROP TABLE :user.ts CASCADE;
CREATE TABLE :user.ts
(
  ts_id         SERIAL8     NOT NULL,
  ts_strategy   CHAR(32)    NOT NULL,
  ts_fdate      DATE        NOT NULL,
  ts_tdate      DATE        NOT NULL,
  ts_rule       TEXT        NOT NULL,
  ts_indicator  TEXT        NOT NULL,
  CONSTRAINT ts_pkey PRIMARY KEY (ts_id)
)
WITHOUT OIDS;
ALTER TABLE :user.ts OWNER TO :filu;
--
--
-- *** create the result table tsr = trading strategy results ***
--
-- Table: :user.tsr
-- DROP TABLE :user.tsr;
CREATE TABLE :user.tsr
(
  tsr_id         SERIAL8     NOT NULL,
  ts_id          SERIAL8     NOT NULL,
  fi_id          INT8        NOT NULL,
  market_id      INT8        NOT NULL,
  wltp           FLOAT       NOT NULL, -- Won long trades %
  lltp           FLOAT       NOT NULL, -- Lost long trades %
  agwlp          FLOAT       NOT NULL, -- Avg gain of won lt %
  alltp          FLOAT       NOT NULL, -- Avg lost of lost lt %
 -- FIXME: add short stuff
  tpp            FLOAT       NOT NULL, -- Total Performance %
  score          FLOAT       NOT NULL, -- ln(exp(won long trades * Avg gain of won lt%)/exp(lost long trades * Avg lost of lost lt%))

  CONSTRAINT tsr_pkey PRIMARY KEY (tsr_id),
  FOREIGN KEY(ts_id) REFERENCES :user.ts(ts_id) ON DELETE CASCADE,
  FOREIGN KEY(fi_id) REFERENCES :filu.fi(fi_id) ON DELETE CASCADE,
  FOREIGN KEY(market_id) REFERENCES :filu.market(market_id) ON DELETE CASCADE
 )
WITHOUT OIDS;
ALTER TABLE :user.tsr OWNER TO :filu;
--
--
-- ***********************************************************************
-- *
-- *          All stuff related to depots
-- *
-- ***********************************************************************
--
--
-- *** create the depot master table ***
--
--DROP TABLE :user.depot;
CREATE TABLE :user.depot(
  depot_id   SERIAL4       NOT NULL, --no bigserial, 4byte are enough
  caption    VARCHAR(50)   NOT NULL,
  trader     VARCHAR(50)   NOT NULL,
  owner      VARCHAR(50)   NOT NULL,
  currency   INT8          NOT NULL,
  broker_id  INT4          NOT NULL,
PRIMARY KEY(depot_id),
FOREIGN KEY(currency) REFERENCES :filu.fi(fi_id) ON DELETE NO ACTION,
FOREIGN KEY(broker_id) REFERENCES :filu.broker(broker_id) ON DELETE NO ACTION
);

--
--
-- *** create the depot positions table ***
--
--DROP TABLE :user.depotpos;
CREATE TABLE :user.depotpos(
  depotpos_id     SERIAL8   NOT NULL,
  depot_id        INT4      NOT NULL,
  pdate           DATE      NOT NULL,
  fi_id           INT8      NOT NULL,
  pieces          INT4      NOT NULL,
  price           FLOAT     NOT NULL,
  market_id       INT8      NOT NULL,  -- Looks stupid but 'currency' would make things pretty complicate.
                                       -- How should you decide where is the right place to sell?
PRIMARY KEY(depotpos_id),
FOREIGN KEY(depot_id) REFERENCES :user.depot(depot_id) ON DELETE CASCADE,
FOREIGN KEY(fi_id) REFERENCES :filu.fi(fi_id) ON DELETE NO ACTION,
FOREIGN KEY(market_id) REFERENCES :filu.market(market_id) ON DELETE NO ACTION
);

--
--
-- *** create the depot orders table ***
--
--DROP TABLE :user.order;
CREATE TABLE :user.order(
  order_id        SERIAL8   NOT NULL,
  depot_id        INT4      NOT NULL,
  odate           DATE      NOT NULL, -- order date
  vdate           DATE      NOT NULL, -- valid date
  fi_id           INT8      NOT NULL,
  pieces          INT4      NOT NULL, -- ordered pieces
  olimit          FLOAT     NOT NULL, -- order limit
  buy             BOOL      NOT NULL, -- buy=true, sell=false
  market_id       INT8      NOT NULL,
  status          INT2      NOT NULL, -- see FiluU.h
PRIMARY KEY(order_id),
FOREIGN KEY(depot_id) REFERENCES :user.depot(depot_id) ON DELETE CASCADE,
FOREIGN KEY(fi_id) REFERENCES :filu.fi(fi_id) ON DELETE NO ACTION,
FOREIGN KEY(market_id) REFERENCES :filu.market(market_id) ON DELETE NO ACTION
);

--
--
-- *** create the account table ***
--
--DROP TABLE :user.account;
CREATE TABLE :user.account(
  account_id      SERIAL8      NOT NULL,
  depot_id        INT4         NOT NULL,
  bdate           DATE         NOT NULL, -- booking date
  btype           INT2         NOT NULL, -- see FiluU.h
  btext           VARCHAR(100) NOT NULL, -- valid date
  bvalue          FLOAT        NOT NULL, -- booking value/amount posted
PRIMARY KEY(account_id),
FOREIGN KEY(depot_id) REFERENCES :user.depot(depot_id) ON DELETE CASCADE
);
