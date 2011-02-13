*  file name: AddDepotOrder.sql
*  Used in function: FiluU::addOrder
*
*  Purpose:
*     Insert or update a order postion.
*     Returns Id or ErrorNo. All done by psql function
*
*  Inputs: (variable names are important and begins with a colon)
*     :orderId
*     :depotId
*     :odate
*     :vdate
*     :fiId
*     :pieces
*     :limit
*     :buy
*     :marketId
*     :status
*     :note
*
*  Outputs: (order is important)
*     depotPosId or ErrorNo
*

-- AddDepotOrder.sql
SELECT * FROM :user.order_insert
( :orderId, :depotId
, CAST(:oDate as date)
, CAST(:vDate as date)
, :fiId, :pieces, :limit, :buy, :marketId
, CAST(:status as smallint)
, :note
);
