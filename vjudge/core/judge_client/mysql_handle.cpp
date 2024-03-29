#include "judge_client.h"

MYSQL *prepare_mysql(void)
{
	MYSQL *conn = mysql_init(NULL);
	if (conn == NULL) {
		write_log("init mysql error:%s.\n", mysql_error(conn));
		return NULL;
	}
	int ret = mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT,
			&db_timeout);
	if (ret) {
		write_log("set mysql timeout error:%s.\n", mysql_error(conn));
		return NULL;
	}

	write_log("try to connect database\n");
	conn = mysql_real_connect(conn, db_host, db_user, db_passwd,
			db_name, db_port, NULL, 0);
	if (conn == NULL) {
		write_log("connect database error:%s.\n", mysql_error(conn));
		return NULL;
	}
	if (mysql_set_character_set(conn, "utf8")) {
		write_log("set mysql character set error:%s.\n", mysql_error(conn));
		cleanup_mysql();
		return NULL;
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

void ping(void)
{
	if (mysql_ping(conn)) {
		cleanup_mysql();
		write_log("reconnect mysql server.\n");
		conn = prepare_mysql();
	}
}

int execute_sql(const char *fmt, ...)
{
	char *sql = (char *)malloc(BUFSIZE * BUFSIZE);
	if (sql == NULL) {
		write_log("alloc execute_sql sql buf memory error!\n");
		return -1;
	}
	memset(sql, 0, BUFSIZE * BUFSIZE);
	va_list ap;
	va_start(ap, fmt);
	vsprintf(sql, fmt, ap);
	va_end(ap);

	write_log("execute sql：%s.\n", sql);
	ping();
	if (mysql_real_query(conn, sql, strlen(sql))) {
		write_log("execute sql error:%s.\n", mysql_error(conn));
		free(sql);
		return -1;
	} else {
		free(sql);
		return 0;
	}
}
