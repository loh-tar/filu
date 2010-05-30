*  file name: AddTradingResult.sql
*  Used in function: FiluU::
*
*  Purpose:
*     
*
*  Inputs: (variable names are important and begins with a colon)
*     
*

INSERT INTO  :user.tsr (ts_id, fi_id, market_id, wltp, lltp, agwlp, alltp, tpp, score)
VALUES (:tsId, :fiId, :marketId, :wltp, :lltp, :agwlp, :alltp, :tpp, :score)

-- tsr_id         SERIAL8     NOT NULL,
-- ts_id          SERIAL8     NOT NULL,
-- fi_id          INT8        NOT NULL,
-- market_id      INT8        NOT NULL,
-- wltp           FLOAT       NOT NULL, -- Won long trades %
-- lltp           FLOAT       NOT NULL, -- Lost long trades %
-- agwlp          FLOAT       NOT NULL, -- Avg gain of won lt %
-- alltp          FLOAT       NOT NULL, -- Avg lost of lost lt %
-- -- FIXME: add short stuff
-- tpp            FLOAT       NOT NULL, -- Total Performance %
-- score          FLOAT       NOT NULL, -- ln(exp(won long trades * Avg gain of won lt%)/exp(lost long trades * Avg lost of lost lt%))
