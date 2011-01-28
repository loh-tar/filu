*  file name: GetCOs.sql
*  Used by function: Filu::getCOs()
*
*
*  Purpose:
*     Fetch *all* Chart Objects of a user defined time frame,
*     anyway which market.
*
*  Inputs: (variable names are important and begins with a colon)
*     :fiId
*     :fromDate
*     :toDate
*
*  Outputs: (order is important)
*     co_id
*     fi_id
*     market_id
*     plot
*     co_date
*     co_type
*     co_argv

-- GetCOs.sql
SELECT co_id, fi_id, market_id, co_plot, co_date, co_type, co_argv
  FROM :user.co
  WHERE fi_id = :fiId
    and (co_date BETWEEN :fromDate and :toDate or co_date = '1000-01-01')

  ORDER BY co_date
