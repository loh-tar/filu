/*
 *  Purpose:
 *      Get all available developmend schemata
 *
 *  Inputs: (variable names are important and begins with a colon)
 *      :username
 *
 *  Outputs: (order is important)
 *
 */

SELECT nspname FROM pg_namespace
    WHERE position('user_' || :username || '_' in nspname) > 0
