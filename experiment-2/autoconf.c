#include "postgres.h"

// shared load check
#include "miscadmin.h"

#include "postmaster/bgworker.h"
#include "storage/ipc.h"
#include "tcop/utility.h"

#include "libpq-fe.h"

#include <limits.h>

#include "utils/builtins.h"
#include "utils/guc.h"
#include "funcapi.h"

// include <sys/sysinfo.h>

#ifdef __APPLE__
#include <sys/time.h>
#include <sys/resource.h>

#endif /* __APPLE__ */

PGDLLEXPORT void AutoconfBackgroundWorker(Datum main_arg);

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pg_autoconf_recommendations);

/*
 * Module Load Callback
 */
void _PG_init(void)
{

// FIXME: hot standby


	// extension character = on a db level???
	//// = activated by a first? (tables will be accessible only in a single)

	// TODO: try to provide table - level info - try it - CREATE EXTENSION

	// TODO: HERE
	//DefineCustomRealVariable("auto_explain.sample_rate",
	//						 "Fraction of queries to process.",
	//						 NULL,
	//						 &auto_explain_sample_rate,
	//						 1.0,
	//						 0.0,
	//						 1.0,
	//						 PGC_SUSET,
	//						 0,
	//						 NULL,
	//						 NULL,
	//						 NULL);

	//MarkGUCPrefixReserved("auto_explain");

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

	// SetConfigOption("max_connections", "500", PGC_SIGHUP, PGC_S_OVERRIDE);

	// FIXME: Could be called from other contexts???
	// FIXME: is this safe?

	// FIXME: wrap apple
	int memSizeBytes = 0;
	struct rusage usage;
	if (0 == getrusage(RUSAGE_SELF, &usage))
		memSizeBytes = usage.ru_maxrss; // bytes

	int sharedBuffersMB;
	sharedBuffersMB = memSizeBytes > 0 ? memSizeBytes * 0.25 / 1024 : 512;
	const char* sharedBuffersStr = psprintf("%dMB", sharedBuffersMB);


    const char* prevValue = GetConfigOption("shared_buffers", true, false);
	ereport(WARNING,
			(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				errmsg("shared_buffers are \"%s\"", prevValue),
				errdetail("Current value of 'shared_buffers' \"%s\"", prevValue)));

	// note - this does not work well, because there is an override in postgresql.conf
	// SetConfigOption("max_connections", "500", PGC_POSTMASTER, PGC_S_DYNAMIC_DEFAULT);
	SetConfigOption("max_connections", "500", PGC_POSTMASTER, PGC_S_FILE);
	SetConfigOption("shared_buffers", sharedBuffersStr, PGC_POSTMASTER, PGC_S_FILE);

    const char* prevValueAfter =  GetConfigOption("shared_buffers", true, false);
	ereport(WARNING,
			(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				errmsg("shared_buffers after \"%s\"", prevValueAfter),
				errdetail("Current value of 'shared_buffers' \"%s\"", prevValueAfter)));

	// discussion: PGC_S_FILE vs. ... dynamic_default

	pfree(sharedBuffersStr);
	// 2025-03-18 08:00:21.937 CET [55300] FATAL:  parameter "max_connections" cannot be changed without restarting the server
	// problem on SIGHUP:
	// 2025-03-19 19:10:33.057 CET [24019] LOG:  received SIGHUP, reloading configuration files
	// 2025-03-19 19:10:33.059 CET [24019] LOG:  parameter "max_connections" cannot be changed without restarting the server
	// 2025-03-19 19:10:33.059 CET [24019] LOG:  parameter "shared_buffers" cannot be changed without restarting the server
	// 2025-03-19 19:10:33.059 CET [24019] LOG:  configuration file "/Users/david.pech/pg/data/postgresql.conf" contains errors; unaffected changes were applied

	// FIXME: THIS OVERRIDES the S_FILE
	//SetConfigOption("work_mem", "8MB", PGC_SUSET, PGC_S_OVERRIDE);
	SetConfigOption("work_mem", "8MB", PGC_SUSET, PGC_S_FILE);


	BackgroundWorker worker;

	MemSet(&worker, 0, sizeof(BackgroundWorker));
	worker.bgw_flags = BGWORKER_SHMEM_ACCESS | BGWORKER_BACKEND_DATABASE_CONNECTION;
	worker.bgw_start_time = BgWorkerStart_RecoveryFinished;
	worker.bgw_restart_time = 1;
	worker.bgw_main_arg = Int32GetDatum(0);
	worker.bgw_notify_pid = 0;
	sprintf(worker.bgw_library_name, "autoconf");
	sprintf(worker.bgw_function_name, "AutoconfBackgroundWorker");
	snprintf(worker.bgw_name, BGW_MAXLEN, "pg_autoconf worker");
	snprintf(worker.bgw_type, BGW_MAXLEN, "pg_autoconf worker");

	// FIXME: also some upgrade check somehwere??
	// ????? if (process_shared_preload_libraries_in_progress)
	RegisterBackgroundWorker(&worker);

	ereport(WARNING,
			(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				errmsg("after reg"),
				errdetail("after reg")));
}


/*
 * Background worker logic.
 */
void
AutoconfBackgroundWorker(Datum main_arg)
{
	//2025-04-15 20:13:46.558 CEST [19908] LOG:  background worker "pg_autoconf worker": background workers without shared memory access are not supported

	ereport(WARNING,
			(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				errmsg("hello from wo"),
				errdetail("hello from wo")));
	//dsm_segment *seg;
	//shm_toc *toc;
	//char *database;
	//char *username;
	//char *command;
	//shm_mq *mq;
	//shm_mq_handle *responseq;

	/* handle SIGTERM like regular backend */
	pqsignal(SIGTERM, die);
	BackgroundWorkerUnblockSignals();

	/* Set up a memory context and resource owner. */
//	Assert(CurrentResourceOwner == NULL);
//	CurrentResourceOwner = ResourceOwnerCreate(NULL, "pg_autoconf");
//	CurrentMemoryContext = AllocSetContextCreate(TopMemoryContext,
//												 "pg_autoconf worker",
//												 ALLOCSET_DEFAULT_MINSIZE,
//												 ALLOCSET_DEFAULT_INITSIZE,
//												 ALLOCSET_DEFAULT_MAXSIZE);
//
	/* Set up a dynamic shared memory segment. */
	//seg = dsm_attach(DatumGetInt32(main_arg));
	//if (seg == NULL)
	//	ereport(ERROR,
	//			(errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE),
	//			 errmsg("unable to map dynamic shared memory segment")));
	//toc = shm_toc_attach(PG_CRON_MAGIC, dsm_segment_address(seg));
	//if (toc == NULL)
	//	ereport(ERROR,
	//			(errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE),
	//		   errmsg("bad magic number in dynamic shared memory segment")));

	//database = shm_toc_lookup(toc, PG_CRON_KEY_DATABASE, false);
	//username = shm_toc_lookup(toc, PG_CRON_KEY_USERNAME, false);
	//command = shm_toc_lookup(toc, PG_CRON_KEY_COMMAND, false);
	//mq = shm_toc_lookup(toc, PG_CRON_KEY_QUEUE, false);

	//shm_mq_set_sender(mq, MyProc);
	//responseq = shm_mq_attach(mq, seg, NULL);
	//pq_redirect_to_shm_mq(seg, responseq);

	// FIXME: decide - either use low-level approach from pg_cron, or libpq 
	//BackgroundWorkerInitializeConnection("postgres", NULL, 0);
	//BackgroundWorkerInitializeConnection(database, username, 0);
	// vs Oid variant? BackgroundWorkerInitializeConnection(database, username, 0);

	//BackgroundWorkerInitializeConnectionByOid(MyDatabaseId, InvalidOid, 0);

	///* Prepare to execute the query. */
	//SetCurrentStatementStartTimestamp();
	//debug_query_string = command;
	//pgstat_report_activity(STATE_RUNNING, command);
	//StartTransactionCommand();
	//if (StatementTimeout > 0)
	//	enable_timeout_after(STATEMENT_TIMEOUT, StatementTimeout);
	//else
	//	disable_timeout(STATEMENT_TIMEOUT, false);

	///* Execute the query. */
	//ExecuteSqlString(command);

	///* Post-execution cleanup. */
	//disable_timeout(STATEMENT_TIMEOUT, false);
	//CommitTransactionCommand();
	//pgstat_report_activity(STATE_IDLE, command);
	//pgstat_report_stat(true);

	/* Signal that we are done. */
	//ReadyForQuery(DestRemote);

	ereport(WARNING,
			(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				errmsg("hello from worker"),
				errdetail("hello from woker")));

	// does not work?
	//SetConfigOption("work_mem", "16MB", PGC_POSTMASTER, PGC_S_FILE);
	// FIXME: chert return
	// FIXME: how to correctly set a value from background worker, so it gets updates system wide without touching the postgresql.auto.conf?
	set_config_option("work_mem", "16111222",
								PGC_POSTMASTER, PGC_S_FILE,
								GUC_ACTION_SET, true, 0, false);

	const char conninfo[] = "dbname=postgres user=david.pech";
	PGconn *conn = PQconnectdb(conninfo);
	/* Check to see that the backend connection was successfully made */
	if (PQstatus(conn) != CONNECTION_OK) {
		ereport(WARNING,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
					errmsg("conn not ok %s", PQerrorMessage(conn)),
					errdetail("conn not ok")));
		PQfinish(conn);
		exit(1);
	}

	// FIXME: PG_TRY?
	PGresult *res = PQexec(conn, "SELECT 1024 * 1024 * EXTRACT(DAY FROM NOW()) AS value");
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		ereport(WARNING,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
					errmsg("could not clear"),
					errdetail("could not clear")));
		PQclear(res);
		PQfinish(conn);
		exit(1);
	}
    // print column name
    for (int i = 0; i < PQnfields(res); i++) {
		ereport(WARNING,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
					errmsg("%s", PQfname(res, i)),
					errdetail("cols")));
    }
    // print column values
    for (int i = 0; i < PQntuples(res); i++) {
      for (int j = 0; j < PQnfields(res); j++) {
		ereport(WARNING,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
					errmsg("%d:%d %s", i, j, PQgetvalue(res, i, j)),
					errdetail("val")));
		//if (i == 0 && j == 0) {
		//	SetConfigOption("work_mem", PQgetvalue(res, i, j), PGC_SUSET, PGC_S_OVERRIDE);
		//}
      }
    }

	PQclear(res);
	
	// ??? not a preferred method
	res = PQexec(conn, "ALTER SYSTEM SET work_mem='16MB'");
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		ereport(WARNING,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
					errmsg("could not clear %s", PQerrorMessage(conn)),
					errdetail("could not clear")));
		PQclear(res);
		PQfinish(conn);
		exit(1);
	}
	PQclear(res);
	res = PQexec(conn, "SELECT pg_reload_conf()");
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		ereport(WARNING,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
					errmsg("could not clear %s", PQerrorMessage(conn)),
					errdetail("could not clear")));
		PQclear(res);
		PQfinish(conn);
		exit(1);
	}
	PQclear(res);

	PQfinish(conn);

	//dsm_detach(seg);
	proc_exit(0);
}





// config options [SELECT ] 1 [FROM fake_autconf_single_row_table]
// guc1 = 1
// guc1 = 1+1
// guc1 = 1+1+"FAKE_COLUMN_AS_SINGLE_ROW_VARIABLE"
// guc1 = (SELECT 1)
// guc1 = (SELECT 1+1)
// guc1 = (SELECT ... some system catalog perhaps..)

/***
 * 
 *  "select * from pg_autoconf_recommendations;"
psql: warning: extra command-line argument "mytest" ignored
    guc_name     | guc_value |               alter_sql                
-----------------+-----------+----------------------------------------
 some.random.guc | 123       | ALTER SYSTEM SET some.random.guc = 123
(1 row)

 */
Datum
pg_autoconf_recommendations(PG_FUNCTION_ARGS)
{
	TupleDesc	tupdesc;

	Datum		result;
	HeapTuple	tuple;
	Datum		values[3];
	bool		nulls[3];

	// FIXME: ???
	//if (!superuser())
	//	ereport(ERROR,
	//			(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
	//			 errmsg("must be superuser to use raw page functions")));
	/*
	 * Superusers or roles with the privileges of pg_read_all_stats members
	 * are allowed
	 */
	// FIXME: HERE
	//is_allowed_role = has_privs_of_role(userid, ROLE_PG_READ_ALL_STATS);

	/* Build a tuple descriptor for our result type */
	if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
		elog(ERROR, "return type must be a row type");

	values[0] = CStringGetTextDatum("some.random.guc");
	values[1] = CStringGetTextDatum("123");
	values[2] = CStringGetTextDatum("ALTER SYSTEM SET some.random.guc = 123");

	/* Build and return the tuple. */

	memset(nulls, 0, sizeof(nulls));

	tuple = heap_form_tuple(tupdesc, values, nulls);
	result = HeapTupleGetDatum(tuple);

	PG_RETURN_DATUM(result);
}