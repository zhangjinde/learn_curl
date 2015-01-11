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
extern char work_dir[BUFSIZE];
extern char java_xms[BUFSIZE];
extern char java_xmx[BUFSIZE];
extern char LANG_NAME[BUFSIZE];
extern char lang_ext[15][8];
extern MYSQL *conn;
extern struct solution_t *solution;
extern int call_counter[BUFSIZE];
extern int call_array_size;

int special_judge(char *infile, char *outfile, char *userfile)
{
	write_log("start special judge solution %d.\n", solution->solution_id);
	pid_t pid;
	pid = fork();
	int ret = 0;
	if (pid < 0) {
		write_log("fork error.\n");
	} else if (pid == 0) {
		while (setgid(1536) != 0) {
			sleep(1);
		}
		while (setuid(1536) != 0) {
			sleep(1);
		}
		while (setresuid(1536, 1536, 1536) != 0) {
			sleep(1);
		}
		struct rlimit LIM;	// time limit, file limit& memory limit
		LIM.rlim_cur = 5;
		LIM.rlim_max = LIM.rlim_cur;
		setrlimit(RLIMIT_CPU, &LIM);
		alarm(0);
		alarm(10);
		// file limit
		LIM.rlim_max = STD_F_LIM + STD_MB;
		LIM.rlim_cur = STD_F_LIM;
		setrlimit(RLIMIT_FSIZE, &LIM);
		ret = execute_cmd("%s/data/%d/spj %s %s %s", oj_home,
				solution->problem_info.problem_id, infile,
				outfile, userfile);
		if (ret) {
			exit(EXIT_FAILURE);
		} else {
			exit(EXIT_SUCCESS);
		}
	} else {
		int status;
		waitpid(pid, &status, 0);
		ret = WEXITSTATUS(status);
	}
	return ret;
}
