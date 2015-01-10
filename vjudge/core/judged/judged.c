/*
 * Copyright 2008 sempr <iamsempr@gmail.com>
 *
 * Refacted and modified by zhblue<newsclan@gmail.com> 
 * Bug report email newsclan@gmail.com
 * 
 * This file is part of HUSTOJ.
 *
 * HUSTOJ is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HUSTOJ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HUSTOJ. if not, see <http://www.gnu.org/licenses/>.
 */

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <mysql/mysql.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/resource.h>

#define BUFFER_SIZE 1024
#define LOCKFILE "/var/run/judged.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define STD_MB 1048576

#define OJ_WT0 0			//Pending
#define OJ_WT1 1			//Pending Rejudge
#define OJ_CI 2				//Compiling
#define OJ_RI 3				//Running & Judging
#define OJ_AC 4				//Accepted
#define OJ_PE 5				//Persentation Error
#define OJ_WA 6				//Wrong Answer
#define OJ_TL 7				//Time Limit Exceeded
#define OJ_ML 8				//Memory Limit Exceeded
#define OJ_OL 9				//Output Limit Exceeded
#define OJ_RE 10			//Runtime Error
#define OJ_CE 11			//Compile Error
#define OJ_CO 12

int db_port;
int max_running;
int sleep_time;
int db_timeout;
int oj_tot;
int oj_mod;
char db_host[BUFFER_SIZE];
char db_user[BUFFER_SIZE];
char db_passwd[BUFFER_SIZE];
char db_name[BUFFER_SIZE];
char oj_home[BUFFER_SIZE];
char oj_lang_set[BUFFER_SIZE];

int STOP = 0;
int DEBUG = 0;

MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;
char query[BUFFER_SIZE];

void call_for_exit(int s)
{
	STOP = 1;
	printf("Stopping judged...\n");
}

int write_log(const char *fmt, ...)
{
	va_list ap;
	char buffer[BUFFER_SIZE * 4];
	sprintf(buffer, "%s/log/judged.log", oj_home);
	FILE *fp = fopen(buffer, "a+");
	if (DEBUG) {
		freopen("/dev/stdout", "w", fp);
	}
	if (fp == NULL) {
		fprintf(stderr, "open log file error:%s.\n", strerror(errno));
		return 0;
	}
	va_start(ap, fmt);
	vsprintf(buffer, fmt, ap);
	time_t tm = time(NULL);
	char timestr[BUFFER_SIZE];
	sprintf(timestr, "%s", ctime(&tm));
	int len = strlen(timestr);
	timestr[len - 1] = '\0';
	int ret = fprintf(fp, "[%s]:%s", timestr, buffer);
	va_end(ap);
	fclose(fp);
	return ret;
}

int after_equal(char *c)
{
	int i = 0;
	for (; c[i] != '\0' && c[i] != '='; i++) ;
	return ++i;
}

void trim(char *c)
{
	char buf[BUFFER_SIZE];
	char *start, *end;
	strcpy(buf, c);
	start = buf;
	while (isspace(*start))
		start++;
	end = start;
	while (!isspace(*end))
		end++;
	*end = '\0';
	strcpy(c, start);
}

int read_buf(char *buf, const char *key, char *value)
{
	if (strncmp(buf, key, strlen(key)) == 0) {
		strcpy(value, buf + after_equal(buf));
		trim(value);
		write_log("%s = %s\n", key, value);
		return 1;
	}
	return 0;
}

void read_int(char *buf, const char *key, int *value)
{
	char buf2[BUFFER_SIZE];
	if (read_buf(buf, key, buf2)) {
		sscanf(buf2, "%d", value);
	}
}

// read the configue file
void init_mysql_conf()
{
	write_log("init mysql config.\n");
	FILE *fp = NULL;
	char buf[BUFFER_SIZE];
	db_port = 3306;
	max_running = 4;
	sleep_time = 1;
	oj_tot = 1;
	oj_mod = 0;
	strcpy(oj_lang_set, "0,1,2,3,4,5,6,7,8,9,10");
	fp = fopen("./etc/judge.conf", "r");
	if (fp != NULL) {
		while (fgets(buf, BUFFER_SIZE - 1, fp)) {
			read_buf(buf, "DB_HOST", db_host);
			read_buf(buf, "DB_USER", db_user);
			read_buf(buf, "DB_PASSWD", db_passwd);
			read_buf(buf, "DB_NAME", db_name);
			read_int(buf, "DB_PORT", &db_port);
			read_int(buf, "DB_TIMEOUT", &db_timeout);
			read_int(buf, "MAX_RUNNING", &max_running);
			read_int(buf, "SLEEP_TIME", &sleep_time);
			read_int(buf, "OJ_TOTAL", &oj_tot);
			read_int(buf, "OJ_MOD", &oj_mod);
			read_buf(buf, "OJ_LANG_SET", oj_lang_set);

		}
		//从数据库中选出没有判过的提交
		//limit限定查询出来的结果数
		sprintf(query, "SELECT solution_id FROM solution WHERE "
				"language in (%s) and result<2 and "
				"MOD(solution_id,%d)=%d ORDER BY result "
				"ASC,solution_id ASC limit %d",
				oj_lang_set, oj_tot, oj_mod, max_running * 2);
		fclose(fp);
	} else {
		write_log("init mysql config error:%s.\n", strerror(errno));
	}
}

void run_client(int runid, int clientid)
{
	char clientidstr[BUFFER_SIZE], runidstr[BUFFER_SIZE];
	struct rlimit LIM;
	LIM.rlim_max = 800;
	LIM.rlim_cur = 800;
	setrlimit(RLIMIT_CPU, &LIM);

	LIM.rlim_max = 80 * STD_MB;
	LIM.rlim_cur = 80 * STD_MB;
	setrlimit(RLIMIT_FSIZE, &LIM);

	LIM.rlim_max = STD_MB << 11;
	LIM.rlim_cur = STD_MB << 11;
	setrlimit(RLIMIT_AS, &LIM);

	LIM.rlim_cur = LIM.rlim_max = 200;
	setrlimit(RLIMIT_NPROC, &LIM);

	sprintf(runidstr, "%d", runid);
	sprintf(clientidstr, "%d", clientid);

	write_log("run client%d for judge solution %d.\n", clientid, runid);
	//运行判题客户端程序
	if (!DEBUG) {
		execl("/usr/bin/judge_client", "/usr/bin/judge_client",
		      runidstr, clientidstr, oj_home, (char *)NULL);
	} else {
		execl("/usr/bin/judge_client", "/usr/bin/judge_client",
		      runidstr, clientidstr, oj_home, "debug", (char *)NULL);
	}
}

int executesql(const char *sql)
{
	write_log("execute sql：%s.\n", sql);
	if (mysql_real_query(conn, sql, strlen(sql))) {
		write_log("execute sql error:%s.\n", mysql_error(conn));
		sleep(db_timeout);
		conn = NULL;
		return 1;
	} else {
		return 0;
	}
}

// 返回0表示执行成功
int init_mysql()
{
	if (conn == NULL) {
		conn = mysql_init(NULL);	// init the database connection
		mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &db_timeout);

		write_log("try to connect database\n");
		if (!mysql_real_connect(conn, db_host, db_user, db_passwd,
					db_name, db_port, 0, 0)) {
			write_log("connect database error:%s.\n", mysql_error(conn));
			sleep(sleep_time);
			return 1;
		} else {
			return 0;
		}
	} else {
		return executesql("set names utf8");
	}
}

int get_jobs(int *jobs)
{
	write_log("get jobs.\n");
	if (mysql_real_query(conn, query, strlen(query))) {
		write_log("get jobs error:%s\n", mysql_error(conn));
		sleep(db_timeout);
		return 0;
	}
	res = mysql_store_result(conn);
	int i = 0;
	int ret = 0;
	while ((row = mysql_fetch_row(res)) != NULL) {
		jobs[i++] = atoi(row[0]);
	}
	ret = i;
	while (i <= max_running * 2) {
		jobs[i++] = 0;
	}
	return ret;
}

int check_out(int solution_id, int result)
{
	write_log("update solution %d for compiling.\n", solution_id);
	char sql[BUFFER_SIZE];
	sprintf(sql,
		"UPDATE solution SET result=%d,time=0,memory=0,judgetime=NOW() WHERE solution_id=%d and result<2 LIMIT 1",
		result, solution_id);
	if (mysql_real_query(conn, sql, strlen(sql))) {
		return 0;
	} else {
		if (mysql_affected_rows(conn) > 0) {
			return 1;
		} else {
			return 0;
		}
	}
}

int work(void)
{
	static int retcnt = 0;
	int i = 0;
	static pid_t ID[100];
	static int workcnt = 0;
	int runid = 0;
	int jobs[max_running * 2 + 1];
	pid_t tmp_pid = 0;

	/* get the database info */
	//获取判题任务
	if (!get_jobs(jobs)) {
		retcnt = 0;
	}

	/* exec the submit */
	int j = 0;
	for (j = 0; jobs[j] > 0; j++) {
		runid = jobs[j];
		if (runid % oj_tot != oj_mod) {
			continue;
		}
		write_log("judging solution %d.\n", runid);
		if (workcnt >= max_running) {	// if no more client can running
			//总共有4个判题的进程,等待任何一个退出,可以在配置
			//文件中设置个数
			tmp_pid = waitpid(-1, NULL, 0);	// wait 4 one child exit
			workcnt--;
			retcnt++;
			for (i = 0; i < max_running; i++) {	// get the client id
				if (ID[i] == tmp_pid) {
					break;	// got the client id
				}
			}
			ID[i] = 0;
		} else {	// have free client
			for (i = 0; i < max_running; i++)	// find the client id
				if (ID[i] == 0) {
					break;		// got the client id
				}
		}
		if (workcnt < max_running && check_out(runid, OJ_CI)) {
			workcnt++;
			ID[i] = fork();		// start to fork
			if (ID[i] == 0) {
				write_log("judge solution %d in client%d.\n",
						runid, i);
				// 子进程运行判题客户端
				run_client(runid, i);	// if the process is the son, run it
				exit(EXIT_SUCCESS);
			}
		} else {
			ID[i] = 0;
		}
	}
	while ((tmp_pid = waitpid(-1, NULL, WNOHANG)) > 0) {
		workcnt--;
		retcnt++;
		for (i = 0; i < max_running; i++) {	// get the client id
			if (ID[i] == tmp_pid) {
				break;	// got the client id
			}
		}
		ID[i] = 0;
		write_log("client%d judge done.\n", i);
	}
	mysql_free_result(res);	// free the memory
	executesql("commit");
	write_log("total %d solution judge done.\n", retcnt);

	return retcnt;
}

int lockfile(int fd)
{
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return (fcntl(fd, F_SETLK, &fl));
}

int already_running()
{
	int fd;
	char buf[16];
	fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);
	if (fd < 0) {
		write_log("can't open %s: %s.\n", LOCKFILE, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (lockfile(fd) < 0) {
		if (errno == EACCES || errno == EAGAIN) {
			close(fd);
			return 1;
		}
		write_log("can't lock %s: %s.\n", LOCKFILE, strerror(errno));
		exit(EXIT_FAILURE);
	}
	ftruncate(fd, 0);
	sprintf(buf, "%d", getpid());
	write(fd, buf, strlen(buf) + 1);

	return 0;
}

int daemon_init(void)
{
	pid_t pid = fork();
	if (pid < 0) {
		return -1;
	} else if (pid != 0) {
		exit(EXIT_SUCCESS);	/* parent exit */
	}

	/* child continues */
	setsid();		/* become session leader */
	chdir(oj_home);		/* change working directory */
	umask(0);		/* clear file mode creation mask */
	close(0);		/* close stdin */
	close(1);		/* close stdout */
	close(2);		/* close stderr */

	return 0;
}

/*
 * Usage: ./judged oj_home [debug]
 */
int main(int argc, char *argv[])
{
	DEBUG = (argc > 2);
	//如果第2个参数指定了家目录,就设置为那个目录
	//在debug时有用
	//否则默认为/home/judge
	if (argc > 1) {
		strcpy(oj_home, argv[1]);
	} else {
		strcpy(oj_home, "/home/judge");
	}

	chdir(oj_home);		// change the dir

	//不调试就设为守护进程
	if (!DEBUG) {
		daemon_init();
	}

	//进程已经运行了
	if (strcmp(oj_home, "/home/judge") == 0 && already_running()) {
		write_log("This daemon program is already running.\n");
		exit(EXIT_FAILURE);
	}

	init_mysql_conf();	// set the database info

	//设置信号的回调函数
	signal(SIGQUIT, call_for_exit);
	signal(SIGKILL, call_for_exit);
	signal(SIGTERM, call_for_exit);

	int j = 1;
	while (1) {		// start to run
		//从数据库中查询出来的没有判的题目判完
		//然后一直询问
		while (j && !init_mysql()) {
			j = work();
		}
		write_log("next query after %d seconds.\n", sleep_time);
		sleep(sleep_time);
		j = 1;
	}

	return 0;
}
