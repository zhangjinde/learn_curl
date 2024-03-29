#include "judge_client.h"

int special_judge(char *infile, char *outfile, char *userfile)
{
	write_log("start special judge solution %d.\n", solution->solution_id);
	pid_t pid;
	pid = fork();
	int ret = 0;
	if (pid < 0) {
		write_log("special_judge fork error.\n");
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
