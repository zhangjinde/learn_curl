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
extern char work_dir[BUFSIZE];
extern char db_passwd[BUFSIZE];
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

int save_custom_input(void)
{
	char src_path[BUFSIZE];
	// get the source code
	if (execute_sql("SELECT input_text FROM custominput WHERE "
			"solution_id=%d", solution->solution_id) < 0) {
		return -1;
	}
	MYSQL_RES *result = mysql_store_result(conn);
	if (result == NULL) {
		write_log("read mysql result error:%s.\n", mysql_error(conn));
		return -1;
	}
	my_ulonglong cnt = mysql_num_rows(result);
	if (cnt > 0) {
		MYSQL_ROW row = mysql_fetch_row(result);
		if (row == NULL) {
			mysql_free_result(result);
			write_log("fetch mysql row error:%s\n", mysql_error(conn));
			return -1;
		}
		// create the src file
		sprintf(src_path, "data.in");
		FILE *fp_src = fopen(src_path, "w");
		if (fp_src == NULL) {
			write_log("can't create file %s.\n", src_path);
			return -1;
		}
		fprintf(fp_src, "%s", row[0]);
		fclose(fp_src);
		if (DEBUG) {
			write_log("solution_id = %d\n", solution->solution_id);
		}
	} else {
		write_log("no custom intput %d.\n", solution->solution_id);
		mysql_free_result(result);
		return -1;
	}
	mysql_free_result(result);
	return 0;
}

int addcustomout(int solution_id)
{
	return addreinfo(solution_id, "user.out");
}

void test_run(void)
{
	save_custom_input();
	init_syscalls_limits(solution->language);

	pid_t pid = fork();
	if (pid == 0) {		//在子进程中
		//运行编译后的程序,生成用户产生的结果user.out文件
		run_solution();
	} else {		//父进程中
		watch_solution(pid, NULL, NULL);
	}
	if (solution->result == OJ_TL) {
		solution->time = solution->problem_info.time_limit * 1000;
	}
	if (solution->result == OJ_RE) {
		addreinfo(solution->solution_id, "error.out");
	} else {
		addcustomout(solution->solution_id);
	}
	solution->result = OJ_TR;
	update_solution();
}
