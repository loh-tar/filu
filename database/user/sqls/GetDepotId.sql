/*
 *  Purpose:
 *     Yes, fetch the depotId by name and owner.
 *     Returns 0 if not found.
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :name
 *     :owner
 *
 *  Outputs: (order is important)
 *     depot_id or 0
 */

-- GetDepotId.sql
SELECT COALESCE(depot_id, 0)
        FROM :user.depot
        WHERE lower(caption) = lower(:name) AND lower(owner) = lower(:owner);
