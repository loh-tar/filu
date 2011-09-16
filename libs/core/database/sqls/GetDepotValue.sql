/*
 *  Purpose:
 *     Calc the value of the depot at given date, but without cash
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :depotId
 *     :date
 *
 *  Outputs: (order is important)
 *     value
 */

-- GetDepotValue.sql
-- FIXME: Looks pretty expensive, could you do it nicer?
SELECT COALESCE(sum("Value"), 0.0) AS "Value"
FROM (

    select :filu.convert_currency(cast(sum(pieces * price) as real), ficurr, depcurr, :date) as "Value"
    from(

        SELECT sum(p.pieces) AS pieces, m.currency_fi_id AS ficurr, br.currency_fi_id AS depcurr,
          ( select qclose from :filu.eodbar
                          where fi_id = p.fi_id and qdate <= :date order by qdate DESC limit 1
          ) as price

        FROM :user.depotpos AS p
        LEFT JOIN :filu.market AS m  USING(market_id)
        LEFT JOIN :user.depot  AS d  USING(depot_id)
        LEFT JOIN :filu.broker AS br USING(broker_id)
        WHERE depot_id = :depotId and pdate <= :date
        GROUP BY p.fi_id, m.currency_fi_id, br.currency_fi_id

    ) as acc
    group by ficurr, depcurr

) AS final
