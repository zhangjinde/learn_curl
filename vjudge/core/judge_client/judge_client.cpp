//
// File:   judge_client.c
/*
 * Copyright 2008 sempr <iamsempr@gmail.com>
 *
 * Refacted and modified by zhblue<newsclan@gmail.com>
 * Bug report email newsclan@gmail.com
 *
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

int DEBUG = 1;
int db_port;
int shm_run = 0;
int max_running;
int db_timeout;
int sleep_time;
int java_time_bonus = 5;
int java_memory_bonus = 512;
int sim_enable = 0;
int oi_mode = 0;
int use_max_time = 0;
char record_call = 0;
char db_host[BUFSIZE];
char work_dir[BUFSIZE];
char db_user[BUFSIZE];
char db_passwd[BUFSIZE];
char db_name[BUFSIZE];
char oj_home[BUFSIZE];
char java_xms[BUFSIZE];
char java_xmx[BUFSIZE];
int call_counter[BUFSIZE] = {0};
char LANG_NAME[BUFSIZE];
int call_array_size = 512;
char lang_ext[15][8] = {"c", "cc", "pas", "java", "rb", "sh", "py",
	"php", "pl", "cs", "m", "bas", "scm", "c", "cc"};
MYSQL *conn;
struct solution_t *solution;

// read the configue file
void init_conf()
{
	write_log("init mysql config.\n");
	char buf[BUFSIZE];
	db_port = 3306;
	max_running = 4;
	sleep_time = 5;
	strcpy(java_xms, "-Xms32m");
	strcpy(java_xmx, "-Xmx256m");
	sprintf(buf, "%s/etc/judge.conf", oj_home);
	FILE *fp = fopen("./etc/judge.conf", "r");
	if (fp != NULL) {
		while (fgets(buf, BUFSIZE - 1, fp) != NULL) {
			read_buf(buf, "DB_HOST", db_host);
			read_buf(buf, "DB_USER", db_user);
			read_buf(buf, "DB_PASSWD", db_passwd);
			read_buf(buf, "DB_NAME", db_name);
			read_int(buf, "DB_PORT", &db_port);
			read_int(buf, "DB_TIMEOUT", &db_timeout);
			read_int(buf, "JAVA_TIME_BONUS", &java_time_bonus);
			read_int(buf, "JAVA_MEMORY_BONUS", &java_memory_bonus);
			read_int(buf, "SIM_ENABLE", &sim_enable);
			read_buf(buf, "JAVA_XMS", java_xms);
			read_buf(buf, "JAVA_XMX", java_xmx);
			read_int(buf, "OI_MODE", &oi_mode);
			read_int(buf, "SHM_RUN", &shm_run);
			read_int(buf, "USE_MAX_TIME", &use_max_time);
		}
		fclose(fp);
	} else {
		write_log("init mysql config error:%s.\n", strerror(errno));
	}
}

int isinfile(const char fname[])
{
	int l = strlen(fname);
	if (l <= 3 || strcmp(fname + l - 3, ".in") != 0) {
		return 0;
	} else {
		return l - 3;
	}
}

int adddiffinfo(int solution_id)
{
	return addreinfo(solution_id, "diff.out");
}

void prepare_files(char *filename, int namelen, char *infile, char *outfile,
		char *userfile, int runner_id)
{
	write_log("prepare necessary files.\n");
	int p_id = solution->problem_info.problem_id;
	char fname[BUFSIZE];
	memset(fname, 0, sizeof(fname));
	strncpy(fname, filename, namelen);
	sprintf(infile, "%s/data/%d/%s.in", oj_home, p_id, fname);
	execute_cmd("/bin/cp %s %s/data.in", infile, work_dir);
	execute_cmd("/bin/cp %s/data/%d/*.dic %s/", oj_home, p_id, work_dir);
	sprintf(outfile, "%s/data/%d/%s.out", oj_home, p_id, fname);
	sprintf(userfile, "%s/run%d/user.out", oj_home, runner_id);
}

void clean_session(pid_t p)
{
	//char cmd[BUFSIZE];
	const char *pre = "ps awx -o \"\%p \%P\"|grep -w ";
	const char *post = " | awk \'{ print $1  }\'|xargs kill -9";
	execute_cmd("%s %d %s", pre, p, post);
	execute_cmd("ps aux |grep \\^judge|awk '{print $2}'|xargs kill");
}

void clean_workdir(void)
{
	execute_cmd("/bin/umount %s/proc", work_dir);
	if (DEBUG) {
		execute_cmd("/bin/mv %s/* %slog/", work_dir, work_dir);
	} else {
		execute_cmd("/bin/rm -Rf %s/*", work_dir);
	}

}

void mk_shm_workdir(void)
{
	char shm_path[BUFSIZE];
	sprintf(shm_path, "/dev/shm/zzuoj/%s", work_dir);
	execute_cmd("/bin/mkdir -p %s", shm_path);
	execute_cmd("/bin/rm -rf %s", work_dir);
	execute_cmd("/bin/ln -s %s %s/", shm_path, oj_home);
	execute_cmd("/bin/chown judge %s ", shm_path);
	execute_cmd("chmod 755 %s ", shm_path);
	//sim need a soft link in shm_dir to work correctly
	sprintf(shm_path, "/dev/shm/zzuoj/%s/", oj_home);
	execute_cmd("/bin/ln -s %s/data %s", oj_home, shm_path);
}

void print_call_array()
{
	printf("int LANG_%sV[256]={", LANG_NAME);
	int i = 0;
	for (i = 0; i < call_array_size; i++) {
		if (call_counter[i]) {
			printf("%d,", i);
		}
	}
	printf("0};\n");

	printf("int LANG_%sC[256]={", LANG_NAME);
	for (i = 0; i < call_array_size; i++) {
		if (call_counter[i]) {
			printf("HOJ_MAX_LIMIT,");
		}
	}
	printf("0};\n");
}

int main(int argc, char **argv)
{
	int solution_id = 1000;
	int runner_id = 0;

	//获取参数,得到solution_id,runner_id,如果指定了oj_home就设置oj_home
	init_parameters(argc, argv, &solution_id, &runner_id);

	//从judge.conf文件中读取各种字段
	init_conf();

	//set work directory to start running & judging
	sprintf(work_dir, "%s/run%d/", oj_home, runner_id);

	write_log("work_dir = %s\n", work_dir);

	//如果使用了/dev/shm的共享内存虚拟磁盘来运行答案
	//启用能提高判题速度，但需要较多内存。
	if (shm_run) {
		mk_shm_workdir();
	}

	chdir(work_dir);

	if (!DEBUG) {
		clean_workdir();
	}

	conn = prepare_mysql();

	// 获取solution的各种信息
	solution = get_solution(solution_id);
	if (solution == NULL) {
		write_log("get solution %d error.\n", solution_id);
		exit(EXIT_FAILURE);
	}

	//将提交的源代码存放在work_dir的Main.*文件中
	save_solution_src();

	//编译源文件
	solution->isce = compile();
	//编译不成功,将信息更新回数据库
	if (solution->isce) {
		write_log("solution %d compile error.\n", solution_id);
		solution->result = OJ_CE;
		update_solution();
		update_user();
		update_problem();
		if (!DEBUG) {
			clean_workdir();
		}
		cleanup_mysql();
		free(solution);
		exit(EXIT_SUCCESS);
	} else {
		//如果编译成功就设置为正在运行
		write_log("solution %d running.\n", solution->solution_id);
		solution->result = OJ_RI;
		update_solution();
	}

	// test run
	if (solution->problem_info.problem_id == 0) {
		test_run();
		free(solution);
		cleanup_mysql();
		exit(EXIT_SUCCESS);
	}

	// virtual judge
	if (solution->problem_info.ojtype >= 0) {
		vjudge();
		cleanup_mysql();
		free(solution);
		exit(EXIT_SUCCESS);
	}

	// run
	char fullpath[BUFSIZE];
	char infile[BUFSIZE];
	char outfile[BUFSIZE];
	char userfile[BUFSIZE];
	double pass_rate = 0.0;
	int max_case_time = 0;
	int num_of_test = 0;
	int finalACflg = OJ_AC;

	// the fullpath of data dir
	sprintf(fullpath, "%s/data/%d", oj_home, solution->problem_info.problem_id);

	write_log("fullpath = %s\n", fullpath);

	// open DIRs
	DIR *dp;
	struct dirent *dirp;
	//打开存放测试数据的目录,如果失败,则返回
	if (solution->problem_info.problem_id > 0
			&& (dp = opendir(fullpath)) == NULL) {
		write_log("no such dir: %s!\n", fullpath);
		free(solution);
		cleanup_mysql();
		exit(EXIT_FAILURE);
	}

	// 为某些语言拷贝运行时库
	copy_runtime();

	solution->result = OJ_AC;
	while ((oi_mode || solution->result == OJ_AC) && (dirp = readdir(dp)) != NULL) {
		// check if the file is *.in or not
		int namelen = isinfile(dirp->d_name);
		if (namelen == 0) {
			continue;
		}

		prepare_files(dirp->d_name, namelen, infile, outfile,
				userfile, runner_id);
		init_syscalls_limits(solution->language);

		pid_t pid = fork();
		if (pid < 0) {
			write_log("fork error:%s.\n", strerror(errno));
		} else if (pid == 0) {		// son run solution
			run_solution();
		} else {
			num_of_test++;
			watch_solution(pid, outfile, userfile);
			judge_solution(infile, outfile, userfile, num_of_test);
			if (use_max_time) {
				max_case_time = solution->time > max_case_time ? solution->time : max_case_time;
				solution->time = 0;
			}
		}
		if (oi_mode) {
			if (solution->result == OJ_AC) {
				++pass_rate;
			}
			if (finalACflg < solution->result) {
				finalACflg = solution->result;
			}
			solution->result = OJ_AC;
		}
		write_log("judge test data %d done.\n", num_of_test);
	}

	if (solution->result == OJ_AC && solution->ispe == OJ_PE) {
		solution->result = OJ_PE;
	}

	switch (solution->result) {
		case OJ_WT0: write_log("solution %d pending.\n", solution_id); break;
		case OJ_WT1: write_log("solution %d pending rejudge.\n", solution_id); break;
		case OJ_CI: write_log("solution %d compiling.\n", solution_id); break;
		case OJ_RI: write_log("solution %d running and judging.\n", solution_id); break;
		case OJ_AC: write_log("solution %d accepted.\n", solution_id); break;
		case OJ_PE: write_log("solution %d persentation error.\n", solution_id); break;
		case OJ_WA: write_log("solution %d wrong answer.\n", solution_id); break;
		case OJ_TL: write_log("solution %d time limit exceeded.\n", solution_id); break;
		case OJ_ML: write_log("solution %d memory limit exceeded.\n", solution_id); break;
		case OJ_OL: write_log("solution %d output limit exceeded.\n", solution_id); break;
		case OJ_RE: write_log("solution %d runtime error.\n", solution_id); break;
		case OJ_CE: write_log("solution %d complie error.\n", solution_id); break;
		default: write_log("solution %d result is %d.\n", solution_id, solution->result);
	}

	if (solution->result == OJ_AC) {
	}
	if (solution->problem_info.ischa && sim_enable
			&& solution->result == OJ_AC
			&& (!oi_mode || finalACflg == OJ_AC)
			&& solution->language < 5) {	//bash don't supported
		get_sim();
		update_sim();
	}

	if ((oi_mode && finalACflg == OJ_RE) || solution->result == OJ_RE) {
		addreinfo(solution_id, "error.out");
	}
	if (use_max_time) {
		solution->time = max_case_time;
	}
	if (solution->result == OJ_TL) {
		solution->time = solution->problem_info.time_limit * 1000;
	}

	solution->memory >>= 10;
	if (oi_mode) {
		if (num_of_test > 0) {
			pass_rate /= num_of_test;
		}
		solution->pass_rate = pass_rate;
		solution->result = finalACflg;
	}

	update_solution();

	if ((oi_mode && finalACflg == OJ_WA) || solution->result == OJ_WA) {
		if (!solution->problem_info.spj) {
			adddiffinfo(solution_id);
		}
	}

	update_user();
	update_problem();
	if (!DEBUG) {
		clean_workdir();
	}
	cleanup_mysql();
	free(solution);
	closedir(dp);
	if (record_call) {
		print_call_array();
	}

	return 0;
}
