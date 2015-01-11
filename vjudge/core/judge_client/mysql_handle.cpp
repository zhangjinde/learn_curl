#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mysql/mysql.h>

#include "judge_client.h"

extern int DEBUG;
extern int db_port;
extern int shm_run;
extern int max_running;
extern int db_timeout;
extern int sleep_time;
extern int java_time_bonus;
extern int java_memory_bonus;
extern int sim_enable;
extern int oi_mode;
extern int use_max_time;
extern char record_call;
extern char db_host[BUFSIZE];
extern char db_user[BUFSIZE];
extern char db_passwd[BUFSIZE];
extern char work_dir[BUFSIZE];
extern char db_name[BUFSIZE];
extern char oj_home[BUFSIZE];
extern char java_xms[BUFSIZE];
extern char java_xmx[BUFSIZE];
extern char LANG_NAME[BUFSIZE];
extern char lang_ext[15][8];
extern MYSQL *conn;
extern struct solution_t *solution;
extern int call_counter[BUFSIZE];
extern int call_array_size;

MYSQL *prepare_mysql(void)
{
	MYSQL *conn = mysql_init(NULL);
	if (conn == NULL) {
		write_log("init mysql error.\n");
		exit(EXIT_FAILURE);
	}
	int ret = mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT,
			&db_timeout);
	if (ret) {
		write_log("set mysql timeout error:%s.\n", mysql_error(conn));
		exit(EXIT_FAILURE);
	}

	write_log("try to connect database\n");
	conn = mysql_real_connect(conn, db_host, db_user, db_passwd,
			db_name, db_port, NULL, 0);
	if (conn == NULL) {
		write_log("connect database error:%s.\n", mysql_error(conn));
		exit(EXIT_FAILURE);
	}
	if (mysql_set_character_set(conn, "utf8")) {
		write_log("set mysql character set error:%s.\n", mysql_error(conn));
		exit(EXIT_FAILURE);
	}
	return conn;
}

void cleanup_mysql(void)
{
	if (conn != NULL) {
		write_log("close mysql connection.\n");
		mysql_close(conn);
	}
}

int execute_sql(const char *fmt, ...)
{
	char *sql = (char *)malloc(BUFSIZE * BUFSIZE);
	if (sql == NULL) {
		write_log("alloc memory error!\n");
		return -1;
	}
	memset(sql, 0, BUFSIZE * BUFSIZE);
	va_list ap;
	va_start(ap, fmt);
	vsprintf(sql, fmt, ap);
	va_end(ap);

	write_log("execute sql：%s.\n", sql);
	if (mysql_real_query(conn, sql, strlen(sql))) {
		write_log("execute sql error:%s.\n", mysql_error(conn));
		free(sql);
		sleep(db_timeout);
		return -1;
	} else {
		free(sql);
		return 0;
	}
}

void ping(void)
{
	if (!mysql_ping(conn)) {
	}
}
