*  file name: GetGroupPathById.sql
*  Used by Exporter::expGroups()
*
*  Purpose:
*
*
*  Inputs: (variable names are important and begins with a colon)
*     :groupId
*
*  Outputs: (order is important)
*     only the path
*

-- GetGroupPathById.sql
SELECT * FROM :user.group_id_to_path(:groupId);