#include "postgres.h"

#include <limits.h>

#include "utils/guc.h"

PG_MODULE_MAGIC;

/*
 * Module Load Callback
 */
void
_PG_init(void)
{
	//guc = find_option("shared_buffers", false, false, ERROR);

	//	set_config_option_ext("wal_consistency_checking",
	//							wal_consistency_checking_string,
	//							guc->scontext, guc->source, guc->srole,
	//							GUC_ACTION_SET, true, ERROR, false);
	//
	// SetConfigOption("shared_buffers", "32768", PGC_SIGHUP, PGC_S_OVERRIDE);
	// 2025-03-18 07:17:24.851 CET [38695] FATAL:  parameter "shared_buffers" cannot be changed without restarting the server
	// use another config file with a required restart???
	// https://vscode.dev/github/postgres/postgres/blob/master/src/backend/utils/misc/guc.c#L282
	// https://vscode.dev/github/postgres/postgres/blob/master/src/backend/utils/init/miscinit.c#L1899
	// SetConfigOption("shared_buffers", "32768", PGC_SIGHUP, PGC_S_DYNAMIC_DEFAULT);

	//SetConfigOption("max_connections", "500", PGC_SIGHUP, PGC_S_OVERRIDE);

	// FIXME: Could be called from other contexts???
	// FIXME: is this safe?
	SetConfigOption("max_connections", "500", PGC_POSTMASTER, PGC_S_FILE);
	SetConfigOption("shared_buffers", "256MB", PGC_POSTMASTER, PGC_S_FILE);
	// 2025-03-18 08:00:21.937 CET [55300] FATAL:  parameter "max_connections" cannot be changed without restarting the server
	// problem on SIGHUP:
	//2025-03-19 19:10:33.057 CET [24019] LOG:  received SIGHUP, reloading configuration files
	//2025-03-19 19:10:33.059 CET [24019] LOG:  parameter "max_connections" cannot be changed without restarting the server
	//2025-03-19 19:10:33.059 CET [24019] LOG:  parameter "shared_buffers" cannot be changed without restarting the server
	//2025-03-19 19:10:33.059 CET [24019] LOG:  configuration file "/Users/david.pech/pg/data/postgresql.conf" contains errors; unaffected changes were applied

	SetConfigOption("work_mem", "8MB", PGC_SUSET, PGC_S_OVERRIDE);
}
