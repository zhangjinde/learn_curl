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

static void hdu_starttag_h1(void *cbdata, ekhtml_string_t * tag,
				ekhtml_attr_t * attrs)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->istitle = 1;
}

static void hdu_endtag_h1(void *cbdata, ekhtml_string_t * str)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->istitle = 0;
}

static void hdu_starttag_span(void *cbdata, ekhtml_string_t * tag,
				ekhtml_attr_t * attrs)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->islimit = 1;
	state->isstat = 1;
	state->isspj = 1;
}

static void hdu_endtag_span(void *cbdata, ekhtml_string_t * str)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->islimit = 0;
	state->isstat = 0;
	state->isspj = 0;
}

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

static void hdu_starttag(void *cbdata, ekhtml_string_t * tag,
			    ekhtml_attr_t * attrs)
{
	char tagname[20];
	struct html_state_t *state = (struct html_state_t *)cbdata;
	memset(tagname, 0, sizeof(tagname));
	strncpy(tagname, tag->str, tag->len);

	if (strcmp(tagname, "DIV") == 0) {
		return;
	}

	starttag(cbdata, tag, attrs);


	if (state->issoutput) {
		if (strcmp(tagname, "I") == 0) {
			--state->issoutput;
		}
	}
}

static void hdu_endtag(void *cbdata, ekhtml_string_t * str)
{
	char tagname[20];
	struct html_state_t *state = (struct html_state_t *)cbdata;
	memset(tagname, 0, sizeof(tagname));
	strncpy(tagname, str->str, str->len);

	if (state->ishint && strcmp(tagname, "I") == 0) {
		return;
	}

	endtag(cbdata, str);
}

static void hdu_data(void *cbdata, ekhtml_string_t * str)
{
	char *buf = (char *)malloc(BUFSIZE * BUFSIZE);
	if (buf == NULL) {
		write_log("alloc hdu_data buf memory error.\n");
		return;
	}
	memset(buf, 0, BUFSIZE * BUFSIZE);
	strncpy(buf, str->str, str->len);
	struct html_state_t *state = (struct html_state_t *)cbdata;

	tagdata(cbdata, str);

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
			memory_limit = memory_limit / 1024 + ((memory_limit % 1024 == 0) ? 0 : 1);
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
