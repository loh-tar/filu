*  file name: AddFiType
*  Used in function: Filu::addFiType
*
*  Purpose:
*     Add or update FI types
*
*  Inputs: (variable names are important and begins with a colon)
*     :fiType
*     :fiTypeId

-- AddFiType.sql
SELECT * FROM :filu.ftype_insert(:fiType, :fiTypeId);
