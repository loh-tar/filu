*  file name: AddDepotPos.sql
*  Used in function: FiluU::addDepotPos
*
*  Purpose:
*     Insert or update a depot postion.
*     Returns Id or ErrorNo. All done by psql function
*
*  Inputs: (variable names are important and begins with a colon)
*     :depotId
*     :date
*     :fiId
*     :pieces
*     :price
*     :marketId
*     :note
*     :depotPosId
*
*  Outputs: (order is important)
*     depotPosId or ErrorNo
*

-- AddDepotPos.sql
SELECT * FROM :user.depotpos_insert(
    :depotPosId, :depotId
  , CAST(:date as date)
  , :fiId, :pieces, :price, :marketId, :note
);
