/*************************************************************************
	> File Name: ekhtml_handle.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2015年01月12日 星期一 22时33分17秒
 ************************************************************************/

#include "get_problem.h"

ekhtml_parser_t *prepare_ekhtml(void *cbdata)
{
	ekhtml_parser_t *ekparser = ekhtml_parser_new(NULL);
	ekhtml_parser_cbdata_set(ekparser, cbdata);
	return ekparser;
}

void cleanup_ekhtml(ekhtml_parser_t *ekparser)
{
	ekhtml_parser_flush(ekparser, 1);
	ekhtml_parser_destroy(ekparser);
}

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
static void starttag(void *cbdata, ekhtml_string_t * tag,
			    ekhtml_attr_t * attrs)
{
	char tagname[20];
	char *tmp_str = (char *)malloc(BUFSIZE * BUFSIZE);
	if (tmp_str == NULL) {
		write_log("alloc starttag tmp_str buf memory error.\n");
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
				strcat(tmp_str, oj_imgurl[oj_type]);
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
static void endtag(void *cbdata, ekhtml_string_t * str)
{
	char tagname[20];
	char *tmp_str = (char *)malloc(BUFSIZE * BUFSIZE);
	if (tmp_str == NULL) {
		write_log("alloc endtag tmp_str buf memory error.\n");
		return;
	}
	struct html_state_t *state = (struct html_state_t *)cbdata;
	memset(tagname, 0, sizeof(tagname));
	memset(tmp_str, 0, BUFSIZE * BUFSIZE);
	strncpy(tagname, str->str, str->len);

	sprintf(tmp_str, "</%s>", tagname);
	if (state->isdescription) {
		strcat(state->problem_info->description, tmp_str);
	}
	if (state->isinput) {
		strcat(state->problem_info->input, tmp_str);
	}
	if (state->isoutput) {
		strcat(state->problem_info->output, tmp_str);
	}
	if (state->ishint) {
		strcat(state->problem_info->hint, tmp_str);
	}
	free(tmp_str);
}

// process tag data
static void tagdata(void *cbdata, ekhtml_string_t * str)
{
	char *buf = (char *)malloc(BUFSIZE * BUFSIZE);
	if (buf == NULL) {
		write_log("alloc tagdata buf memory error.\n");
		return;
	}
	memset(buf, 0, BUFSIZE * BUFSIZE);
	strncpy(buf, str->str, str->len);
	struct html_state_t *state = (struct html_state_t *)cbdata;
	if (state->istitle) {
		strncpy(state->problem_info->title, str->str, str->len);
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
