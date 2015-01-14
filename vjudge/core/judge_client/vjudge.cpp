#include "judge_client.h"

/**
 * copy from bunoj
 * Check whether the result is final
 * @param result        Current result
 * @return Is final one or not
 */
int is_final_result(char *buf)
{
	char result[BUFSIZE];
	strcpy(result, buf);
	trim(result);
	to_lowercase(result);

	write_log("test result %s is or not final result.\n", result);
	// Minimum length result is "Accept"
	int ret = 1;
	if (strlen(result) < 6) {
		ret = 0;
	}
	if (strstr(result, "waiting") != NULL) {
		ret = 0;
	}
	if (strstr(result, "running") != NULL) {
		ret = 0;
	}
	if (strstr(result, "judging") != NULL) {
		ret = 0;
	}
	if (strstr(result, "presentation") == NULL
	    && strstr(result, "sent") != NULL) {
		ret = 0;
	}
	if (strstr(result, "queu") != NULL) {
		ret = 0;
	}
	if (strstr(result, "compiling") != NULL) {
		ret = 0;
	}
	if (strstr(result, "linking") != NULL) {
		ret = 0;
	}
	if (strstr(result, "received") != NULL) {
		ret = 0;
	}
	if (strstr(result, "pending") != NULL) {
		ret = 0;
	}
	if (strstr(result, "not judged yet") != NULL) {
		ret = 0;
	}
	if (strstr(result, "being judged") != NULL) {
		ret = 0;
	}
	if (strstr(result, "testing") != NULL) {
		ret = 0;
	}

	if (ret) {
		write_log("test result %s is final result.\n", result);
	} else {
		write_log("test result %s is not final result.\n", result);
	}
	return ret;
}

int convert_result(char *buf)
{
	write_log("try to convert %d oj's result %s.\n", solution->problem_info.ojtype, buf);
	int ret = OJ_JE;
	switch (solution->problem_info.ojtype) {
		case 0: ret = convert_result_hduoj(buf); break;
		case 1: ret = convert_result_poj(buf); break;
		case 2: ret = convert_result_cf(buf); break;
	}
	return ret;
}

int login(void)
{
	write_log("try to log in %d.\n", solution->problem_info.ojtype);
	int ret = -1;
	switch (solution->problem_info.ojtype) {
		case 0: ret = login_hduoj(); break;
		case 1: ret = login_poj(); break;
		case 2: ret = login_cf(); break;
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
		case 1: ret = submit_poj(); break;
		case 2: ret = submit_cf(); break;
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
	int ret = OJ_JE;
	switch (solution->problem_info.ojtype) {
		case 0: ret = get_status_hduoj(); break;
		case 1: ret = get_status_poj(); break;
		case 2: ret = get_status_cf(); break;
	}
	return ret;
}

int get_ceinfo(void)
{
	write_log("get solution %d compile error info\n", solution->solution_id);
	int ret = -1;
	switch (solution->problem_info.ojtype) {
		case 0: ret = get_ceinfo_hduoj(); break;
		case 1: ret = get_ceinfo_poj(); break;
		case 2: ret = get_ceinfo_cf(); break;
	}
	return ret;
}

int get_reinfo(void)
{
	write_log("get solution %d runtime error info.\n", solution->solution_id);
	int ret = -1;
	switch (solution->problem_info.ojtype) {
		case 0: ret = get_reinfo_hduoj(); break;
		case 1: ret = get_reinfo_poj(); break;
		case 2: ret = get_reinfo_poj(); break;
	}
	return ret;
}

int vjudge(void)
{
	curl = prepare_curl();
	if (curl == NULL) {
		write_log("prepare curl handle error, try again %d seconds later.\n", sleep_time);
		sleep(sleep_time);
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
		if (!islogin) {
			write_log("not login.");
			solution->result = OJ_JE;
			return -1;
		}
		ret = submit();
		if (ret < 0) {
			write_log("submit solution %d failed. assume sleep for a while, sleep %d seconds.\n",
					solution->solution_id, sleep_time);
			sleep(sleep_time);
			ret = submit();
		}
	}
	if (ret == 0) {
		solution->result = OJ_RI;
		update_solution();
	} else {
		solution->result = OJ_JE;
		return -1;
	}

	// wait for judge
	sleep(sleep_time);
	solution->result = get_status();
	if (solution->result == OJ_TL) {
		solution->time = solution->problem_info.time_limit * 1000;
	}
	if (solution->result == OJ_CE) {
		solution->isce = 1;
		get_ceinfo();
	} else if (solution->result == OJ_RE) {
		solution->isre = 1;
		get_reinfo();
	}
	int solution_id = solution->solution_id;
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
		case OJ_JE: write_log("solution %d judge error.\n", solution_id); break;
		default: write_log("solution %d result is %d.\n", solution_id, solution->result);
	}

	cleanup_curl();

	return 0;
}
