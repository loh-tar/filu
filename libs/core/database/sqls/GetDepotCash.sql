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
*

-- GetDepotCash.sql
SELECT COALESCE(sum(bvalue), 0.0) AS "Cash"
  FROM :user.account AS a
  NATURAL JOIN :user.depot AS d
  WHERE depot_id = :depotId and bdate <= :date
