/*
 *  Purpose:
 *     Fetch all groups
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :motherId
 *
 *  Outputs: (order is important)
 *     group_id
 *     caption
 *     mothergroup_id
 */

-- GetAllGroups.sql
SELECT  group_id, caption, mothergroup_id
  FROM :user.group_childs(:motherId);
