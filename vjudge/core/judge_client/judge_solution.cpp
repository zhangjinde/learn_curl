#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <assert.h>
#include <features.h>

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

void fix_java_mis_judge(void) {
	int comp_res = OJ_AC;
	comp_res = execute_cmd("/bin/grep 'Exception'  %s/error.out", work_dir);
	if (!comp_res) {
		write_log("Exception reported\n");
		solution->result = OJ_RE;
	}
	comp_res = execute_cmd("/bin/grep 'java.lang.OutOfMemoryError'  %s/error.out", work_dir);
	if (!comp_res) {
		write_log("JVM need more Memory!");
		solution->result = OJ_ML;
		solution->memory = solution->problem_info.memory_limit * STD_MB;
	}
	comp_res = execute_cmd("/bin/grep 'java.lang.OutOfMemoryError'  %s/user.out", work_dir);
	if (!comp_res) {
		write_log("JVM need more Memory or Threads!");
		solution->result = OJ_ML;
		solution->memory = solution->problem_info.memory_limit * STD_MB;
	}
	comp_res = execute_cmd("/bin/grep 'Could not create'  %s/error.out", work_dir);
	if (!comp_res) {
		write_log("jvm need more resource,tweak -Xmx(OJ_JAVA_BONUS) Settings");
		solution->result = OJ_RE;
	}
}

void judge_solution(char *infile, char *outfile, char *userfile, double num_of_test)
{
	write_log("judge solution %d.\n", solution->solution_id);
	int comp_res;
	if (!oi_mode) {
		num_of_test = 1.0;
	}
	if (solution->result == OJ_AC && solution->time
			> (solution->problem_info.time_limit * 1000
			* (use_max_time ? 1 : num_of_test))) {
		solution->result = OJ_TL;
	}
	if (solution->memory > solution->problem_info.memory_limit * STD_MB) {
		solution->result = OJ_ML;
	}

	// compare
	if (solution->result == OJ_AC) {
		if (isspj) {
			comp_res = special_judge(solution->problem_info.problem_id, infile, outfile, userfile);
			if (comp_res == 0)
				comp_res = OJ_AC;
			else {
				comp_res = OJ_WA;
			}
		} else {
			comp_res = compare(outfile, userfile);
		}
		if (comp_res == OJ_WA) {
			solution->result = OJ_WA;
		} else if (comp_res == OJ_PE) {
			solution->ispe = OJ_PE;
		}
		solution->result = comp_res;
	}
	//jvm popup messages, if don't consider them will get miss-WrongAnswer
	if (solution->language == 3) {
		fix_java_mis_judge();
	}
}
