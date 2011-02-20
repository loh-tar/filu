/*
 *  Purpose:
 *     Add a FI to a group
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :group_id
 *     :fiId
 *
 *  Outputs: (order is important)
 */

-- AddGMember.sql
INSERT INTO  :user.gmember (group_id, fi_id)
VALUES (:group_id, :fiId)
