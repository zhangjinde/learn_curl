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

int get_page_fault_mem(struct rusage ruse, pid_t pid)
{
	//java use pagefault
	int m_vmpeak, m_vmdata, m_minflt;
	m_minflt = ruse.ru_minflt * getpagesize();
	if (0 && DEBUG) {
		m_vmpeak = get_proc_status(pid, "VmPeak:");
		m_vmdata = get_proc_status(pid, "VmData:");
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

void watch_solution(pid_t pid, char *outfile, char *userfile)
{
	int lang = solution->language;
	int tempmemory;
	int status, sig, exitcode;
	struct user_regs_struct reg;
	struct rusage ruse;
	write_log("process %d watch solution %d.\n", pid, solution->solution_id);
	while (1) {
		// check the usage
		wait4(pid, &status, 0, &ruse);

		//jvm gc ask VM before need,so used kernel page fault times and page size
		if (lang == 3) {
			tempmemory = get_page_fault_mem(ruse, pid);
		} else {	//other use VmPeak
			tempmemory = get_proc_status(pid, "VmPeak:") << 10;
		}
		if (tempmemory > solution->memory) {
			solution->memory = tempmemory;
		}
		if (solution->memory > solution->problem_info.memory_limit * STD_MB) {
			if (solution->result == OJ_AC) {
				solution->result = OJ_ML;
			}
			ptrace(PTRACE_KILL, pid, NULL, NULL);
			break;
		}

		//如果子进程正常返回,则退出
		if (WIFEXITED(status)) {
			break;
		}

		//出现了错误RE
		if ((lang < 4 || lang == 9) && get_file_size("error.out")
				&& !oi_mode) {
			solution->result = OJ_RE;
			ptrace(PTRACE_KILL, pid, NULL, NULL);
			break;
		}

		if (!solution->problem_info.spj && get_file_size(userfile) > get_file_size(outfile) * 2 + 1024) {
			solution->result = OJ_OL;
			ptrace(PTRACE_KILL, pid, NULL, NULL);
			break;
		}

		//返回子进程的返回状态
		exitcode = WEXITSTATUS(status);
		/*
		 * exitcode == 5 waiting for next CPU allocation
		 * ruby using system to run,exit 17 ok
		 */
		if ((lang >= 3 && exitcode == 17) || exitcode == 0x05
				|| exitcode == 0) {
			//go on and on
			;
		} else {
			if (solution->result == OJ_AC) {
				switch (exitcode) {
					case SIGCHLD:
					case SIGALRM: alarm(0);
					case SIGKILL:
					case SIGXCPU: solution->result = OJ_TL; break;
					case SIGXFSZ: solution->result = OJ_OL; break;
					default: solution->result = OJ_RE; break;
				}
				print_runtimeerror(strsignal(exitcode));
			}
			ptrace(PTRACE_KILL, pid, NULL, NULL);
			break;
		}

		if (WIFSIGNALED(status)) {
			/* WIFSIGNALED: if the process is terminated by signal
			 * psignal(int sig, char *s)，like perror(char *s)，
			 * print out s, with error msg from system of sig  
			 * sig = 5 means Trace/breakpoint trap
			 * sig = 11 means Segmentation fault
			 * sig = 25 means File size limit exceeded
			 */
			sig = WTERMSIG(status);
			if (DEBUG) {
				printf("WTERMSIG=%d\n", sig);
				psignal(sig, NULL);
			}
			if (solution->result == OJ_AC) {
				switch (sig) {
					case SIGCHLD:
					case SIGALRM: alarm(0);
					case SIGKILL:
					case SIGXCPU: solution->result = OJ_TL; break;
					case SIGXFSZ: solution->result = OJ_OL; break;
					default: solution->result = OJ_RE; break;
				}
				print_runtimeerror(strsignal(sig));
			}
			break;
		}

		/* comment from http://www.felix021.com/blog/read.php?1662
		 * WIFSTOPPED: return true if the process is paused or stopped
		 * while ptrace is watching on it
		 * WSTOPSIG: get the signal if it was stopped by signal
		 */

		// check the system calls
		ptrace(PTRACE_GETREGS, pid, NULL, &reg);
		if (call_counter[reg.REG_SYSCALL]) {
			//call_counter[reg.REG_SYSCALL]--;
		} else if (record_call) {
			call_counter[reg.REG_SYSCALL] = 1;
		} else {	//do not limit JVM syscall for using different JVM
			solution->result = OJ_RE;
			char error[BUFSIZE];
			sprintf(error, "[ERROR] A Not allowed system call: "
					"runid:%d callid:%ld\n"
					"TO FIX THIS , ask admin to add the "
					"CALLID into corresponding LANG_XXV[] "
					"located at okcalls32/64.h ,and "
					"recompile judge_client",
					solution->solution_id,
					(long)reg.REG_SYSCALL);
			print_runtimeerror(error);
			ptrace(PTRACE_KILL, pid, NULL, NULL);
		}
		ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
	}
	solution->time += (ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000);
	solution->time += (ruse.ru_stime.tv_sec * 1000 + ruse.ru_stime.tv_usec / 1000);
}
