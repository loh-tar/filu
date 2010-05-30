*  file name: AddGroup.sql
*  Used in function: Importer::addGroup()
*
*  Purpose:
*     Creates a new group and return its id or returns
*     the id if group already exist. All done by psql function
*
*  Inputs: (variable names are important and begins with a colon)
*     :groupPath -- like: /Suggestions/Someone
*
*  Outputs: (order is important)

-- AddGroup.sql
-- Returns mother group id, whereas 0 is a valid value for root group
SELECT * FROM :user.group_insert(:groupPath);
