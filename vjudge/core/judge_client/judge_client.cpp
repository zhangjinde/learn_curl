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

#include "okcalls.h"
#include "judge_client.h"

#define ZOJ_COM

/*copy from ZOJ
 http://code.google.com/p/zoj/source/browse/trunk/judge_client/client/tracer.cc?spec=svn367&r=367#39
 */
#ifdef __i386
#define REG_SYSCALL orig_eax
#define REG_RET eax
#define REG_ARG0 ebx
#define REG_ARG1 ecx
#else
#define REG_SYSCALL orig_rax
#define REG_RET rax
#define REG_ARG0 rdi
#define REG_ARG1 rsi
#endif

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
const int call_array_size = 512;
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

void find_next_nonspace(int *c1, int *c2, FILE **f1, FILE **f2, int *ret)
{
	// Find the next non-space character or \n.
	while ((isspace(*c1)) || (isspace(*c2))) {
		if (*c1 != *c2) {
			if (*c2 == EOF) {
				do {
					*c1 = fgetc(*f1);
				} while (isspace(*c1));
				continue;
			} else if (*c1 == EOF) {
				do {
					*c2 = fgetc(*f2);
				} while (isspace(*c2));
				continue;
			} else if ((*c1 == '\r' && *c2 == '\n')) {
				*c1 = fgetc(*f1);
			} else if ((*c2 == '\r' && *c1 == '\n')) {
				*c2 = fgetc(*f2);
			} else {
				if (DEBUG)
					printf("%d=%c\t%d=%c", *c1, *c1, *c2, *c2);
				;
				*ret = OJ_PE;
			}
		}
		if (isspace(*c1)) {
			*c1 = fgetc(*f1);
		}
		if (isspace(*c2)) {
			*c2 = fgetc(*f2);
		}
	}
}

/***
 int compare_diff(const char *file1,const char *file2){
 char diff[1024];
 sprintf(diff,"diff -q -B -b -w --strip-trailing-cr %s %s",file1,file2);
 int d=system(diff);
 if (d) return OJ_WA;
 sprintf(diff,"diff -q -B --strip-trailing-cr %s %s",file1,file2);
 int p=system(diff);
 if (p) return OJ_PE;
 else return OJ_AC;

 }
 */
const char *getFileNameFromPath(const char *path)
{
	int i = 0;
	for (i = strlen(path); i >= 0; i--) {
		if (path[i] == '/')
			return &path[i];
	}
	return path;
}

void make_diff_out(FILE * f1, FILE * f2, int c1, int c2, const char *path)
{
	FILE *out;
	char buf[45];
	out = fopen("diff.out", "a+");
	fprintf(out, "=================%s\n", getFileNameFromPath(path));
	fprintf(out, "Right:\n%c", c1);
	if (fgets(buf, 44, f1)) {
		fprintf(out, "%s", buf);
	}
	fprintf(out, "\n-----------------\n");
	fprintf(out, "Your:\n%c", c2);
	if (fgets(buf, 44, f2)) {
		fprintf(out, "%s", buf);
	}
	fprintf(out, "\n=================\n");
	fclose(out);
}

/*
 * translated from ZOJ judger r367
 * http://code.google.com/p/zoj/source/browse/trunk/judge_client/client/text_checker.cc#25
 *
 */
int compare_zoj(const char *file1, const char *file2)
{
	int ret = OJ_AC;
	int c1, c2;
	FILE *f1, *f2;
	f1 = fopen(file1, "r");
	f2 = fopen(file2, "r");
	if (!f1 || !f2) {
		ret = OJ_RE;
	} else {
		for (;;) {
			// Find the first non-space character at the beginning of line.
			// Blank lines are skipped.
			c1 = fgetc(f1);
			c2 = fgetc(f2);
			find_next_nonspace(&c1, &c2, &f1, &f2, &ret);
			// Compare the current line.
			for (;;) {
				// Read until 2 files return a space or 0 together.
				while ((!isspace(c1) && c1)
				       || (!isspace(c2) && c2)) {
					if (c1 == EOF && c2 == EOF) {
						goto end;
					}
					if (c1 == EOF || c2 == EOF) {
						break;
					}
					if (c1 != c2) {
						// Consecutive non-space characters should be all exactly the same
						ret = OJ_WA;
						goto end;
					}
					c1 = fgetc(f1);
					c2 = fgetc(f2);
				}
				find_next_nonspace(&c1, &c2, &f1, &f2, &ret);
				if (c1 == EOF && c2 == EOF) {
					goto end;
				}
				if (c1 == EOF || c2 == EOF) {
					ret = OJ_WA;
					goto end;
				}

				if ((c1 == '\n' || !c1) && (c2 == '\n' || !c2)) {
					break;
				}
			}
		}
	}
end:	if (ret == OJ_WA)
		make_diff_out(f1, f2, c1, c2, file1);
	if (f1)
		fclose(f1);
	if (f2)
		fclose(f2);
	return ret;
}

void delnextline(char s[])
{
	int L;
	L = strlen(s);
	while (L > 0 && (s[L - 1] == '\n' || s[L - 1] == '\r'))
		s[--L] = 0;
}

int compare(const char *file1, const char *file2)
{
#ifdef ZOJ_COM
	//compare ported and improved from zoj don't limit file size
	return compare_zoj(file1, file2);
#endif
#ifndef ZOJ_COM
	//the original compare from the first version of hustoj has file size limit
	//and waste memory
	FILE *f1, *f2;
	char *s1, *s2, *p1, *p2;
	int PEflg;
	s1 = new char[STD_F_LIM + 512];
	s2 = new char[STD_F_LIM + 512];
	if (!(f1 = fopen(file1, "r")))
		return OJ_AC;
	for (p1 = s1; EOF != fscanf(f1, "%s", p1);)
		while (*p1)
			p1++;
	fclose(f1);
	if (!(f2 = fopen(file2, "r")))
		return OJ_RE;
	for (p2 = s2; EOF != fscanf(f2, "%s", p2);)
		while (*p2)
			p2++;
	fclose(f2);
	if (strcmp(s1, s2) != 0) {
		//              printf("A:%s\nB:%s\n",s1,s2);
		delete[]s1;
		delete[]s2;

		return OJ_WA;
	} else {
		f1 = fopen(file1, "r");
		f2 = fopen(file2, "r");
		PEflg = 0;
		while (PEflg == 0 && fgets(s1, STD_F_LIM, f1)
		       && fgets(s2, STD_F_LIM, f2)) {
			delnextline(s1);
			delnextline(s2);
			if (strcmp(s1, s2) == 0)
				continue;
			else
				PEflg = 1;
		}
		delete[]s1;
		delete[]s2;
		fclose(f1);
		fclose(f2);
		if (PEflg)
			return OJ_PE;
		else
			return OJ_AC;
	}
#endif
}

void update_solution1(int solution_id, int result, int time, int memory, int sim,
		     int sim_s_id, double pass_rate)
{
	if (result == OJ_TL && memory == 0) {
		result = OJ_ML;
	}
	char sql[BUFSIZE];
	if (oi_mode) {
		sprintf(sql,
			"UPDATE solution SET result=%d,time=%d,memory=%d,judgetime=NOW(),pass_rate=%f WHERE solution_id=%d LIMIT 1%c",
			result, time, memory, pass_rate, solution_id, 0);
	} else {
		sprintf(sql,
			"UPDATE solution SET result=%d,time=%d,memory=%d,judgetime=NOW() WHERE solution_id=%d LIMIT 1%c",
			result, time, memory, solution_id, 0);
	}
	//      printf("sql= %s\n",sql);
	if (mysql_real_query(conn, sql, strlen(sql))) {
		//              printf("..update failed! %s\n",mysql_error(conn));
	}
	if (sim) {
		sprintf(sql,
			"insert into sim(s_id,sim_s_id,sim) values(%d,%d,%d) on duplicate key update  sim_s_id=%d,sim=%d",
			solution_id, sim_s_id, sim, sim_s_id, sim);
		//      printf("sql= %s\n",sql);
		if (mysql_real_query(conn, sql, strlen(sql))) {
			//              printf("..update failed! %s\n",mysql_error(conn));
		}

	}
}

int adddiffinfo(int solution_id)
{
	return addreinfo(solution_id, "diff.out");
}


/*
 int read_proc_statm(int pid){
 FILE * pf;
 char fn[4096];
 int ret;
 sprintf(fn,"/proc/%d/statm",pid);
 pf=fopen(fn,"r");
 fscanf(pf,"%d",&ret);
 fclose(pf);
 return ret;
 }
 */
int get_proc_status(int pid, const char *mark)
{
	FILE *pf;
	char fn[BUFSIZE], buf[BUFSIZE];
	int ret = 0;
	sprintf(fn, "/proc/%d/status", pid);
	pf = fopen(fn, "r");
	int m = strlen(mark);
	while (pf && fgets(buf, BUFSIZE - 1, pf)) {

		buf[strlen(buf) - 1] = 0;
		if (strncmp(buf, mark, m) == 0) {
			sscanf(buf + m + 1, "%d", &ret);
		}
	}
	if (pf)
		fclose(pf);
	return ret;
}

void prepare_files(char *filename, int namelen, char *infile, char *outfile,
		char *userfile, int runner_id)
{
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

int fix_java_mis_judge(char *work_dir, int *ACflg, int *topmemory, int mem_lmt)
{
	int comp_res = OJ_AC;
	if (DEBUG)
		execute_cmd("cat %s/error.out", work_dir);
	comp_res = execute_cmd("/bin/grep 'Exception'  %s/error.out", work_dir);
	if (!comp_res) {
		printf("Exception reported\n");
		*ACflg = OJ_RE;
	}

	comp_res =
	    execute_cmd("/bin/grep 'java.lang.OutOfMemoryError'  %s/error.out",
			work_dir);

	if (!comp_res) {
		printf("JVM need more Memory!");
		*ACflg = OJ_ML;
		*topmemory = mem_lmt * STD_MB;
	}
	comp_res =
	    execute_cmd("/bin/grep 'java.lang.OutOfMemoryError'  %s/user.out",
			work_dir);

	if (!comp_res) {
		printf("JVM need more Memory or Threads!");
		*ACflg = OJ_ML;
		*topmemory = mem_lmt * STD_MB;
	}
	comp_res = execute_cmd("/bin/grep 'Could not create'  %s/error.out",
			       work_dir);
	if (!comp_res) {
		printf
		    ("jvm need more resource,tweak -Xmx(OJ_JAVA_BONUS) Settings");
		*ACflg = OJ_RE;
		//topmemory=0;
	}
	return comp_res;
}

int special_judge(char *oj_home, int problem_id, char *infile, char *outfile,
		  char *userfile)
{
	pid_t pid;
	printf("pid=%d\n", problem_id);
	pid = fork();
	int ret = 0;
	if (pid == 0) {

		while (setgid(1536) != 0)
			sleep(1);
		while (setuid(1536) != 0)
			sleep(1);
		while (setresuid(1536, 1536, 1536) != 0)
			sleep(1);

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

		ret =
		    execute_cmd("%s/data/%d/spj %s %s %s", oj_home, problem_id,
				infile, outfile, userfile);
		if (DEBUG)
			printf("spj1=%d\n", ret);
		if (ret)
			exit(1);
		else
			exit(0);
	} else {
		int status;

		waitpid(pid, &status, 0);
		ret = WEXITSTATUS(status);
		if (DEBUG)
			printf("spj2=%d\n", ret);
	}
	return ret;

}

void judge_solution(int *ACflg, int usedtime, int time_lmt, int isspj,
		    int p_id, char *infile, char *outfile, char *userfile,
		    int *PEflg, int lang, char *work_dir, int *topmemory,
		    int mem_lmt, int solution_id, double num_of_test)
{
	//usedtime-=1000;
	int comp_res;
	if (!oi_mode)
		num_of_test = 1.0;
	if (*ACflg == OJ_AC
	    && usedtime > time_lmt * 1000 * (use_max_time ? 1 : num_of_test))
		*ACflg = OJ_TL;
	if (*topmemory > mem_lmt * STD_MB)
		*ACflg = OJ_ML;	//issues79
	// compare
	if (*ACflg == OJ_AC) {
		if (isspj) {
			comp_res =
			    special_judge(oj_home, p_id, infile, outfile,
					  userfile);

			if (comp_res == 0)
				comp_res = OJ_AC;
			else {
				if (DEBUG)
					printf("fail test %s\n", infile);
				comp_res = OJ_WA;
			}
		} else {
			comp_res = compare(outfile, userfile);
		}
		if (comp_res == OJ_WA) {
			*ACflg = OJ_WA;
			if (DEBUG)
				printf("fail test %s\n", infile);
		} else if (comp_res == OJ_PE)
			*PEflg = OJ_PE;
		*ACflg = comp_res;
	}
	//jvm popup messages, if don't consider them will get miss-WrongAnswer
	if (lang == 3) {
		comp_res =
		    fix_java_mis_judge(work_dir, ACflg, topmemory, mem_lmt);
	}
}

int get_page_fault_mem(struct rusage ruse, pid_t pidApp)
{
	//java use pagefault
	int m_vmpeak, m_vmdata, m_minflt;
	m_minflt = ruse.ru_minflt * getpagesize();
	if (0 && DEBUG) {
		m_vmpeak = get_proc_status(pidApp, "VmPeak:");
		m_vmdata = get_proc_status(pidApp, "VmData:");
		printf("VmPeak:%d KB VmData:%d KB minflt:%d KB\n", m_vmpeak,
		       m_vmdata, m_minflt >> 10);
	}
	return m_minflt;
}

void print_runtimeerror(char *err)
{
	FILE *ferr = fopen("error.out", "a+");
	fprintf(ferr, "Runtime Error:%s\n", err);
	fclose(ferr);
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

int get_sim(int solution_id, int lang, int pid)
{
	int sim_s_id;
	char src_pth[BUFSIZE];
	//char cmd[BUFSIZE];
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
					update_solution(solution_id, OJ_RI,
							0, 0, sim, sim_s_id,
							0.0);
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

	conn = prepare_mysql();

	//set work directory to start running & judging
	sprintf(work_dir, "%s/run%d/", oj_home, runner_id);

	//如果使用了/dev/shm的共享内存虚拟磁盘来运行答案
	//启用能提高判题速度，但需要较多内存。
	if (shm_run) {
		mk_shm_workdir();
	}

	chdir(work_dir);

	if (!DEBUG) {
		clean_workdir();
	}

	// 获取solution的各种信息
	solution = get_solution(solution_id);

	//将提交的源代码存放在work_dir的Main.*文件中
	save_solution_src();

	solution->result = OJ_AC;

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
		write_log("solution %d running.\n");
		solution->result = OJ_RI;
		update_solution();
	}

	// test run
	if (p_id == 0) {
		test_run();
		free(solution);
		cleanup_mysql();
		exit(EXIT_SUCCESS);
	}

	// virtual judge
	if (solution->problem_info.problem_id >= 0) {
		vjudge();
		cleanup_mysql();
		free(solution);
		exit(EXIT_SUCCESS);
	}

	// run
	int p_id = 0;
	int time_lmt = 1;
	int mem_lmt  = 128;
	int lang = 0;
	int isspj = 0;
	int max_case_time = 0;
	char user_id[BUFSIZE];
	char fullpath[BUFSIZE];
	char infile[BUFSIZE];
	char outfile[BUFSIZE];
	char userfile[BUFSIZE];
	int ACflg = OJ_AC;
	int PEflg = OJ_AC;
	int usedtime = 0, topmemory = 0;
	double pass_rate = 0.0;
	int num_of_test = 0;
	int finalACflg = ACflg;

	// the fullpath of data dir
	sprintf(fullpath, "%s/data/%d", oj_home, solution->problem_info.problem_id);

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

	while ((oi_mode || ACflg == OJ_AC) && (dirp = readdir(dp)) != NULL) {
		// check if the file is *.in or not
		int namelen = isinfile(dirp->d_name);
		if (namelen == 0) {
			continue;
		}

		prepare_files(dirp->d_name, namelen, infile, outfile,
				userfile, runner_id);
		init_syscalls_limits(lang);

		pid_t pid = fork();
		if (pidApp == 0) {
			run_solution(lang, work_dir, time_lmt, usedtime,
				     mem_lmt);
		} else {	//父进程等待子进程判题结束,获取结果
			num_of_test++;
			watch_solution(pidApp, infile, &ACflg, isspj, userfile,
				       outfile, solution_id, lang, &topmemory,
				       mem_lmt, &usedtime, time_lmt, p_id, PEflg,
				       work_dir);
			judge_solution(&ACflg, usedtime, time_lmt, isspj, p_id,
				       infile, outfile, userfile, &PEflg, lang,
				       work_dir, &topmemory, mem_lmt,
				       solution_id, num_of_test);
			if (use_max_time) {
				max_case_time =
				    usedtime >
				    max_case_time ? usedtime : max_case_time;
				usedtime = 0;
			}
			//clean_session(pidApp);
		}
		if (oi_mode) {
			if (ACflg == OJ_AC) {
				++pass_rate;
			}
			if (finalACflg < ACflg) {
				finalACflg = ACflg;
			}

			ACflg = OJ_AC;
		}
	}

	if (ACflg == OJ_AC && PEflg == OJ_PE)
		ACflg = OJ_PE;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char sql[BUFSIZE];
	//这里将查询语句写错了，导致不能判题
	sprintf(sql,
		"select ischa from solution,cha where cha.problem_id=solution.problem_id "
		"and solution_id=%d", solution_id);
	int ischa = 0;
	int errnum = 0;
	if ((errnum = mysql_real_query(conn, sql, strlen(sql))) == 0) {
		res = mysql_store_result(conn);
		row = mysql_fetch_row(res);
		if (row != NULL) {
			ischa = row[0][0] - '0';
		}
	} else {
		write_log("mysql_real_query error = %d", errnum);
		write_log("mysql_error = %s", mysql_error(conn));
		write_log("sql = %s", sql);
	}
	if (DEBUG) {
		write_log("ischa = %d", ischa);
	}
	if (ischa && sim_enable && ACflg == OJ_AC
	    && (!oi_mode || finalACflg == OJ_AC)
	    && lang < 5) {	//bash don't supported
		get_sim(solution_id, lang, p_id);
	}

	//write_log("solution_id = %d", solution_id);
	//write_log("sim_s_id = %d", sim_s_id);
	//write_log("sim = %d", sim);
	//if(ACflg == OJ_RE)addreinfo(solution_id, "error.out");

	if ((oi_mode && finalACflg == OJ_RE) || ACflg == OJ_RE) {
		if (DEBUG)
			printf("add RE info of %d..... \n", solution_id);
		addreinfo(solution_id, "error.out");
	}
	if (use_max_time) {
		usedtime = max_case_time;
	}
	if (ACflg == OJ_TL) {
		usedtime = time_lmt * 1000;
	}
	if (oi_mode) {
		if (num_of_test > 0)
			pass_rate /= num_of_test;
		update_solution(solution_id, finalACflg, usedtime,
				topmemory >> 10, 0, 0, pass_rate);
	} else {
		//write_log("ACflg = %d\n", ACflg);
		//前面已经更新过sim了
		update_solution(solution_id, ACflg, usedtime, topmemory >> 10,
				0, 0, 0);
	}
	if ((oi_mode && finalACflg == OJ_WA) || ACflg == OJ_WA) {
		if (DEBUG)
			printf("add diff info of %d..... \n", solution_id);
		if (!isspj)
			adddiffinfo(solution_id);
	}
	update_user(user_id);
	update_problem(p_id);
	clean_workdir();

	if (DEBUG)
		write_log("result=%d", oi_mode ? finalACflg : ACflg);
	mysql_close(conn);
	if (record_call) {
		print_call_array();
	}
	closedir(dp);
	return 0;
}
