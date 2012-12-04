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

SET client_min_messages TO WARNING;
DROP SCHEMA :filuDevil, :userDevil CASCADE;
