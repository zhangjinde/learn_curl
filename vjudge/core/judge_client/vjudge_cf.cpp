#include "judge_client.h"

// modified from bnuoj
int get_csrf(const char *url, char *csrf)
{
	char err[BUFSIZE];
	char filename[] = "csrf";
	char *html = (char *)malloc(BUFSIZE * BUFSIZE);
	if (html == NULL) {
		write_log("alloc get_csrf html buf memory error.\n");
		return -1;
	}

	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_URL, url);

	if (DEBUG) {
		write_log("perform url is %s.\n", url);
	}

	perform_curl(filename);
	load_file(filename, html);

	if (strstr(html, "Codeforces is temporary unavailable") != NULL) {
		write_log("get_csrf remote server error.\n");
		free(html);
		return -1;
	}

	int status, i, j;
	regmatch_t pmatch[2];
	const int nmatch = 2;
	regex_t reg;
	const char *pattern = "<meta name=\"X-Csrf-Token\" "
		"content=\"([^/]*)\"/>";
	char match_str[BUFSIZE];
	int ret = regcomp(&reg, pattern, REG_EXTENDED);
	if (ret) {
		regerror(ret, &reg, err, BUFSIZE);
		write_log("compile regex error: %s.\n", err);
		return -1;
	}
	status = regexec(&reg, html, nmatch, pmatch, 0);
	if (status == REG_NOMATCH) {
		write_log("no match regex: %s.\n", pattern);
		write_log("get %s csrf error.\n", url);
		free(html);
		regfree(&reg);
		return -1;
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
				case 1: strcpy(csrf, buf); break;
			}
		}
		write_log("match_str = %s\n", match_str);
		write_log("csrf = %s\n", csrf);
	}

	free(html);
	regfree(&reg);
	return -1;
}

// modified from bnuoj
void calculate_tta(char tta[])
{
	int i = 0;
	int j = 0;
	int len = strlen(tta);
	for (i = 0; i < len; ++i) {
		j = (j + (i + 1) * (i + 2) * tta[i]) % 1009;
		if (i % 3 == 0) {
			++j;
		}
		if (i % 2 == 0) {
			j *= 2;
		}
		if (i > 0) {
			j -= ((int)(tta[i / 2] / 2)) * (j % 5);
		}
		while (j < 0) {
			j += 1009;
		}
		while (j >= 1009) {
			j -= 1009;
		}
	}
	sprintf(tta, "%d", j);
	write_log("tta = %s.\n", tta);
}

// modified from bnuoj
int get_tta(char tta[])
{
	char err[BUFSIZE];
	char cookies[BUFSIZE];
	load_file(cookiename, cookies);

	int status, i, j;
	regmatch_t pmatch[2];
	const int nmatch = 2;
	regex_t reg;
	const char *pattern = "39ce7\\t(.*)";
	char match_str[BUFSIZE];
	int ret = regcomp(&reg, pattern, REG_EXTENDED);
	if (ret) {
		regerror(ret, &reg, err, BUFSIZE);
		write_log("compile regex error: %s.\n", err);
		return -1;
	}
	status = regexec(&reg, cookies, nmatch, pmatch, 0);
	if (status == REG_NOMATCH) {
		write_log("no match regex: %s.\n", pattern);
		write_log("get tta error.\n");
		regfree(&reg);
		return -1;
	} else if (status == 0) {
		char buf[BUFSIZE];
		for (i = 0; i < nmatch; ++i) {
			int cnt = 0;
			for (j = pmatch[i].rm_so; j < pmatch[i].rm_eo; ++j) {
				buf[cnt++] = cookies[j];
			}
			buf[cnt] = '\0';
			switch (i) {
				case 0: strcpy(match_str, buf); break;
				case 1: strcpy(tta, buf); break;
			}
		}
		write_log("match_str = %s\n", match_str);
		write_log("tta = %s\n", tta);
	}
	calculate_tta(tta);
	return 0;
}

int login_cf(void)
{
	char post_str[BUFSIZE];
	char csrf[BUFSIZE];
	char tta[BUFSIZE];
	char filename[BUFSIZE];
	char url[] = "http://codeforces.com/enter";
	char *html = (char *)malloc(BUFSIZE * BUFSIZE);
	if (html == NULL) {
		write_log("alloc login_cf html buf memory error.\n");
		return -1;
	}

	if (get_csrf(url, csrf) < 0) {
		write_log("login_cf get csrf error.\n");
		return -1;
	}

	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_REFERER, url);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	// 设置参数
	sprintf(filename, "%dlogin.txt", solution->solution_id);
	if (get_tta(tta) < 0) {
		write_log("login_cf get tta error.\n");
		return -1;
	}
	sprintf(post_str, "action=center&handle=%s&password=%s&csrf_token=%s"
			"&_tta=%s", vjudge_user, vjudge_passwd, csrf, tta);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_str);
	if (DEBUG) {
		write_log("perform url is %s.\n", url);
		write_log("post data is %s.\n", post_str);
	}

	perform_curl(filename);
	load_file(filename, html);

	if (strstr(html, "Codeforces is temporary unavailable") != NULL
			|| strstr(html, "Invalid handle or password") != NULL) {
		write_log("login_cf remote server error.\n");
		free(html);
		return -1;
	}

	free(html);
	return 0;
}

int submit_cf(void)
{
	char url[] = "http://poj.org/submit";
	char *post_str = (char *)malloc(BUFSIZE * BUFSIZE);
	if (post_str == NULL) {
		write_log("alloc submit_cf post_str buf memory error.\n");
		return -1;
	}

	memset(post_str, 0, BUFSIZE * BUFSIZE);
	sprintf(post_str, "problem_id=%d&language=%d&source=",
			solution->problem_info.origin_id,
			lang_table[solution->problem_info.ojtype][solution->language]);
	char *str = url_encode(solution->src);
	if (str == NULL) {
		write_log("encode url error.\n");
		free(str);
		free(post_str);
		return -1;
	}
	strcat(post_str, str);

	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_URL, url);
	// 设置参数
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_str);

	char filename[BUFSIZE];
	sprintf(filename, "%dsubmit.txt", solution->solution_id);
	if (DEBUG) {
		write_log("perform url is %s.\n", url);
		write_log("post data is %s.\n", post_str);
	}
	perform_curl(filename);

	char *html = (char *)malloc(BUFSIZE * BUFSIZE);
	if (html == NULL) {
		write_log("alloc submit_cf html buf memory error.\n");
		free(str);
		free(post_str);
		return -1;
	}
	load_file(filename, html);

	// modified form bnuoj
	if (strstr(html, "Error Occurred") != NULL
		|| strstr(html, "The page is temporarily unavailable") != NULL) {
		write_log("submit_cf remote server error.\n");
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

int get_ceinfo_cf(void)
{
	int status, i;
	regmatch_t pmatch[2];
	const int nmatch = 2;
	regex_t reg;
	const char *pattern = "<pre>(.*)</pre>";
	char url[BUFSIZE];
	sprintf(url, "http://poj.org/showcompileinfo?solution_id=%d",
			solution->remote_rid);

	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_URL, url);

	char err[BUFSIZE];
	int ret = regcomp(&reg, pattern, REG_EXTENDED);
	if (ret) {
		regerror(ret, &reg, err, BUFSIZE);
		write_log("compile regex error: %s.\n", err);
		return -1;
	}
	char *html = (char *)malloc(BUFSIZE * BUFSIZE);
	if (html == NULL) {
		write_log("alloc get_ceinfo_cf html buf memory error.\n");
		regfree(&reg);
		return -1;
	}
	perform_curl(cefname);
	load_file(cefname, html);
	// modified form bnuoj
	if (strstr(html, "Error Occurred") != NULL
		|| strstr(html, "The page is temporarily unavailable") != NULL) {
		write_log("get_ceinfo_cf remote server error.\n");
		write_log("get solution %d compile error info error.\n", solution->solution_id);
		free(html);
		regfree(&reg);
		return -1;
	} else {
		status = regexec(&reg, html, nmatch, pmatch, 0);
		if (status == REG_NOMATCH) {
			write_log("get_ceinfo_cf regex no match.\n", solution->solution_id);
			write_log("get solution %d compile error info error.\n", solution->solution_id);
			regfree(&reg);
			free(html);
			return -1;
		} else if (status == 0) {
			int cnt = 0;
			for (i = pmatch[1].rm_so; i < pmatch[1].rm_eo; ++i) {
				solution->compileinfo[cnt++] = html[i];
			}
			solution->compileinfo[cnt] = '\0';
			write_log("match_str = %s\n", solution->compileinfo);
			save_file(cefname, solution->compileinfo);
		}
	}
	write_log("get compile error info: %s.\n", solution->compileinfo);
	free(html);
	regfree(&reg);
	return 0;
}

int get_reinfo_cf(void)
{
	save_file(refname, solution->runtimeinfo);
	write_log("get runtime error info: %s.\n", solution->runtimeinfo);
	return 0;
}
int convert_result_cf(char *buf)
{
	if (strstr(buf, "Accepted") != NULL) {
		return OJ_AC;
	} else if (strstr(buf, "Presentation Error") != NULL) {
		return OJ_PE;
	} else if (strstr(buf, "Runtime Error") != NULL) {
		return OJ_RE;
	} else if (strstr(buf, "Wrong Answer") != NULL) {
		return OJ_WA;
	} else if (strstr(buf, "Time Limit Exceeded") != NULL) {
		return OJ_TL;
	} else if (strstr(buf, "Memory Limit Exceeded") != NULL) {
		return OJ_ML;
	} else if (strstr(buf, "Output Limit Exceeded") != NULL) {
		return OJ_OL;
	} else if (strstr(buf, "Compile Error") != NULL) {
		return OJ_CE;
	} else {
		return OJ_JE;
	}

	return OJ_JE;
}

int get_cf_problem_id(void)
{
	char url[] = "http://codeforces.com/api/problemset.problems";
	char filename[] = "cf_json";
	char buf[BUFSIZE];

	// 设置网址
	curl_easy_setopt(curl, CURLOPT_URL, url);

	write_log("try to get codeforces problem id.\n");
	write_log("url = %s.\n", url);

	// 执行数据请求
	if (perform_curl(filename) < 0) {
		return -1;
	}

	json_object *obj = json_object_from_file(filename);
	if (obj == NULL) {
		write_log("load json object from file %s error.\n", filename);
		return -1;
	}
	json_object *problems;
	if (strcmp(json_get_str(obj, "status"), "OK") != 0) {
		if (!DEBUG) {
			execute_cmd("rm -rf %s", filename);
		}
		return -1;
	} else {
		problems = json_get_obj(obj, "result.problems");
	}
	int i = 0;
	int len = json_object_array_length(problems);
	int pid = 0;
	for (i = 0; i < len; ++i) {
		sprintf(buf, "[%d].contestId", i);
		pid = json_get_int(problems, buf);
		sprintf(buf, "[%d].index", i);
		pid = pid * 10 + json_get_str(problems, buf)[0] - 'A';
		cf_pid[i] = pid;
	}
	cf_pid[len] = -1;
	cf_pid_len = len;

	if (!DEBUG) {
		execute_cmd("rm -rf %s", filename);
	}
	json_object_put(obj);
	return 0;
}

int get_status_cf(void)
{
	int status, i, j;
	regmatch_t pmatch[5];
	const int nmatch = 5;
	regex_t reg;
	const char *pattern = "<tr align=center><td>([0-9]*)</td>"
		"<td><a[^>]*>[^<]*</a></td>"
		"<td><a[^>]*>[^<]*</a></td>"
		"<td><font[^>]*>([ a-zA-Z]*)</font></td>"
		"<td>([0-9]*)K?</td>"
		"<td>([0-9]*)(MS)?</td>";
	char url[BUFSIZE];
	sprintf(url, "http://poj.org/status?problem_id=%d&user_id=%s&language=%d",
			solution->problem_info.origin_id, vjudge_user,
			lang_table[solution->problem_info.ojtype][solution->language]);

	if (DEBUG) {
		write_log("perform url is %s.\n", url);
	}

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
		write_log("alloc get_status_cf html buf memory error.\n");
		regfree(&reg);
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

		json_object *obj = json_object_from_file(filename);
		if (obj == NULL) {
			write_log("load json object from file %s error.\n", filename);
			return OJ_JE;
		}
		json_object *problems;
		if (strcmp(json_get_str(obj, "status"), "OK") != 0) {
			return OJ_JE;
		} else {
			problems = json_get_obj(obj, "result.problems");
		}
		int i = 0;
		int len = json_object_array_length(problems);
		int pid = 0;
		for (i = 0; i < len; ++i) {
			sprintf(buf, "[%d].contestId", i);
			pid = json_get_int(problems, buf);
			sprintf(buf, "[%d].index", i);
			pid = pid * 10 + json_get_str(problems, buf)[0] - 'A';
			cf_pid[i] = pid;
		}

		cf_pid[len] = -1;
		cf_pid_len = len;

		if (strstr(html, "Error Occurred") != NULL
			|| strstr(html, "The page is temporarily unavailable") != NULL) {
			write_log("get_status_cf remote server error.\n");
			write_log("get solution %d status error.\n", solution->solution_id);
			free(html);
			regfree(&reg);
			return OJ_JE;
		} else {
			status = regexec(&reg, html, nmatch, pmatch, 0);
			if (status == REG_NOMATCH) {
				write_log("no match regex: %s.\n", pattern);
				write_log("get solution %d status error.\n", solution->solution_id);
				free(html);
				regfree(&reg);
				return OJ_JE;
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
						case 3: memory = atoi(buf); break;
						case 4: usedtime = atoi(buf); break;
					}
				}
				write_log("match_str = %s\n", match_str);
				write_log("rid = %d\n", rid);
				write_log("usedtime = %d\n", usedtime);
				write_log("memory = %d\n", memory);
				write_log("result = %s\n", result);
				if (is_final_result(result)) {
					solution->remote_rid = rid;
					solution->time = usedtime;
					solution->memory = memory;
					free(html);
					regfree(&reg);
					int ret = convert_result(result);
					if (ret == OJ_RE) {
						strcpy(solution->runtimeinfo, result);
					}
					write_log("get solution %d status over"
							": %d.\n", solution->solution_id, ret);
					return ret;
				}
			}
		}
	}

	free(html);
	regfree(&reg);

	return OJ_JE;
}
