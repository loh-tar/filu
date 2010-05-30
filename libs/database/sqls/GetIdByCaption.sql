*  file name: GetIdByCaption.sql
*  Used at
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

-- GetIdByCaption.sql
-- Returns:
--   0, if no data available(caption not found)
--  >0, found id to caption
--  -1, caption is more than one times in table

SELECT * FROM :filu.id_from_caption(:table, :caption);
