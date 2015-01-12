#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>
#include <unistd.h>
#include <mysql/mysql.h>

#include "judge_client.h"

int login_hduoj(void)
{
	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_URL, "http://acm.hdu.edu.cn/userloginex.php?action=login");
	// 设置参数
	char post_str[BUFSIZE];
	char filename[BUFSIZE];
	char *html = (char *)malloc(BUFSIZE * BUFSIZE);
	if (html == NULL) {
		write_log("alloc memory error.\n");
		return -1;
	}
	sprintf(filename, "%dlogin.txt", solution->solution_id);
	sprintf(post_str, "username=%s&userpass=%s", vjudge_user, vjudge_passwd);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_str);
	perform_curl(filename);
	load_file(filename, html);

	// modified form bnuoj
	if (strstr(html, "No such user or wrong password.") != NULL
		|| strstr(html, "<b>One or more following ERROR(s) occurred.") != NULL
		|| strstr(html, "<h2>The requested URL could not be retrieved</h2>") != NULL
		|| strstr(html, "<H1 style=\"COLOR: #1A5CC8\" align=center>Sign In Your Account</H1>") != NULL
		|| strstr(html, "PHP: Maximum execution time of") != NULL) {
		free(html);
		return -1;
	}
	free(html);
	return 0;
}

int submit_hduoj()
{
	char *post_str = (char *)malloc(BUFSIZE * BUFSIZE);
	if (post_str == NULL) {
		write_log("alloc memory error.\n");
		return -1;
	}

	memset(post_str, 0, BUFSIZE * BUFSIZE);
	sprintf(post_str, "check=0&problemid=%d&language=%d&usercode=",
			solution->problem_info.origin_id,
			lang_table[solution->problem_info.ojtype][solution->language]);
	// 对其进行url编码
	utf2gbk(solution->src, solution->code_length);
	char *str = url_encode(solution->src);
	if (str == NULL) {
		write_log("encode url error.\n");
		free(str);
		free(post_str);
		return -1;
	}
	strcat(post_str, str);

	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_URL, "http://acm.hdu.edu.cn/submit.php?action=submit");
	// 设置参数
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_str);

	char filename[BUFSIZE];
	sprintf(filename, "%dsubmit.txt", solution->solution_id);
	perform_curl(filename);

	char *html = (char *)malloc(BUFSIZE * BUFSIZE);
	if (html == NULL) {
		write_log("alloc memory error.\n");
		free(str);
		free(post_str);
		return -1;
	}
	load_file(filename, html);

	// modified form bnuoj
	if (strstr(html, "Connect(0) to MySQL Server failed.") != NULL
		|| strstr(html, "<b>One or more following ERROR(s) occurred.") != NULL
		|| strstr(html, "<h2>The requested URL could not be retrieved</h2>") != NULL
		|| strstr(html, "<H1 style=\"COLOR: #1A5CC8\" align=center>Sign In Your Account</H1>") != NULL
		|| strstr(html, "PHP: Maximum execution time of") != NULL
		|| strstr(html, "<DIV>Exercise Is Closed Now!</DIV>") != NULL) {
		free(str);
		free(html);
		free(post_str);
		return -1;
	}

	free(str);
	free(html);
	free(post_str);

	return 0;
}

int get_reinfo_hduoj(void)
{
	return 0;
}

int get_ceinfo_hduoj(void)
{
	return 0;
}
int convert_result_hduoj(char *buf)
{
	if (strcmp("Accepted", buf) == 0) {
		return OJ_AC;
	} else if (strcmp("Presentation Error", buf) == 0) {
		return OJ_PE;
	} else if (strcmp("Runtime Error", buf) == 0) {
		return OJ_RE;
	} else if (strcmp("Wrong Answer", buf) == 0) {
		return OJ_WA;
	} else if (strcmp("Time Limit Exceeded", buf) == 0) {
		return OJ_TL;
	} else if (strcmp("Memory Limit Exceeded", buf) == 0) {
		return OJ_ML;
	} else if (strcmp("Output Limit Exceeded", buf) == 0) {
		return OJ_OL;
	} else if (strcmp("Compilation Error", buf) == 0) {
		return OJ_CE;
	} else {
		return OJ_JE;
	}

	return OJ_JE;
}

int get_status_hduoj(void)
{
	int status, i, j;
	regmatch_t pmatch[5];
	const int nmatch = 5;
	regex_t reg;
	const char *pattern = "<td height=22px>([0-9]*)</td>"
		"<td>[: 0-9-]*</td>"
		"<td><font color=[ a-zA-Z]*>([ a-zA-Z]*)</font></td>"
		"<td><a[ 0-9a-zA-Z\\./\\?\\\"=]*>[0-9]*</a></td>"
		"<td>([0-9]*)MS</td>"
		"<td>([0-9]*)K</td>"
		;
	char url[BUFSIZE];
	sprintf(url, "http://acm.hdu.edu.cn/status.php?first=&pid=%d&user=%s"
			"&lang=0&status=0", solution->problem_info.origin_id,
			vjudge_user);

	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_URL, url);

	char filename[BUFSIZE];
	sprintf(filename, "%dstatus.txt", solution->solution_id);

	time_t begin_time = time(NULL);
	int rid = 0;
	int usedtime = 0;
	int memory = 0;
	char result[BUFSIZE];
	char err[BUFSIZE];
	char match_str[BUFSIZE];
	int ret = regcomp(&reg, pattern, REG_EXTENDED);
	if (ret) {
		regerror(ret, &reg, err, BUFSIZE);
		write_log("compile regex error: %s.\n", err);
		return OJ_JE;
	}
	char *html = (char *)malloc(BUFSIZE * BUFSIZE);
	if (html == NULL) {
		write_log("alloc memory error.\n");
		return OJ_JE;
	}
	while (1) {
		if (time(NULL) - begin_time > vj_max_wait_time) {
			write_log("judge time out.\n");
			free(html);
			return OJ_JE;
		}
		perform_curl(filename);

		load_file(filename, html);
		gbk2utf8(html, strlen(html));
		// modified form bnuoj
		if (strstr(html, "Connect(0) to MySQL Server failed.") != NULL
			|| strstr(html, "<b>One or more following ERROR(s) occurred.") != NULL
			|| strstr(html, "<h2>The requested URL could not be retrieved</h2>") != NULL
			|| strstr(html, "<H1 style=\"COLOR: #1A5CC8\" align=center>Sign In Your Account</H1>") != NULL
			|| strstr(html, "PHP: Maximum execution time of") != NULL
			|| strstr(html, "<DIV>Exercise Is Closed Now!</DIV>") != NULL) {
			write_log("get solution %d status error.\n", solution->solution_id);
			free(html);
			return OJ_JE;
		} else {
			status = regexec(&reg, html, nmatch, pmatch, 0);
			if (status == REG_NOMATCH) {
				write_log("get solution %d status error.\n", solution->solution_id);
				free(html);
				return OJ_WT0;
			} else if (status == 0) {
				char buf[BUFSIZE];
				for (i = 0; i < nmatch; ++i) {
					int cnt = 0;
					for (j = pmatch[i].rm_so; j < pmatch[i].rm_eo; ++j) {
						buf[cnt++] = html[j];
					}
					buf[cnt] = '\0';
					switch (i) {
						case 0: strcpy(match_str, buf); break;
						case 1: rid = atoi(buf); break;
						case 2: strcpy(result, buf); break;
						case 3: usedtime = atoi(buf); break;
						case 4: memory = atoi(buf); break;
					}
				}
				write_log("match_str = %s\n", match_str);
				write_log("rid = %d\n", rid);
				write_log("usedtime = %d\n", usedtime);
				write_log("memory = %d\n", memory);
				write_log("result = %s\n", result);
				if (is_final_result(result)) {
					solution->time = rid;
					solution->time = usedtime;
					solution->memory = memory;
					free(html);
					return convert_result(result);
				}
			}
		}
	}

	free(html);

	return OJ_JE;
}
