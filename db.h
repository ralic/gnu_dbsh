#ifndef DB_H
#define DB_H

#include <sys/time.h>
#include <time.h>

#include <sql.h>
#include <sqlext.h>

typedef struct db_results db_results;
typedef struct db_column db_column;

struct db_results {
	SQLSMALLINT ncols;
	SQLINTEGER nrows;
	char **cols;
	char ***data;
	struct timeval time_taken;
};

SQLHENV alloc_env();
void list_all_dsns(SQLHENV);
SQLHDBC connect_dsn(SQLHENV, const char *, const char *, const char *);
db_results *execute_query(SQLHDBC, const char *);
void free_results(db_results *);

#endif
