-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION autoconf" to load this file. \quit

CREATE FUNCTION pg_autoconf_recommendations(
    OUT guc_name text,
    OUT guc_value text,
    OUT alter_sql text
)
RETURNS SETOF record
AS 'MODULE_PATHNAME', 'pg_autoconf_recommendations'
LANGUAGE C STRICT VOLATILE PARALLEL SAFE;

-- Register a view on the function for ease of use.
CREATE VIEW pg_autoconf_recommendations AS
  SELECT * FROM pg_autoconf_recommendations();

GRANT SELECT ON pg_autoconf_recommendations TO PUBLIC;

-- Don't want this to be available to non-superusers.
REVOKE ALL ON FUNCTION pg_autoconf_recommendations() FROM PUBLIC;
