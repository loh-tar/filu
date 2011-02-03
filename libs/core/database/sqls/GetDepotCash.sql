*  file name: GetDepotCash.sql
*  Used in function: FiluU::
*
*  Purpose:
*     Fetch the cash of a depot at given date
*
*  Inputs: (variable names are important and begins with a colon)
*     :depotId
*     :date
*
*  Outputs: (order is important)
*     cash

-- GetDepotCash.sql
SELECT COALESCE(sum(bvalue), 0.0) as "Cash"
  FROM :user.account
  WHERE depot_id = :depotId and bdate <= :date




-- GetNeededCashForOpenOrders.sql
SELECT

     sum( -pieces *
            CASE WHEN olimit=0.0
                      THEN ( select
                              --b.qclose
                              avg(b.qclose)
                              from filu.eodbar  b
                              where
                              fi_id = o.fi_id --and market_id = o.market_id
                              and
                              qdate BETWEEN o.odate-5 and o.vdate
                            )
                      ELSE olimit
            END
        ) as "required"
        , m.currency_fi_id

  FROM user_lot.order AS o
    LEFT JOIN filu.fi AS fi USING(fi_id)
    LEFT JOIN filu.market AS m USING(market_id)

  WHERE depot_id = 4
        --and :date BETWEEN o.odate and o.vdate
        and o.buy
        and o.status > 3

  GROUP BY m.currency_fi_id





-- GetDepotCash.sql
SELECT
sum(acc.cash), max(acc.bdate)
--blub.cash
-- *
FROM
(
    SELECT bvalue as "cash", bdate
      FROM user_lot.account AS a
      WHERE depot_id = 4 and bdate <= '2012-01-01'

      UNION SELECT 200.00, '2010-05-01'
) AS acc




-- semi final GetDepotCash.sql
select
sum(acc.cash), max(acc.bdate)
--blub.cash
-- *
from(
    SELECT bvalue as "cash", bdate--, d.currency
      FROM user_lot.account AS a
      NATURAL JOIN user_lot.depot AS d
      --NATURAL JOIN filu.market AS m
      WHERE depot_id = 4 and bdate <= '2012-01-01'

      UNION SELECT --200.00, '2010-05-01', 0, 0

     sum( -pieces *
            CASE WHEN olimit=0.0
                      THEN ( select
                              --b.qclose
                              avg(b.qclose)
                              from filu.eodbar  b
                              where
                              fi_id = o.fi_id --and market_id = o.market_id
                              and
                              qdate BETWEEN o.odate-5 and o.vdate
                            )
                      ELSE olimit
            END
        ) as "required"
        , o.odate
        --, m.currency_fi_id

  FROM user_lot.order AS o
    LEFT JOIN filu.fi AS fi USING(fi_id)
    LEFT JOIN filu.market AS m USING(market_id)

  WHERE depot_id = 4
        --and :date BETWEEN o.odate and o.vdate
        and o.buy
        and o.status > 3

  GROUP BY m.currency_fi_id, o.odate

) as acc