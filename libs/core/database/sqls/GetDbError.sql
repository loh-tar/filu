*  file name: GetDbError
*  Used in function: Filu::dbFuncErrText
*
*  Purpose:
*     Yes, the the error text to the filu error code
*
*  Inputs: (variable names are important and begins with a colon)
*     :errCode
*
*  Outputs: (order is important)
*     the error text
*

-- GetDbError.sql
SELECT etext FROM :filu.error WHERE error_id = :errCode
