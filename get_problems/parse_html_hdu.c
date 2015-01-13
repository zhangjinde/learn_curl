/*************************************************************************
	> File Name: parse_html_hdu.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时02分35秒
 ************************************************************************/

/*
 * get hduoj problem
 */
#include "get_problem.h"

// title start
static void hdu_starttag_h1(void *cbdata, ekhtml_string_t * tag,
				ekhtml_attr_t * attrs)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->istitle = 1;
}

// title end
static void hdu_endtag_h1(void *cbdata, ekhtml_string_t * str)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->istitle = 0;
}

// time limit and status start
static void hdu_starttag_span(void *cbdata, ekhtml_string_t * tag,
				ekhtml_attr_t * attrs)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->islimit = 1;
	state->isstat = 1;
	state->isspj = 1;
}

// time limit and status end
static void hdu_endtag_span(void *cbdata, ekhtml_string_t * str)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->islimit = 0;
	state->isstat = 0;
	state->isspj = 0;
}

// div start
static void hdu_endtag_div(void *cbdata, ekhtml_string_t * str)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
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

// tag start
static void hdu_starttag(void *cbdata, ekhtml_string_t * tag,
			    ekhtml_attr_t * attrs)
{
	char tagname[20];
	char *tmp_str = (char *)malloc(BUFSIZE * BUFSIZE);
	if (tmp_str == NULL) {
		write_log("alloc hdu_starttag tmp_str buf memory error.\n");
		return;
	}
	struct html_state_t *state = (struct html_state_t *)cbdata;
	memset(tagname, 0, sizeof(tagname));
	memset(tmp_str, 0, BUFSIZE * BUFSIZE);
	strncpy(tagname, tag->str, tag->len);

	if (strcmp(tagname, "DIV") == 0) {
		free(tmp_str);
		return;
	}

	if (state->isdescription || state->isinput || state->isoutput
			|| state->issinput || state->issoutput
			|| state->ishint || state->istitle || state->islimit) {
		ekhtml_attr_t *attr;
		sprintf(tmp_str, "<%s ", tagname);
		for (attr = attrs; attr; attr = attr->next) {
			strncat(tmp_str, attr->name.str, attr->name.len);
			strcat(tmp_str, "=\"");
			// 特殊处理图像
			if (strcmp(tagname, "IMG") == 0 && strncmp(attr->name.str, "src", 3) == 0) {
				char attrval[BUFSIZE];
				memset(attrval, 0, sizeof(attrval));
				strncpy(attrval, attr->val.str, attr->val.len);
				strcat(tmp_str, "http://acm.hdu.edu.cn/data/images");
				strcat(tmp_str, &attrval[strrchr(attrval, '/') - attrval]);
			} else {
				if (!attr->isBoolean) {
					strncat(tmp_str, attr->val.str, attr->val.len);
				} else {
					strncat(tmp_str, attr->name.str, attr->name.len);
				}
			}
			strcat(tmp_str, "\" ");
		}
		strcat(tmp_str, ">");
	}

	if (state->isdescription) {
		strcat(state->problem_info->description, tmp_str);
	}
	if (state->isinput) {
		strcat(state->problem_info->input, tmp_str);
	}
	if (state->isoutput) {
		strcat(state->problem_info->output, tmp_str);
	}
	if (state->issoutput) {
		if (strcmp(tagname, "I") == 0) {
			--state->issoutput;
		}
	}
	if (state->ishint) {
		strcat(state->problem_info->hint, tmp_str);
	}
	free(tmp_str);
}

// tag end
static void hdu_endtag(void *cbdata, ekhtml_string_t * str)
{
	char tagname[20];
	char *tmp_str = (char *)malloc(BUFSIZE * BUFSIZE);
	if (tmp_str == NULL) {
		fprintf(stderr, "分配内存失败！\n");
		return;
	}
	struct html_state_t *state = (struct html_state_t *)cbdata;
	memset(tagname, 0, sizeof(tagname));
	memset(tmp_str, 0, BUFSIZE * BUFSIZE);
	strncpy(tagname, str->str, str->len);

	if (state->isdescription) {
		sprintf(tmp_str, "</%s>", tagname);
		strcat(state->problem_info->description, tmp_str);
	}
	if (state->isinput) {
		sprintf(tmp_str, "</%s>", tagname);
		strcat(state->problem_info->input, tmp_str);
	}
	if (state->isoutput) {
		sprintf(tmp_str, "</%s>", tagname);
		strcat(state->problem_info->output, tmp_str);
	}
	if (state->ishint) {
		if (strcmp(tagname, "I") != 0) {
			sprintf(tmp_str, "</%s>", tagname);
			strcat(state->problem_info->hint, tmp_str);
		}
	}
	free(tmp_str);
}

// process tag data
static void hdu_data(void *cbdata, ekhtml_string_t * str)
{
	char *buf = (char *)malloc(BUFSIZE * BUFSIZE);
	if (buf == NULL) {
		fprintf(stderr, "分配内存失败！\n");
		return;
	}
	memset(buf, 0, BUFSIZE * BUFSIZE);
	strncpy(buf, str->str, str->len);
	struct html_state_t *state = (struct html_state_t *)cbdata;
	// 获取标题
	if (state->istitle) {
		strncpy(state->problem_info->title, str->str, str->len);
	}
	if (state->islimit) {		// 获取限制
		if (strstr(buf, "Time Limit") != NULL) {
			int time_limit = 0;
			int memory_limit = 0;
			int tmp[2];
			sscanf(buf, "Time Limit: %d/%d MS (Java/Others)&nbsp"
					";&nbsp;&nbsp;&nbsp;Memory Limit:"
					" %d/%d K (Java/Others)",
					&tmp[0], &time_limit, &tmp[1], &memory_limit);
			time_limit = time_limit / 1000 + ((time_limit % 1000 == 0) ? 0 : 1);
			memory_limit /= 1024;
			state->problem_info->time_limit = time_limit;
			state->problem_info->memory_limit = memory_limit;
		}
	}
	if (state->isstat) {
		if (strstr(buf, "Total Submission") != NULL) {
			int submit = 0;
			int accepted = 0;
			sscanf(buf, "Total Submission(s): %d&nbsp;&nbsp;"
					"&nbsp;&nbsp;Accepted Submission(s): "
					"%d", &submit, &accepted);
			state->problem_info->submit = submit;
			state->problem_info->accepted = accepted;
		}
	}
	if (state->isspj) {
		if (strstr(buf, "Special Judge") != NULL) {
			state->problem_info->spj = 1;
		}
	}
	if (state->isdescription) {
		strcat(state->problem_info->description, buf);
	}
	if (state->isinput) {
		strcat(state->problem_info->input, buf);
	}
	if (state->isoutput) {
		strcat(state->problem_info->output, buf);
	}
	if (state->issinput) {
		strcat(state->problem_info->sample_input, buf);
	}
	if (state->issoutput) {
		strcat(state->problem_info->sample_output, buf);
	}
	if (state->ishint) {
		strcat(state->problem_info->hint, buf);
	}

	if (strcmp("Problem Description", buf) == 0) {
		state->isdescription = 2;
	}
	if (strcmp("Input", buf) == 0) {
		state->isinput = 2;
	}
	if (strcmp("Output", buf) == 0) {
		state->isoutput = 2;
	}
	if (strcmp("Sample Input", buf) == 0) {
		state->issinput = 2;
	}
	if (strcmp("Sample Output", buf) == 0) {
		state->issoutput = 2;
	}
	if (strcmp("Hint", buf) == 0) {
		state->ishint = 2;
	}
	free(buf);
}

int parse_html_hdu(char *buf)
{
	struct html_state_t cbdata;
	memset(&cbdata, 0, sizeof(struct html_state_t));
	cbdata.problem_info = problem_info;
	ekhtml_parser_t *ekparser = prepare_ekhtml(&cbdata);

	// set callback function or data
	ekhtml_parser_datacb_set(ekparser, hdu_data);
	ekhtml_parser_startcb_add(ekparser, NULL, hdu_starttag);
	ekhtml_parser_startcb_add(ekparser, "H1", hdu_starttag_h1);
	ekhtml_parser_startcb_add(ekparser, "SPAN", hdu_starttag_span);
	ekhtml_parser_endcb_add(ekparser, NULL, hdu_endtag);
	ekhtml_parser_endcb_add(ekparser, "H1", hdu_endtag_h1);
	ekhtml_parser_endcb_add(ekparser, "SPAN", hdu_endtag_span);
	ekhtml_parser_endcb_add(ekparser, "DIV", hdu_endtag_div);

	ekhtml_string_t str;
	str.str = buf;
	str.len = strlen(buf);
	ekhtml_parser_feed(ekparser, &str);
	ekhtml_parser_flush(ekparser, 0);

	cleanup_ekhtml(ekparser);
	return 0;
}
