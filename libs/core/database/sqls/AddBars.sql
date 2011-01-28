*  file name: AddBars.sql
*  Used in function: Filu::addEODBarData()
*
*  Purpose:
*     Add or update bars with values to the DB
*
*  Inputs: (variable names are important and begins with a colon)
*     :fiId
*     :marketId
*     :date
*     :open
*     :high
*     :low
*     :close
*     :vol
*     :oi
*     :status

-- AddBars.sql
SELECT :filu.eodbar_insert
  ( :fiId
  , :marketId
  , CAST(:date   AS date)
  , CAST(:open   AS float4)
  , CAST(:high   AS float4)
  , CAST(:low    AS float4)
  , CAST(:close  AS float4)
  , CAST(:vol    AS float)
  , CAST(:oi     AS int4)
  , CAST(:status AS int2) )
