*  file name: PutCOs.sql
*  Used by function: Filu::putCOs()
*                    Importer::addCO()
*
*  Purpose:
*     Set und update Chart Objects.
*
*  Inputs: (variable names are important and begins with a colon)
*     :id
*     :fiId
*     :marketId
*     :plot
*     :date
*     :type
*     :parameters
*
*  Outputs: (order is important)
*
*
-- PutCOs.sql
INSERT INTO :user.co
          (co_id, fi_id, market_id, co_plot, co_date, co_type, co_argv)
  VALUES (:id, :fiId, :marketId, :plot, :date, :type, :parameters)
