/*************************************************************************
	> File Name: parse_html_poj.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时02分35秒
 ************************************************************************/

/*
 * get poj problem
 */
#include "get_problem.h"

// p tag end
static void poj_endtag_p(void *cbdata, ekhtml_string_t * str)
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

// pre tag end
static void poj_endtag_pre(void *cbdata, ekhtml_string_t * str)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	if (state->issinput) {
		--state->issinput;
	}
	if (state->issoutput) {
		--state->issoutput;
	}
}

// div start
static void poj_endtag_div(void *cbdata, ekhtml_string_t * str)
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
	if (state->isspj) {
		--state->isspj;
	}
}

// tag start
static void poj_starttag(void *cbdata, ekhtml_string_t * tag,
			    ekhtml_attr_t * attrs)
{
	char tagname[20];
	struct html_state_t *state = (struct html_state_t *)cbdata;
	memset(tagname, 0, sizeof(tagname));
	strncpy(tagname, tag->str, tag->len);

	if (strcmp(tagname, "DIV") == 0) {
		ekhtml_attr_t *attr;
		for (attr = attrs; attr; attr = attr->next) {
			if (strncmp(attr->name.str, "class", attr->name.len) == 0) {
				if (strncmp(attr->val.str, "ptt", attr->val.len) == 0) {
					state->istitle = 1;
				}
				if (strncmp(attr->val.str, "plm", attr->val.len) == 0) {
					state->isspj = 1;
				}
			}
		}
		return;
	}

	starttag(cbdata, tag, attrs);
}

// tag end
static void poj_endtag(void *cbdata, ekhtml_string_t * str)
{
	endtag(cbdata, str);
}

// process tag data
static void poj_data(void *cbdata, ekhtml_string_t * str)
{
	char *buf = (char *)malloc(BUFSIZE * BUFSIZE);
	if (buf == NULL) {
		write_log("alloc poj_data buf memory error.\n");
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
		sscanf(buf, "%dMS", &time_limit);
		state->problem_info->time_limit = time_limit / 1000 +
			((time_limit % 1000 == 0) ? 0 : 1);
		state->islimit = 0;
	}
	// memory limit
	if (state->islimit == 2) {
		sscanf(buf, "%dK", &memory_limit);
		state->problem_info->memory_limit = memory_limit / 1024
			+ ((memory_limit % 1024 == 0) ? 0 : 1);
		state->islimit = 0;
	}
	// total submissions
	if (state->isstat == 1) {
		sscanf(buf, "%d", &state->problem_info->submit);
		state->isstat = 0;
	}
	// accepted
	if (state->isstat == 2) {
		sscanf(buf, "%d", &state->problem_info->accepted);
		state->isstat = 0;
	}

	if (state->isspj) {
		if (strstr(buf, "Time Limit:") != NULL) {
			state->islimit = 1;
		}
		if (strstr(buf, "Memory Limit:") != NULL) {
			state->islimit = 2;
		}
		if (strstr(buf, "Total Submissions:") != NULL) {
			state->isstat = 1;
		}
		if (strstr(buf, "Accepted:") != NULL) {
			state->isstat = 2;
		}
		if (strstr(buf, "Special Judge") != NULL) {
			state->problem_info->spj = 1;
		}
	}

	if (strcmp("Description", buf) == 0) {
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

int parse_html_poj(char *buf)
{
	struct html_state_t cbdata;
	memset(&cbdata, 0, sizeof(struct html_state_t));
	cbdata.problem_info = problem_info;
	ekhtml_parser_t *ekparser = prepare_ekhtml(&cbdata);

	// set callback function or data
	ekhtml_parser_datacb_set(ekparser, poj_data);
	ekhtml_parser_startcb_add(ekparser, NULL, poj_starttag);
	ekhtml_parser_endcb_add(ekparser, NULL, poj_endtag);
	ekhtml_parser_endcb_add(ekparser, "P", poj_endtag_p);
	ekhtml_parser_endcb_add(ekparser, "DIV", poj_endtag_div);
	ekhtml_parser_endcb_add(ekparser, "PRE", poj_endtag_pre);

	ekhtml_string_t str;
	str.str = buf;
	str.len = strlen(buf);
	ekhtml_parser_feed(ekparser, &str);
	ekhtml_parser_flush(ekparser, 0);

	cleanup_ekhtml(ekparser);
	return 0;
}
