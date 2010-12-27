*  file name: GetFi.sql
*  Used in function: Filu::getFi()
*
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
*

-- GetFi.sql
SELECT f.fi_id, f.ftype_id, f.caption, t.caption
  FROM :filu.fi f, :filu.ftype t
  WHERE f.ftype_id = t.ftype_id
   AND CASE WHEN :fiId = 0
             THEN CASE WHEN :fuzzy = TRUE
                   THEN   UPPER(f.caption) LIKE '%'|| UPPER(:name) ||'%'
                      AND UPPER(t.caption) LIKE '%'|| UPPER(:type)||'%'
                   ELSE   f.caption = :name AND t.caption = :type
                  END
             ELSE f.fi_id = :fiId
        END



