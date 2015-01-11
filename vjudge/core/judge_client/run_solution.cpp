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

void run_solution(void)
{
	write_log("run solution %d.\n", solution->solution_id);
	//将优先级调成19(最低级)
	//优先级数为-20到19,数字越小优先级越高
	nice(19);
	// now the user is "judger"
	chdir(work_dir);
	// open the files
	freopen("data.in", "r", stdin);
	freopen("user.out", "w", stdout);
	freopen("error.out", "a+", stderr);

	// trace me
	// 本进程被其父进程所跟踪。
	ptrace(PTRACE_TRACEME, 0, NULL, NULL);

	// run me
	if (solution->language != 3) {
		chroot(work_dir);
	}
	while (setgid(1536) != 0) {
		sleep(1);
	}
	while (setuid(1536) != 0) {
		sleep(1);
	}
	while (setresuid(1536, 1536, 1536) != 0) {
		sleep(1);
	}

	// child
	// set the limit
	struct rlimit LIM;	// time limit, file limit& memory limit
	// time limit
	if (oi_mode) {		//oi模式
		LIM.rlim_cur = solution->problem_info.time_limit + 1;
	} else {
		LIM.rlim_cur = (solution->problem_info.time_limit - solution->time / 1000) + 1;
	}
	LIM.rlim_max = LIM.rlim_cur;
	setrlimit(RLIMIT_CPU, &LIM);
	alarm(0);
	alarm(solution->problem_info.time_limit * 10);

	// file limit
	LIM.rlim_max = STD_F_LIM + STD_MB;
	LIM.rlim_cur = STD_F_LIM;
	setrlimit(RLIMIT_FSIZE, &LIM);

	// proc limit
	switch (solution->language) {
		// java
		case 3:
		case 12: LIM.rlim_cur = LIM.rlim_max = 50; break;
		// bash
		case 5: LIM.rlim_cur = LIM.rlim_max = 3; break;
		// C#
		case 9: LIM.rlim_cur = LIM.rlim_max = 50; break;
		default: LIM.rlim_cur = LIM.rlim_max = 1; break;
	}
	setrlimit(RLIMIT_NPROC, &LIM);

	// set the stack
	LIM.rlim_cur = STD_MB << 6;
	LIM.rlim_max = STD_MB << 6;
	setrlimit(RLIMIT_STACK, &LIM);

	// set the memory
	LIM.rlim_cur = STD_MB * solution->problem_info.memory_limit / 2 * 3;
	LIM.rlim_max = STD_MB * solution->problem_info.memory_limit * 2;
	if (solution->language < 3) {
		setrlimit(RLIMIT_AS, &LIM);
	}

	switch (solution->language) {
		case 0:
		case 1:
		case 2:
		case 10:
		case 11:
		case 13:
		case 14: execl("./Main", "./Main", (char *)NULL); break;
		case 3: execl("/usr/bin/java", "/usr/bin/java", java_xms,
					java_xmx, "-Djava.security.manager",
					"-Djava.security.policy=./java.policy",
					"Main", (char *)NULL); break;
		case 4: execl("/ruby", "/ruby", "Main.rb", (char *)NULL); break;
		// bash
		case 5: execl("/bin/bash", "/bin/bash", "Main.sh", (char *)NULL); break;
		// python
		case 6: execl("/python", "/python", "Main.py", (char *)NULL); break;
		// php
		case 7: execl("/php", "/php", "Main.php", (char *)NULL); break;
		// perl
		case 8: execl("/perl", "/perl", "Main.pl", (char *)NULL); break;
		// mono C#
		case 9: execl("/mono", "/mono", "--debug", "Main.exe", (char *)NULL); break;
		// guile
		case 12: execl("/guile", "/guile", "Main.scm", (char *)NULL); break;
	}
	exit(EXIT_SUCCESS);
}
