/*
 *  Purpose:
 *     Fetch all positions as sum view
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :depotId
 *     :fiId
 *
 *  Outputs: (order *and* names are important)
 *     pdate     as "Date"
 *     fi_id     as "FiId"
 *     pieces    as "Pieces"
 *     price     as "Price"
 *     market_id as "MarketId"
 */

-- GetDepotPositionsTraderView.sql
SELECT rpdate     as "Date"
     , rfi_id     as "FiId"
     , rfiname    as "FiName"
     , rpieces    as "Pieces"
     , rprice     as "Price"
     , rmarket_id as "MarketId"
FROM :user.depotpos_traderview(:depotId, :fiId)
