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

int compile(void)
{
	int lang = solution->language;
	const char *CP_C[] = {"gcc", "Main.c", "-o", "Main", "-fno-asm",
		"-Wall", "-lm", "--static", "-std=c99", "-DONLINE_JUDGE",
		NULL };
	const char *CP_X[] = {"g++", "Main.cc", "-o", "Main", "-fno-asm",
		"-Wall", "-lm", "--static", "-std=c++0x", "-DONLINE_JUDGE",
		NULL};
	const char *CP_P[] = {"fpc", "Main.pas", "-O2", "-Co", "-Ct", "-Ci",
		NULL };
	const char *CP_R[] = {"ruby", "-c", "Main.rb", NULL};
	const char *CP_B[] = {"chmod", "+rx", "Main.sh", NULL};
	const char *CP_Y[] = {"python", "-c",
		"import py_compile; py_compile.compile(r'Main.py')", NULL};
	const char *CP_PH[] = {"php", "-l", "Main.php", NULL};
	const char *CP_PL[] = {"perl", "-c", "Main.pl", NULL};
	const char *CP_CS[] = {"gmcs", "-warn:0", "Main.cs", NULL};
	const char *CP_OC[] = {"gcc", "-o", "Main", "Main.m",
		"-fconstant-string-class=NSConstantString", "-I",
		"/usr/include/GNUstep/", "-L", "/usr/lib/GNUstep/Libraries/",
		"-lobjc", "-lgnustep-base", NULL};
	const char *CP_BS[] = {"fbc", "Main.bas", NULL};
	const char *CP_CLANG[] = {"clang", "Main.c", "-o", "Main", "-fno-asm",
		"-Wall", "-lm", "--static", "-std=c99", "-DONLINE_JUDGE",
		NULL};
	const char *CP_CLANG_CPP[] = {"clang++", "Main.cc", "-o", "Main",
		"-fno-asm", "-Wall", "-lm", "--static", "-std=c++0x",
		"-DONLINE_JUDGE", NULL};
	char javac_buf[7][16];
	char *CP_J[7];
	int i = 0;
	for (i = 0; i < 7; i++) {
		CP_J[i] = javac_buf[i];
	}
	sprintf(CP_J[0], "javac");
	sprintf(CP_J[1], "-J%s", java_xms);
	sprintf(CP_J[2], "-J%s", java_xmx);
	sprintf(CP_J[3], "-encoding");
	sprintf(CP_J[4], "UTF-8");
	sprintf(CP_J[5], "Main.java");
	CP_J[6] = (char *)NULL;

	write_log("compile solution %d.\n", solution->solution_id);
	pid_t pid = fork();
	if (pid < 0) {
		write_log("fork error:%s.\n", strerror(errno));
	} else if (pid == 0) {
		struct rlimit LIM;
		LIM.rlim_max = 60;
		LIM.rlim_cur = 60;
		setrlimit(RLIMIT_CPU, &LIM);
		alarm(60);
		LIM.rlim_max = 100 * STD_MB;
		LIM.rlim_cur = 100 * STD_MB;
		setrlimit(RLIMIT_FSIZE, &LIM);

		if (lang == 3) {
			LIM.rlim_max = STD_MB << 11;
			LIM.rlim_cur = STD_MB << 11;
		} else {
			LIM.rlim_max = STD_MB << 10;
			LIM.rlim_cur = STD_MB << 10;
		}
		setrlimit(RLIMIT_AS, &LIM);
		if (lang != 2 && lang != 11) {
			freopen("ce.txt", "w", stderr);
		} else {
			freopen("ce.txt", "w", stdout);
		}
		execute_cmd("chown judge *");
		while (setgid(1536) != 0) {
			sleep(1);
		}
		while (setuid(1536) != 0) {
			sleep(1);
		}
		while (setresuid(1536, 1536, 1536) != 0) {
			sleep(1);
		}

		switch (lang) {
			case 0: execvp(CP_C[0], (char *const *)CP_C); break;
			case 1: execvp(CP_X[0], (char *const *)CP_X); break;
			case 2: execvp(CP_P[0], (char *const *)CP_P); break;
			case 3: execvp(CP_J[0], (char *const *)CP_J); break;
			case 4: execvp(CP_R[0], (char *const *)CP_R); break;
			case 5: execvp(CP_B[0], (char *const *)CP_B); break;
			case 6: execvp(CP_Y[0], (char *const *)CP_Y); break;
			case 7: execvp(CP_PH[0], (char *const *)CP_PH); break;
			case 8: execvp(CP_PL[0], (char *const *)CP_PL); break;
			case 9: execvp(CP_CS[0], (char *const *)CP_CS); break;
			case 10: execvp(CP_OC[0], (char *const *)CP_OC); break;
			case 11: execvp(CP_BS[0], (char *const *)CP_BS); break;
			case 13: execvp(CP_CLANG[0], (char *const *)CP_CLANG); break;
			case 14: execvp(CP_CLANG_CPP[0], (char *const *)CP_CLANG_CPP); break;
			default: printf("nothing to do!\n"); break;
		}
		exit(EXIT_SUCCESS);
	} else {
		int status = 0;
		waitpid(pid, &status, 0);
		if (lang > 3 && lang < 7) {
			status = get_file_size("ce.txt");
		}
		if (DEBUG) {
			write_log("status = %d.\n", status);
		}
		return status;
	}
	return 0;
}
