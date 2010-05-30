*  file name: PutGroup.sql
*  Used as query: mSqlPutGroup
*  Used in function: FiluU::
*
*  Purpose:
*     Create or update a group
*
*  Inputs: (variable names are important and begins with a colon)
*     :group_id
*     :name
*     :motherId
*
*  Outputs: (order is important)
*

-- PutGroup.sql
INSERT INTO  :user.group (group_id, caption, mothergroup_id)
VALUES (:group_id, :name, :motherId)

