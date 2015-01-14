/*************************************************************************
	> File Name: parse_html_cf.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时02分35秒
 ************************************************************************/

/*
 * get codeforces problem
 */
#include "get_problem.h"

static void cf_endtag_div(void *cbdata, ekhtml_string_t * str)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	if (state->istitle) {
		--state->istitle;
	}
	if (state->isdescription) {
		--state->isdescription;
	}
	if (state->isinput) {
		--state->isinput;
	}
	if (state->isoutput) {
		--state->isoutput;
	}
	if (state->issinput) {
		--state->issinput;
	}
	if (state->issoutput) {
		--state->issoutput;
	}
	if (state->ishint) {
		--state->ishint;
	}
}

static void cf_starttag(void *cbdata, ekhtml_string_t * tag,
			    ekhtml_attr_t * attrs)
{
	char tagname[20];
	struct html_state_t *state = (struct html_state_t *)cbdata;
	memset(tagname, 0, sizeof(tagname));
	strncpy(tagname, tag->str, tag->len);

	if (strcmp(tagname, "DIV") == 0) {
		ekhtml_attr_t *attr;
		for (attr = attrs; attr; attr = attr->next) {
			if (attr->name.len == 5 && strncmp(attr->name.str, "class", attr->name.len) == 0) {
				if (attr->val.len == 6 && strncmp(attr->val.str, "header", attr->val.len) == 0) {
					state->isspj = 1;
					state->istitle = 1;
				}
				if (attr->val.len == 19 && strncmp(attr->val.str, "input-specification", attr->val.len) == 0) {
					state->isinput = 2;
				}
				if (attr->val.len == 20 && strncmp(attr->val.str, "output-specification", attr->val.len) == 0) {
					state->isoutput = 2;
				}
				if (attr->val.len == 4 && strncmp(attr->val.str, "note", attr->val.len) == 0) {
					state->ishint = 2;
				}
				if (attr->val.len == 5 && strncmp(attr->val.str, "input", attr->val.len) == 0) {
					state->issinput = 2;
				}
				if (attr->val.len == 6 && strncmp(attr->val.str, "output", attr->val.len) == 0) {
					state->issoutput = 2;
				}
			}
		}
		return;
	}
	if (strcmp(tagname, "SPAN") == 0) {
		ekhtml_attr_t *attr;
		for (attr = attrs; attr; attr = attr->next) {
			if (attr->name.len == 5 && strncmp(attr->name.str, "class", attr->name.len) == 0) {
				if (attr->val.len == 8 && strncmp(attr->val.str, "tex-span", attr->val.len) == 0) {
					return;
				}
			}
		}
	}
	if (strcmp(tagname, "P") == 0) {
		return;
	}

	starttag(cbdata, tag, attrs);
}

static void cf_endtag(void *cbdata, ekhtml_string_t * str)
{
	char tagname[20];
	memset(tagname, 0, sizeof(tagname));
	strncpy(tagname, str->str, str->len);
	// should no span
	if (strcmp(tagname, "SPAN") == 0) {
		return;
	}
	endtag(cbdata, str);
}

static void cf_data(void *cbdata, ekhtml_string_t * str)
{
	char *buf = (char *)malloc(BUFSIZE * BUFSIZE);
	if (buf == NULL) {
		write_log("alloc cf_data buf memory error.\n");
		return;
	}
	memset(buf, 0, BUFSIZE * BUFSIZE);
	strncpy(buf, str->str, str->len);
	struct html_state_t *state = (struct html_state_t *)cbdata;

	tagdata(cbdata, str);

	int time_limit;
	int memory_limit;
	// time limit
	if (state->islimit == 1) {
		sscanf(buf, "%dseconds", &time_limit);
		state->problem_info->time_limit = time_limit;
		state->islimit = 0;
	}
	// memory limit
	if (state->islimit == 2) {
		sscanf(buf, "%dmegabytes", &memory_limit);
		state->problem_info->memory_limit = memory_limit;
		state->islimit = 0;
	}

	if (state->isspj) {
		if (strstr(buf, "time limit per test") != NULL) {
			state->islimit = 1;
		}
		if (strstr(buf, "memory limit per test") != NULL) {
			state->islimit = 2;
		}
		if (strstr(buf, "standard output") != NULL) {
			state->isdescription = 3;
			state->isspj = 0;
		}
	}

	free(buf);
}

// memory should free
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

int parse_html_cf(char *buf)
{
	struct html_state_t cbdata;
	memset(&cbdata, 0, sizeof(struct html_state_t));
	cbdata.problem_info = problem_info;
	ekhtml_parser_t *ekparser = prepare_ekhtml(&cbdata);

	// set callback function or data
	ekhtml_parser_datacb_set(ekparser, cf_data);
	ekhtml_parser_startcb_add(ekparser, NULL, cf_starttag);
	ekhtml_parser_endcb_add(ekparser, NULL, cf_endtag);
	ekhtml_parser_endcb_add(ekparser, "DIV", cf_endtag_div);

	ekhtml_string_t str;
	str.str = buf;
	str.len = strlen(buf);
	ekhtml_parser_feed(ekparser, &str);
	ekhtml_parser_flush(ekparser, 0);

	char tmp[BUFSIZE];
	strcpy(tmp, problem_info->title);
	strcpy(problem_info->title, tmp + 3);

	cleanup_ekhtml(ekparser);
	return 0;
}
