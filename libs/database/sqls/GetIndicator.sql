*  file name: GetIndicator.sql
*  Used as query: mSqlGetIndicator
*  Used in function: Filu::getFi()
*
*  Purpose:
*     To fetch an indicator property
*
*  Inputs: (variable names are important and begins with a colon)
*     :name
*
*  Outputs: (order is important)
*     name
*     input
*     output
*     sql
*     source

--GetIndicator.sql
SELECT caption, arguments, rettype, call, source FROM :filu.filupower_indicator
WHERE
  CASE WHEN length(:name) = 0 THEN TRUE ELSE caption = :name END
