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

#include "judge_client.h"

/**
 * copy from bunoj
 * Check whether the result is final
 * @param result        Current result
 * @return Is final one or not
 */
int is_final_result(char *result)
{
	trim(result);
	to_lowercase(result);

	// Minimum length result is "Accept"
	if (strlen(result) < 6) {
		return 0;
	}
	if (strstr(result, "waiting") != NULL) {
		return 0;
	}
	if (strstr(result, "running") != NULL) {
		return 0;
	}
	if (strstr(result, "judging") != NULL) {
		return 0;
	}
	if (strstr(result, "presentation") == NULL
	    && strstr(result, "sent") != NULL) {
		return 0;
	}
	if (strstr(result, "queu") != NULL) {
		return 0;
	}
	if (strstr(result, "compiling") != NULL) {
		return 0;
	}
	if (strstr(result, "linking") != NULL) {
		return 0;
	}
	if (strstr(result, "received") != NULL) {
		return false;
	}
	if (strstr(result, "pending") != NULL) {
		return false;
	}
	if (strstr(result, "not judged yet") != NULL) {
		return false;
	}
	if (strstr(result, "being judged") != NULL) {
		return 0;
	}

	return 1;
}

int login(void)
{
	write_log("try to log in %d.\n", solution->problem_info.ojtype);
	int ret = -1;
	switch (solution->problem_info.ojtype) {
		case 0: ret = login_hduoj(); break;
		default: write_log("unsupported virtual judge.\n"); break;
	}
	if (ret < 0) {
		write_log("login error.\n");
	} else {
		islogin = 1;
		write_log("logged in.\n");
	}
	return ret;
}

int submit(void)
{
	write_log("try to submit solution %d.\n", solution->solution_id);
	int ret = -1;
	switch (solution->problem_info.ojtype) {
		case 0: ret = submit_hduoj(); break;
		default: write_log("unsupported virtual judge.\n"); break;
	}
	if (ret < 0) {
		write_log("solution %d submit error.\n", solution->solution_id);
	} else {
		write_log("solution %d submited\n", solution->solution_id);
	}
	return ret;
}

int get_status(void)
{
	write_log("get solution %d status.\n", solution->solution_id);
	int ret = OJ_WT0;
	switch (solution->problem_info.ojtype) {
		case 0: ret = get_status_hduoj(); break;
		default: write_log("unsupported virtual judge.\n"); break;
	}
	return ret;
}

int vjudge(void)
{
	curl = prepare_curl();
	if (curl == NULL) {
		write_log("prepare curl handle error, try again 5 seconds later.\n");
		sleep(5);
		curl = prepare_curl();
		if (curl == NULL) {
			write_log("prepare curl handle error.\n");
			solution->result = 0;
			return -1;
		}
	}

	if (!islogin) {
		clear_cookie();
		login();
	}

	int ret = submit();
	if (ret < 0) {
		write_log("submit solution %d failed. assume not logged in.\n",
				solution->solution_id);
		clear_cookie();
		islogin = 0;
		login();
		ret = submit();
		if (ret < 0) {
			write_log("submit solution %d failed. assume sleep for a while, sleep 10 seconds.\n",
					solution->solution_id);
			sleep(10);
			ret = submit();
		}
	}
	if (ret == 0) {
		write_log("solution %d submited.\n", solution->solution_id);
	}

	solution->result = get_status();

	cleanup_curl();

	return 0;
}
