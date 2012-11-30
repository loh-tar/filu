/*
 *  Purpose:
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :orderId
 *     :depotId
 *     :fiId
 *     :status
 *
 *  Outputs: (order *and* names are important)
 *     order_id  as "OrderId"
 *     depot_id  as "DepotId"
 *     odate     as "ODate"
 *     vdate     as "VDate"
 *     fi_id     as "FiId"
 *                  "FiName"
 *                  "Symbol"
 *     pieces    as "Pieces"
 *     olimit    as "Limit"
 *                  "Currency"
 *     status    as "Status"
 *     buy       as "Buy"
 *     market_id as "MarketId"
 *                  "Market"
 */

-- GetDepotOrders.sql
SELECT order_id   as "OrderId"
     , depot_id   as "DepotId"
     , odate      as "ODate"
     , vdate      as "VDate"
     , o.fi_id    as "FiId"
     , fi.caption as "FiName"
     , lf.symbol  as "Symbol"
     , pieces     as "Pieces"
     , olimit     as "Limit"
     , m.currency_fi_id  as "CurrencyId"
     , lm.symbol  as "Currency"
     , status     as "Status"
     , buy        as "Buy"
     , market_id  as "MarketId"
     , m.caption  as "Market"
     , o.note     as "Note"

  FROM :user.order AS o
    LEFT JOIN :filu.fi AS fi USING(fi_id)
    LEFT JOIN :filu.market AS m USING(market_id)
    LEFT JOIN :filu.lovelysymbol AS lm ON lm.fi_id=currency_fi_id
    LEFT JOIN :filu.lovelysymbol AS lf ON lf.fi_id=o.fi_id

  WHERE     CASE WHEN :orderId = -1 THEN true ELSE order_id = :orderId END
        and CASE WHEN :depotId = -1 THEN true ELSE depot_id = :depotId END
        and CASE WHEN :fiId   = -1 THEN true ELSE o.fi_id  = :fiId END
        and CASE WHEN :status = -1 THEN true
                 WHEN :status = 100 THEN o.status >= 100 -- Get not only 'active' but 'ask' too
                 ELSE o.status = :status
            END

  ORDER BY o.odate DESC

