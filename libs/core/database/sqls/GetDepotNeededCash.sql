/*
 *  Purpose:
 *     Fetch the needed cash of a depot at given date. That means
 *     calc the needed money of all open buy orders.
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :depotId
 *     :date
 *
 *  Outputs: (order is important)
 *     cash
 */

-- GetDepotNeededCash.sql
-- FIXME: Looks pretty expensive, could you do it nicer?
SELECT COALESCE(sum("Required"), 0.0) FROM(

select :filu.convert_currency(cast(sum(acc."Required") as real), ficurr, depcurr, :date) as "Required"
from(
     SELECT -- select all open buy orders...
     sum( pieces *
            CASE WHEN olimit = 0.0
                      THEN ( select
                              avg(b.qclose)
                              from :filu.eodbar b
                              where fi_id = o.fi_id
                                and market_id = o.market_id
                                and b.qdate BETWEEN o.odate-5 and o.vdate
                            )
                      ELSE olimit
            END
        ) AS "Required"
        , m.currency_fi_id AS ficurr
        , d.currency AS depcurr

  FROM :user.order AS o
  LEFT JOIN :filu.fi AS fi USING(fi_id)
  LEFT JOIN :filu.market AS m USING(market_id)
  LEFT JOIN :user.depot AS d USING(depot_id)

  WHERE depot_id = :depotId
        and :date BETWEEN o.odate and o.vdate
        and o.buy -- ignore sell orders, well an unlimited could make sense
        and o.status >= 100 -- all kind of active orders

  GROUP BY m.currency_fi_id, d.currency

) as acc
group by ficurr, depcurr

) AS final
