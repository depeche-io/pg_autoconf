PG AutoConf / AutoTune extension / PG BetterDefaults / pg_sane_defaults ???
====

PostgreSQL is notoriously know for a super conservative defaults that are kept for years and IMHO 99% workloads need to be tuned even with a basic parameters.

Main goals:
- easy install
- provide sensible default based on OS, system resources and other factors out of the box for 95% of OLTP workloads
- allow customization of configuration set via Profiles (?)
- play nice with other tooling like K8s operators and Patroni -> if possible, restart or changing the `postgresql.conf` is not required
- 2 modes (must be even possible to combine them partially) - only recommendations vs. apply directly

Project stages:
- POC
  - Is the idea possible? Yes - https://www.enterprisedb.com/docs/pg_extensions/pg_tuner/using/#automatic-tuning
  - create basic extension that is able to change GUC on startup according to templates.  
  - replace template variables with basic OS and other metrics
  - main problem: GUCs requiring restart vs. others

- MVP
  - Similar to SELECT edb_pg_tuner_recommendations; # for recommendations
    - also have a column "APPLY_COMMAND / sql" with SQL that can directly set this (+ extra pg_conf_reload)
    - option to write recommendations to log
  - Have several different Profiles (OLTP, OLAP, ...), support multiple PG versions and OS
  - Provide docs on how to extend the project + links to explain why the formula was chosen (just a *basic* discussion to show some edge cases to be aware of)
  - Provide automatically this: https://pgtune.leopard.in.ua/ (similar scope)
    - meaning memory tuning based on OS params
    - ??? Valualbe if GUC requiring restart, ? not much without them?

```
# https://pgtune.leopard.in.ua/ 
# DB Version: 17
# OS Type: linux
# DB Type: web
# Total Memory (RAM): 16 GB
# CPUs num: 4
# Connections num: 500
# Data Storage: ssd

max_connections = 500 # restart
shared_buffers = 4GB # restart
# ??? wal_buffers = 16MB # restart
huge_pages = off # restart
max_worker_processes = 4 # restart

effective_cache_size = 12GB
maintenance_work_mem = 1GB
checkpoint_completion_target = 0.9
default_statistics_target = 100 # increase????
random_page_cost = 1.1
effective_io_concurrency = 200
work_mem = 4194kB # ?????
min_wal_size = 1GB
max_wal_size = 4GB
max_parallel_workers_per_gather = 2
max_parallel_workers = 4
max_parallel_maintenance_workers = 2
```

- V1
  - Provide case studies, how this helps in specific use-cases
  - Gather Feedback
  - Metrics can be connected to SQL queries inside the DB (note - this means = first start PG, then execute SQL, then pg_reload_conf) (?)
    - very valuable = tune autovacuum based on table sizes, number of tables, basic table+index usage etc.

- V2
  - Evaluate templates periodically
    - Recommend / apply directly
    - Threshold for change (align the result value), minimize noise + flapping

- Vx
  - Be included by default in PG operators in K8s

- Vnever
  - postgres-contrib (apologies for a bad joke)

postgresql.conf:
```
shared_preload_libraries = 'pg_autoconf'
# not needed
#pg_autoconf.profiles='conservative16' # or others preinstalled, or filename
```

Idea
===

- parameters in `postgresql.conf` have a higher precedence over the extensions
  - FIXME: - use extension, if param value == defaults?? (even it's included in postgresql.conf?) -> useful for just 1-line config change, however not cool is some OS changes defaults.
  - select * from pg_file_settings;"

So suggested installation:
- add this extension
- remove majority of lines from config `postgresql.conf` (so the extension overrides the defaults)


Templating example
===
- Templating language
  - major feature == readability

  - might be actually 2 kinds: for Postmaster GUCs (on startup requiring restart otherwise) vs. all runtime GUCs

  1. variant - use SQL
  - not available for "startup" - shared_buffers?
  - not a great readability for more complicated logic
  - very convenient for DBAs

  maintenance_work_mem = SELECT GREATEST(DBInstanceClassMemory/63963136*1024,65536);

  config options [SELECT ] 1 [FROM fake_autconf_single_row_table]
  guc1 = 1
  guc1 = 1+1
  guc1 = 1+1+"FAKE_COLUMN_AS_SINGLE_ROW_VARIABLE"
  guc1 = (SELECT 1)
  guc1 = (SELECT 1+1)
  guc1 = (SELECT ... some system catalog perhaps..)


  2. variant - custom templates:
  -  ??? inspiration from ? cel.dev / Jinja2 / Mustache ? ( too complex, but anything "standard-like" with C-bindings would do)


    # AWS
    https://docs.aws.amazon.com/AmazonRDS/latest/AuroraUserGuide/AuroraPostgreSQL.Reference.ParameterGroups.html

    ```
    effective_cache_size = SUM(DBInstanceClassMemory/12038,-50003)
    maintenance_work_mem = GREATEST(DBInstanceClassMemory/63963136*1024,65536)
    max_parallel_workers = GREATEST($DBInstanceVCPU/2,8)
    ```


    # pgtune example of more complex logic (TS/JS)
    https://github.com/le0pard/pgtune/blob/master/src/features/configuration/configurationSlice.js

    ```
    createSelector([selectSharedBuffers], (sharedBuffersValue) => {
        // Follow auto-tuning guideline for wal_buffers added in 9.1, where it's
        // set to 3% of shared_buffers up to a maximum of 16MB.
        let walBuffersValue = Math.floor((3 * sharedBuffersValue) / 100)
        const maxWalBuffer = (16 * SIZE_UNIT_MAP['MB']) / SIZE_UNIT_MAP['KB']
        if (walBuffersValue > maxWalBuffer) {
            walBuffersValue = maxWalBuffer
        }
        // It's nice of wal_buffers is an even 16MB if it's near that number. Since
        // that is a common case on Windows, where shared_buffers is clipped to 512MB,
        // round upwards in that situation
        const walBufferNearValue = (14 * SIZE_UNIT_MAP['MB']) / SIZE_UNIT_MAP['KB']
        if (walBuffersValue > walBufferNearValue && walBuffersValue < maxWalBuffer) {
            walBuffersValue = maxWalBuffer
        }
        // if less, than 32 kb, than set it to minimum
        if (walBuffersValue < 32) {
            walBuffersValue = 32
        }
        return walBuffersValue
    })
    ```

TODO
===
- Mysql autotune (since 8?) - inspiration
- Will this require custom user-defined GUCs for the formulas?
- https://postgrestuner.enterprisedb.com/configurator/
- https://www.enterprisedb.com/docs/pg_extensions/pg_tuner/configuring/

Brainstorming
===
- Run some simple benchmark for the first time running PG to get disk performance for example ???
    - better example - run benchmark to enable pg_stats_io (which is safe 99.99% cases, except..)
- Feature: provide log warning for "config value too low?"
- "default" in pg_settings
- EDB work_mem tuning with pg_stat_statements enabled behind the scenes? new_work_mem = ceil(max(1.75 * previous_sort_spill, 5.0 * previous_hash_spill))
  - work_mem - https://www.enterprisedb.com/docs/pg_extensions/pg_tuner/using/#auto-tuning-work_mem


Known Problems
===
- disallow for `session_preload_libraries` ???
- shared_preload_libraries position???

Merch
===
These tuning are not like gaining 10% of performance boost, but more running with a tied up legs. One example for all - PostgreSQL does not check the memory limits on the system it runs at all. This extension does not aim to satisfy everyone, but rather to offer a simple option to customize `postgresql.conf` defaults base od calculations. Different opinions or approaches can be accomodated using different Profiles.