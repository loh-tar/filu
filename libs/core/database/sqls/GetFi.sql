/*
 *  Purpose:
 *     To fetch some Fi by long name and type settings
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :name  // like "Apple Computer"
 *     :type  // like "Stock"
 *     :fuzzy // if 1 use fuzzy search
 *     :fiId
 *
 *  Outputs: (order is important)
 *     Id
 *     TypeId
 *     Name
 *     Type
 *     IssueDate (controversial)
 *     MaturityDate (controversial)
 */

-- GetFi.sql
SELECT f.fi_id, f.ftype_id, f.caption, t.caption
  FROM :filu.fi f, :filu.ftype t
  WHERE f.ftype_id = t.ftype_id
   and CASE WHEN :fiId = 0
             THEN CASE WHEN :fuzzy = TRUE
                   THEN   lower(f.caption) LIKE '%'|| lower(:name) ||'%'
                      and lower(t.caption) LIKE '%'|| lower(:type)||'%'
                   ELSE   f.caption = :name and t.caption = :type
                  END
             ELSE f.fi_id = :fiId
        END

