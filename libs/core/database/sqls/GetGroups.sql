*  file name: GetGroups.sql
*  Used as query: sqlGetGroups
*  Used in function: FiluU::
*
*  Purpose:
*     Fetch groups belonging to the mother group
*
*  Inputs: (variable names are important and begins with a colon)
*     :motherId
*
*  Outputs: (order is important)
*     group_id
*     caption
*     mothergroup_id

-- GetGroups.sql
SELECT group_id, caption, mothergroup_id
  FROM :user.group
  WHERE CASE WHEN :motherId < 0 THEN TRUE ELSE mothergroup_id = :motherId END
  ORDER BY caption
