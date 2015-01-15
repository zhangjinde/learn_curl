/*************************************************************************
	> File Name: ekhtml_handle.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2015年01月12日 星期一 22时33分17秒
 ************************************************************************/

#include "get_problem.h"

// tag start
void starttag(void *cbdata, ekhtml_string_t * tag,
			    ekhtml_attr_t * attrs)
{
	char tagname[20];
	char *tmp_str = (char *)malloc(BUFSIZE * BUFSIZE);
	if (tmp_str == NULL) {
		write_log("alloc starttag tmp_str buf memory error.\n");
		return;
	}
	char *attrval = (char *)malloc(BUFSIZE * BUFSIZE);
	if (attrval == NULL) {
		write_log("alloc starttag attrval buf memory error.\n");
		return;
	}
	struct html_state_t *state = (struct html_state_t *)cbdata;
	memset(tagname, 0, sizeof(tagname));
	memset(tmp_str, 0, BUFSIZE * BUFSIZE);
	strncpy(tagname, tag->str, tag->len);

	if (state->isdescription || state->isinput || state->isoutput
			|| state->issinput || state->issoutput
			|| state->ishint || state->istitle || state->islimit) {
		ekhtml_attr_t *attr;
		sprintf(tmp_str, "<%s ", tagname);
		for (attr = attrs; attr; attr = attr->next) {
			strncat(tmp_str, attr->name.str, attr->name.len);
			strcat(tmp_str, "=\"");
			// 特殊处理图像
			if (strcmp(tagname, "IMG") == 0 && strncmp(attr->name.str, "src", attr->name.len) == 0) {
				memset(attrval, 0, BUFSIZE * BUFSIZE);
				strncpy(attrval, attr->val.str, attr->val.len);
				strcat(tmp_str, oj_imgurl[oj_type]);
				int pos = 0;
				while (!(attrval[pos] >= 'a' && attrval[pos] <= 'z')) {
					++pos;
				}
				strcat(tmp_str, &attrval[pos]);
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

	if (state->isdescription == 1) {
		strcat(state->problem_info->description, tmp_str);
	}
	if (state->isinput == 1) {
		strcat(state->problem_info->input, tmp_str);
	}
	if (state->isoutput == 1) {
		strcat(state->problem_info->output, tmp_str);
	}
	if (state->ishint == 1) {
		strcat(state->problem_info->hint, tmp_str);
	}
	free(tmp_str);
}

// tag end
void endtag(void *cbdata, ekhtml_string_t * str)
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

	// codeforces
	if (oj_type == 2 && strcmp(tagname, "P") == 0) {
		strcpy(tmp_str, "<BR>");
	} else {
		sprintf(tmp_str, "</%s>", tagname);
	}
	if (state->isdescription == 1) {
		strcat(state->problem_info->description, tmp_str);
	}
	if (state->isinput == 1) {
		strcat(state->problem_info->input, tmp_str);
	}
	if (state->isoutput == 1) {
		strcat(state->problem_info->output, tmp_str);
	}
	if (state->ishint == 1) {
		strcat(state->problem_info->hint, tmp_str);
	}
	free(tmp_str);
}

void tagdata(void *cbdata, ekhtml_string_t * str)
{
	char *buf = (char *)malloc(BUFSIZE * BUFSIZE);
	if (buf == NULL) {
		write_log("alloc tagdata buf memory error.\n");
		return;
	}
	memset(buf, 0, BUFSIZE * BUFSIZE);
	strncpy(buf, str->str, str->len);
	struct html_state_t *state = (struct html_state_t *)cbdata;
	if (state->istitle == 1) {
		strncpy(state->problem_info->title, str->str, str->len);
	}
	if (state->isdescription == 1) {
		strcat(state->problem_info->description, buf);
	}
	if (state->isinput == 1) {
		strcat(state->problem_info->input, buf);
	}
	if (state->isoutput == 1) {
		strcat(state->problem_info->output, buf);
	}
	if (state->issinput == 1) {
		strcat(state->problem_info->sample_input, buf);
		if (oj_type == 2) {		// codeforces
			strcat(state->problem_info->sample_input, "\n");
		}
	}
	if (state->issoutput == 1) {
		strcat(state->problem_info->sample_output, buf);
		if (oj_type == 2) {		// codeforces
			strcat(state->problem_info->sample_output, "\n");
		}
	}
	if (state->ishint == 1) {
		strcat(state->problem_info->hint, buf);
	}

	free(buf);
}

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
