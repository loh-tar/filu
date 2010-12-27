*  file name: AddFi.sql
*  Used in function: both Filu::addFi(const QString &name...)
*
*  Purpose:
*     Add a new FI to the DB
*
*  Inputs: (variable names are important and begins with a colon)
*     :fiId
*     :name
*     :fType
*     :symbol
*     :sType
*     :market

-- AddFi.sql Returns:
--   >0 if all is fine, the fiId
--   -1 if fiType is not valid
--   -2 if fiName was empty
--   -3 if symbol is empty
--   -4 if stype is not valid
--   -5 if market is not valid
--   -6 if unique violation
--   -7 if symbol was found more than one time and was associated to different FIs
--   -8 if foreign key violation (should impossible?)
--   -9 if other error (should impossible?)
SELECT * FROM :filu.fi_insert(:fiId, :name, :fType, :symbol, :sType, :market);
