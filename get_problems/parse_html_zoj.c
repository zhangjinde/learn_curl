/*************************************************************************
	> File Name: parse_html_zoj.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时02分35秒
 ************************************************************************/

/*
 * get zoj problem
 */
#include "get_problem.h"

static void zoj_starttag_span(void *cbdata, ekhtml_string_t * tag,
				ekhtml_attr_t * attrs)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->istitle = 1;
	state->isspj = 2;
}

static void zoj_starttag_hr(void *cbdata, ekhtml_string_t * tag,
				ekhtml_attr_t * attrs)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	if (!state->isdescription) {
		state->isdescription = 2;
	} else {
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

static void zoj_endtag_span(void *cbdata, ekhtml_string_t * str)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->istitle = 0;
}

static void zoj_endtag_center(void *cbdata, ekhtml_string_t * str)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	if (state->isspj) {
		--state->isspj;
	}
}

static void zoj_endtag_b(void *cbdata, ekhtml_string_t * str)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
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
static void zoj_starttag(void *cbdata, ekhtml_string_t * tag,
			    ekhtml_attr_t * attrs)
{
	starttag(cbdata, tag, attrs);
}

// tag end
static void zoj_endtag(void *cbdata, ekhtml_string_t * str)
{
	endtag(cbdata, str);
}

// process tag data
static void zoj_data(void *cbdata, ekhtml_string_t * str)
{
	char *buf = (char *)malloc(BUFSIZE * BUFSIZE);
	if (buf == NULL) {
		write_log("alloc zoj_data buf memory error.\n");
		return;
	}
	memset(buf, 0, BUFSIZE * BUFSIZE);
	strncpy(buf, str->str, str->len);
	struct html_state_t *state = (struct html_state_t *)cbdata;

	tagdata(cbdata, str);

	int time_limit = 0;
	int memory_limit = 0;
	// time limit
	if (state->islimit == 1) {		// 获取限制
		sscanf(buf, "%dSeconds", &time_limit);
		time_limit = time_limit / 1000 + ((time_limit % 1000 == 0) ? 0 : 1);
		state->problem_info->time_limit = time_limit;
	}
	// memory limit
	if (state->islimit == 2) {
		sscanf(buf, "%dKB", &memory_limit);
		memory_limit = memory_limit / 1024 + ((memory_limit % 1024 == 0) ? 0 : 1);
		state->problem_info->memory_limit = memory_limit;
	}

	if (state->isspj) {
		if (strstr(buf, "Time Limit:") != NULL) {
			state->islimit = 1;
		}
		if (strstr(buf, "Time Limit:") != NULL) {
			state->islimit = 2;
		}
	}

	if (strcmp("Input:", buf) == 0 || strcmp("Input", buf) == 0) {
		state->isdescription = 0;
		state->isinput = 2;
	}
	if (strcmp("Output", buf) == 0 || strcmp("Output:", buf) == 0) {
		state->isinput = 0;
		state->isdescription = 0;
		state->isoutput = 2;
	}
	if (strcmp("Sample Input", buf) == 0
			|| strcmp("Sample Input:", buf) == 0) {
		state->isinput = 0;
		state->isoutput = 0;
		state->isdescription = 0;
		state->issinput = 2;
	}
	if (strcmp("Sample Output", buf) == 0
			|| strcmp("Sample Output:", buf) == 0
			|| strcmp("Output for the Sample Input", buf) == 0
			|| strcmp("Example Output", buf) == 0) {
		state->isinput = 0;
		state->isoutput = 0;
		state->issinput = 0;
		state->isdescription = 0;
		state->issoutput = 2;
	}
	if (strcmp("Hint", buf) == 0) {
		state->isinput = 0;
		state->isoutput = 0;
		state->issinput = 0;
		state->issoutput = 0;
		state->isdescription = 0;
		state->ishint = 2;
	}

	free(buf);
}

/*
 * zoj parse is not work.
 */
int parse_html_zoj(char *buf)
{
	struct html_state_t cbdata;
	memset(&cbdata, 0, sizeof(struct html_state_t));
	cbdata.problem_info = problem_info;
	ekhtml_parser_t *ekparser = prepare_ekhtml(&cbdata);

	// set callback function or data
	ekhtml_parser_datacb_set(ekparser, zoj_data);
	ekhtml_parser_startcb_add(ekparser, NULL, zoj_starttag);
	ekhtml_parser_startcb_add(ekparser, "SPAN", zoj_starttag_span);
	ekhtml_parser_startcb_add(ekparser, "HR", zoj_starttag_hr);
	ekhtml_parser_endcb_add(ekparser, NULL, zoj_endtag);
	ekhtml_parser_endcb_add(ekparser, "SPAN", zoj_endtag_span);
	ekhtml_parser_endcb_add(ekparser, "CENTER", zoj_endtag_center);
	ekhtml_parser_endcb_add(ekparser, "B", zoj_endtag_b);

	ekhtml_string_t str;
	str.str = buf;
	str.len = strlen(buf);
	ekhtml_parser_feed(ekparser, &str);
	ekhtml_parser_flush(ekparser, 0);

	cleanup_ekhtml(ekparser);
	return 0;
}
