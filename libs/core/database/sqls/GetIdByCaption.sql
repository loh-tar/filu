*  file name: GetIdByCaption.sql
*  Used at Filu::searchCaption
*
*  Purpose:
*
*
*  Inputs: (variable names are important and begins with a colon)
*     :table
*     :caption
*
*  Outputs: (order is important)
*     only the ID or error code
*

-- GetIdByCaption.sql
SELECT * FROM :filu.id_from_caption(:table, :caption);
