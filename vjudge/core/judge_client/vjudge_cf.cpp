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
		free(html);
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
	return 0;
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

	memset(cookies, 0, sizeof(tta));

	load_file(cookiename, cookies);

	int status, i, j;
	regmatch_t pmatch[2];
	const int nmatch = 2;
	regex_t reg;
	const char *pattern = "39ce7\\s([a-zA-Z0-9_]*)";
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
	regfree(&reg);
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
		free(html);
		return -1;
	}

	// important
	cleanup_curl();
	curl = prepare_curl();

	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_REFERER, url);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	// 设置参数
	sprintf(filename, "%dlogin.txt", solution->solution_id);
	memset(tta, 0, sizeof(tta));
	if (get_tta(tta) < 0) {
		free(html);
		write_log("login_cf get tta error.\n");
		return -1;
	}
	sprintf(post_str, "csrf_token=%s&action=enter&handle=%s&password=%s"
			"&_tta=%s", csrf, vjudge_user, vjudge_passwd, tta);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_str);
	if (DEBUG) {
		write_log("perform url is %s.\n", url);
		write_log("post data is %s.\n", post_str);
	}

	perform_curl(filename);
	load_file(filename, html);

	if (strstr(html, "Codeforces is temporary unavailable") != NULL
			|| strstr(html, "Invalid handle or password") != NULL
			|| strstr(html, "Fill in the form to login into Codeforces.") != NULL) {
		write_log("login_cf remote server error.\n");
		free(html);
		return -1;
	}

	free(html);
	return 0;
}

// modified from bnuoj
int submit_cf(void)
{
	char url[BUFSIZE] = "http://codeforces.com/problemset/submit";
	char csrf[BUFSIZE];
	char buf[BUFSIZE];

	cleanup_curl();
	curl = prepare_curl();

	if (get_csrf(url, csrf) < 0) {
		write_log("submit_cf get csrf error.\n");
		return -1;
	}

	// add random extra spaces in the end to avoid same code error
	srand(time(NULL));
	int len = strlen(solution->src);
	solution->src[len] = '\n';
	while (rand() % 120) {
		solution->src[len++] = ' ';
	}
	solution->src[len] = '\0';

	cleanup_curl();
	curl = prepare_curl();

	// copy from bnuoj
	// prepare form for post
	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;
	curl_formadd(&formpost, &lastptr,
		     CURLFORM_COPYNAME, "action",
		     CURLFORM_COPYCONTENTS, "submitSolutionFormSubmitted",
		     CURLFORM_END);
	//sprintf(buf, "%d", solution->problem_info.origin_id / 10);
	//curl_formadd(&formpost, &lastptr,
		     //CURLFORM_COPYNAME, "contestId",
		     //CURLFORM_COPYCONTENTS, buf, CURLFORM_END);
	//sprintf(buf, "%d", solution->problem_info.origin_id % 10 + 'A');
	//curl_formadd(&formpost, &lastptr,
		     //CURLFORM_COPYNAME, "submittedProblemIndex",
		     //CURLFORM_COPYCONTENTS, buf, CURLFORM_END);
	sprintf(buf, "%d%c", solution->problem_info.origin_id / 10,
			solution->problem_info.origin_id % 10 + 'A');
	curl_formadd(&formpost, &lastptr,
		     CURLFORM_COPYNAME, "submittedProblemCode",
		     CURLFORM_COPYCONTENTS, buf, CURLFORM_END);
	sprintf(buf, "%d", lang_table[solution->problem_info.ojtype][solution->language]);
	curl_formadd(&formpost, &lastptr,
		     CURLFORM_COPYNAME, "programTypeId",
		     CURLFORM_COPYCONTENTS, buf,
		     CURLFORM_END);
	curl_formadd(&formpost, &lastptr,
		     CURLFORM_COPYNAME, "source",
		     CURLFORM_COPYCONTENTS, solution->src, CURLFORM_END);
	curl_formadd(&formpost, &lastptr,
		     CURLFORM_COPYNAME, "sourceCodeConfirmed",
		     CURLFORM_COPYCONTENTS, "true", CURLFORM_END);
	memset(buf, 0, sizeof(buf));
	if (get_tta(buf) < 0) {
		write_log("submit_cf get tta error.\n");
		curl_formfree(formpost);
		return -1;
	}
	curl_formadd(&formpost, &lastptr,
		     CURLFORM_COPYNAME, "_tta", CURLFORM_COPYCONTENTS,
		     buf, CURLFORM_END);

	cleanup_curl();
	curl = prepare_curl();

	sprintf(url, "http://codeforces.com/problemset/submit?csrf_token=%s", csrf);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

	char filename[BUFSIZE];
	sprintf(filename, "%dsubmit.txt", solution->solution_id);
	if (DEBUG) {
		write_log("perform url is %s.\n", url);
	}

	perform_curl(filename);
	curl_formfree(formpost);

	char *html = (char *)malloc(BUFSIZE * BUFSIZE);
	if (html == NULL) {
		write_log("alloc submit_cf html buf memory error.\n");
		return -1;
	}
	load_file(filename, html);

	// modified form bnuoj
	if (strstr(html, "You have submitted exactly the same code before") != NULL
			|| strstr(html, "Choose valid language") != NULL
			|| strstr(html, "<span class=\"error for__source\">") != NULL
			|| strstr(html, "<a href=\"/enter\">Enter</a>") != NULL) {
		write_log("submit_cf remote server error.\n");
		free(html);
		return -1;
	}

	free(html);
	return 0;
}

// modified from bnuoj
int get_ceinfo_cf(void)
{
	int status, i;
	regmatch_t pmatch[2];
	const int nmatch = 2;
	regex_t reg;
	const char *pattern = "\"(.*)\"";
	char url[BUFSIZE];
	char csrf[BUFSIZE];
	char post_str[BUFSIZE];
	sprintf(url, "http://codeforces.com/problemset/submit");
	if (get_csrf(url, csrf) < 0) {
		write_log("get_ceinfo_cf get csrf error.\n");
		return -1;
	}
	sprintf(url, "http://codeforces.com/data/judgeProtocol");
	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_URL, url);
	sprintf(post_str, "submissionId=%d&csrf_token=%s",
			solution->remote_rid, csrf);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_str);

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

	if (strstr(html, "Codeforces is temporary unavailable") != NULL) {
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
	if (strstr(buf, "OK") != NULL) {
		return OJ_AC;
	} else if (strstr(buf, "PRESENTATION_ERROR") != NULL) {
		return OJ_PE;
	} else if (strstr(buf, "RUNTIME_ERROR") != NULL) {
		return OJ_RE;
	} else if (strstr(buf, "WRONG_ANSWER") != NULL) {
		return OJ_WA;
	} else if (strstr(buf, "TIME_LIMIT_EXCEEDED") != NULL) {
		return OJ_TL;
	} else if (strstr(buf, "MEMORY_LIMIT_EXCEEDED") != NULL
			|| strstr(buf, "IDLENESS_LIMIT_EXCEEDED") != NULL) {
		return OJ_ML;
	} else if (strstr(buf, "COMPILATION_ERROR") != NULL) {
		return OJ_CE;
	} else {
		return OJ_JE;
	}

	return OJ_JE;
}

int get_status_cf(void)
{
	char url[BUFSIZE];
	sprintf(url, "http://codeforces.com/api/contest.status?contestId=%d&from=1&count=1&handle=%s",
			solution->problem_info.origin_id / 10, vjudge_user);

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
	while (1) {
		if (time(NULL) - begin_time > vj_max_wait_time) {
			write_log("judge time out.\n");
			return OJ_JE;
		}

		perform_curl(filename);

		json_object *obj = json_object_from_file(filename);
		if (obj == NULL) {
			write_log("load json object from file %s error.\n", filename);
			return OJ_JE;
		}
		json_object *submission;
		if (strcmp(json_get_str(obj, "status"), "OK") != 0) {
			json_object_put(obj);
			return OJ_JE;
		} else {
			submission = json_get_obj(obj, "result[0]");
		}

		if (submission == NULL) {
			write_log("get status error, maybe not submit.\n");
			json_object_put(obj);
			return OJ_JE;
		}

		rid = json_get_int(submission, "id");
		strcpy(result, json_get_str(submission, "verdict"));
		usedtime = json_get_int(submission, "timeConsumedMillis");
		memory = json_get_int(submission, "memoryConsumedBytes");
		memory = memory / 1024 + ((memory % 1024) ? 1: 0);

		write_log("rid = %d\n", rid);
		write_log("usedtime = %d\n", usedtime);
		write_log("memory = %d\n", memory);
		write_log("result = %s\n", result);
		if (is_final_result(result)) {
			solution->remote_rid = rid;
			solution->time = usedtime;
			solution->memory = memory;
			int ret = convert_result(result);
			if (ret == OJ_RE) {
				strcpy(solution->runtimeinfo, result);
			}
			write_log("get solution %d status over"
					": %d.\n", solution->solution_id, ret);
			json_object_put(obj);
			return ret;
		}
	}

	return OJ_JE;
}
