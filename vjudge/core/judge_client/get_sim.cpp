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

int get_sim(int solution_id, int lang, int pid)
{
	int sim_s_id;
	char src_pth[BUFSIZE];
	sprintf(src_pth, "Main.%s", lang_ext[lang]);

	int sim = 0;
	sim = execute_cmd("/usr/bin/sim.sh %s %d", src_pth, pid);
	if (DEBUG) {
		write_log("get_sim : sim = %d", sim);
	}
	if (!sim) {
		execute_cmd("/bin/mkdir ../data/%d/ac/", pid);
		execute_cmd("/bin/cp %s ../data/%d/ac/%d.%s", src_pth, pid,
			    solution_id, lang_ext[lang]);
		//c cpp will
		//c和cpp互相查重
		if (lang == 0) {
			execute_cmd
			    ("/bin/ln ../data/%d/ac/%d.%s ../data/%d/ac/%d.%s",
			     pid, solution_id, lang_ext[lang], pid, solution_id,
			     lang_ext[lang + 1]);
		}
		if (lang == 1) {
			execute_cmd
			    ("/bin/ln ../data/%d/ac/%d.%s ../data/%d/ac/%d.%s",
			     pid, solution_id, lang_ext[lang], pid, solution_id,
			     lang_ext[lang - 1]);
		}
		//write_log("!sim");
	} else {
		FILE *pf;
		pf = fopen("sim", "r");
		//write_log("sim");
		if (pf) {
			//从sim文件中读取相似的运行号
			//这里将重复的运行号都插入数据库
			//因为后边还会进行一次插入，可能会产生一个
			//数据库错误，因为插入了主键相同的元素。
			while (fscanf(pf, "%d%d", &sim, &sim_s_id) != EOF) {
				if (DEBUG) {
					write_log("sim : sim_s_id = %d : %d",
						  sim, sim_s_id);
				}
				if (sim_s_id > solution_id) {
				}
			}
			fclose(pf);
		} else {
			write_log("open file sim error");
		}

	}
	if (solution_id <= sim_s_id)
		sim = 0;
	return 0;
}
