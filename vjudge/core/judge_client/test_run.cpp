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

#include "okcalls.h"
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
extern const int call_array_size;

void init_syscalls_limits(int lang)
{
	int i;
	memset(call_counter, 0, sizeof(call_counter));
	if (DEBUG)
		write_log("init_call_counter:%d", lang);
	if (record_call) {	// C & C++
		for (i = 0; i < call_array_size; i++) {
			call_counter[i] = 0;
		}
	} else if (lang <= 1 || lang == 13 || lang == 14) {	// C & C++
		for (i = 0; i == 0 || LANG_CV[i]; i++) {
			call_counter[LANG_CV[i]] = HOJ_MAX_LIMIT;
		}
	} else if (lang == 2) {	// Pascal
		for (i = 0; i == 0 || LANG_PV[i]; i++)
			call_counter[LANG_PV[i]] = HOJ_MAX_LIMIT;
	} else if (lang == 3) {	// Java
		for (i = 0; i == 0 || LANG_JV[i]; i++)
			call_counter[LANG_JV[i]] = HOJ_MAX_LIMIT;
	} else if (lang == 4) {	// Ruby
		for (i = 0; i == 0 || LANG_RV[i]; i++)
			call_counter[LANG_RV[i]] = HOJ_MAX_LIMIT;
	} else if (lang == 5) {	// Bash
		for (i = 0; i == 0 || LANG_BV[i]; i++)
			call_counter[LANG_BV[i]] = HOJ_MAX_LIMIT;
	} else if (lang == 6) {	// Python
		for (i = 0; i == 0 || LANG_YV[i]; i++)
			call_counter[LANG_YV[i]] = HOJ_MAX_LIMIT;
	} else if (lang == 7) {	// php
		for (i = 0; i == 0 || LANG_PHV[i]; i++)
			call_counter[LANG_PHV[i]] = HOJ_MAX_LIMIT;
	} else if (lang == 8) {	// perl
		for (i = 0; i == 0 || LANG_PLV[i]; i++)
			call_counter[LANG_PLV[i]] = HOJ_MAX_LIMIT;
	} else if (lang == 9) {	// mono c#
		for (i = 0; i == 0 || LANG_CSV[i]; i++)
			call_counter[LANG_CSV[i]] = HOJ_MAX_LIMIT;
	} else if (lang == 10) {	//objective c
		for (i = 0; i == 0 || LANG_OV[i]; i++)
			call_counter[LANG_OV[i]] = HOJ_MAX_LIMIT;
	} else if (lang == 11) {	//free basic
		for (i = 0; i == 0 || LANG_BASICV[i]; i++)
			call_counter[LANG_BASICV[i]] = HOJ_MAX_LIMIT;
	} else if (lang == 12) {	//scheme guile
		for (i = 0; i == 0 || LANG_SV[i]; i++)
			call_counter[LANG_SV[i]] = HOJ_MAX_LIMIT;
	}
}

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
		write_log("no custom intput %d.\n", sid);
		mysql_free_result(result);
		return -1;
	}
	mysql_free_result(result);
	return 0;
}

void test_run(void)
{
		printf("running a custom input...\n");
		get_custominput(solution_id, work_dir);
		init_syscalls_limits(lang);
		pid_t pidApp = fork();

		if (pidApp == 0) {	//在子进程中
			//运行编译后的程序,生成用户产生的结果user.out文件
			run_solution(lang, work_dir, time_lmt, usedtime,
				     mem_lmt);
		} else {	//父进程中
			watch_solution(pidApp, infile, &ACflg, isspj, userfile,
				       outfile, solution_id, lang, &topmemory,
				       mem_lmt, &usedtime, time_lmt, p_id, PEflg,
				       work_dir);
		}
		if (ACflg == OJ_TL) {
			usedtime = time_lmt * 1000;
		}
		if (ACflg == OJ_RE) {
			if (DEBUG)
				printf("add RE info of %d..... \n",
				       solution_id);
			addreinfo(solution_id, "error.out");
		} else {
			addcustomout(solution_id);
		}
		update_solution(solution_id, OJ_TR, usedtime, topmemory >> 10,
				0, 0, 0);
}
