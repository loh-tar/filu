/*
 *  Purpose:
 *     As the name implies
 *
 *  Inputs: (variable names are important and begins with a colon)
 *     :momId
 *
 *  Outputs: (order is important)
 */

-- DelAllUnderlyingsFromMother.sql
DELETE
FROM
  :filu.underlying
WHERE
  fi_id = :momId
;
