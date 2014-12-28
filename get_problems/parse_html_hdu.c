/*************************************************************************
	> File Name: parse_html_hdu.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时02分35秒
 ************************************************************************/

/*
 * 获取杭电的题目描述
 */
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <curl/curl.h>
#include <mysql/mysql.h>

#include "main.h"
#include "ekhtml.h"

extern int ojcnt;
extern char ojstr[OJMAX][BUFSIZE];
extern char ojurl[OJMAX][BUFSIZE];

// 题目标题开始
static void hdu_starttag_h1(void *cbdata, ekhtml_string_t * tag,
				ekhtml_attr_t * attrs)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->istitle = 1;
}

// 题目标题结束
static void hdu_endtag_h1(void *cbdata, ekhtml_string_t * str)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->istitle = 0;
}

// 题目限制开始
static void hdu_starttag_span(void *cbdata, ekhtml_string_t * tag,
				ekhtml_attr_t * attrs)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->islimit = 1;
}

// 题目限制结束
static void hdu_endtag_span(void *cbdata, ekhtml_string_t * str)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->islimit = 0;
}

// div标签结束
static void hdu_endtag_div(void *cbdata, ekhtml_string_t * str)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	if (state->isdescription) {
		--state->isdescription;
	}
}

// 标签开始
static void hdu_starttag(void *cbdata, ekhtml_string_t * tag,
			    ekhtml_attr_t * attrs)
{
	char tagname[BUFSIZE];
	struct html_state_t *state = (struct html_state_t *)cbdata;
	memset(tagname, 0, sizeof(tagname));
	strncpy(tagname, tag->str, tag->len);

	if (state->isdescription) {
		ekhtml_attr_t *attr;
		for (attr = attrs; attr; attr = attr->next) {
			printf("ATTRIBUTE: \"%.*s\" = ", (int)attr->name.len,
			       attr->name.str);
			if (!attr->isBoolean) {
				printf("\"%.*s\"\n", (int)attr->val.len, attr->val.str);
			} else {
				printf("\"%.*s\"\n", (int)attr->name.len,
				       attr->name.str);
			}
		}
	}
	if (state->isdescription) {
		char tmp_str[BUFSIZE * 2];
		sprintf(tmp_str, "<%s>", tagname);
		strcat(state->problem_info->description, tmp_str);
	}
}

// 标签结束
static void hdu_endtag(void *cbdata, ekhtml_string_t * str)
{
	//struct html_state_t *state = (struct html_state_t *)cbdata;

	//if (strncmp("DIV", str->str, 3) == 0) {
		//if (tdata->isdescription) {
			//tdata->isdescription--;
		//}
		//printf("END: \"%.*s\"\n", (int)str->len, str->str);
	//}
}

// 处理标签中的数据
static void hdu_data(void *cbdata, ekhtml_string_t * str)
{
	char buf[BUFSIZE];
	memset(buf, 0, sizeof(buf));
	strncpy(buf, str->str, str->len);
	struct html_state_t *state = (struct html_state_t *)cbdata;
	// 获取标题
	if (state->istitle) {
		strncpy(state->problem_info->title, str->str, str->len);
	} else if (state->islimit) {		// 获取限制
		if (strstr(buf, "Time Limit") != NULL) {
			if (DEBUG) {
				printf("限制：%.*s\n", (int)str->len, str->str);
			}
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
	} else if (state->isdescription) {
		strcat(state->problem_info->description, buf);
	}

	if (strcmp("Problem Description", buf) == 0) {
		state->isdescription = 2;
	}
}

int parse_html_hdu(char *buf, struct problem_info_t *problem_info, int type, int pid)
{
	struct html_state_t cbdata;
	memset(&cbdata, 0, sizeof(struct html_state_t));
	cbdata.problem_info = problem_info;
	ekhtml_parser_t *ekparser = prepare_ekhtml(&cbdata);

	// 设置回调函数
	ekhtml_parser_datacb_set(ekparser, hdu_data);
	ekhtml_parser_startcb_add(ekparser, NULL, hdu_starttag);
	ekhtml_parser_startcb_add(ekparser, "H1", hdu_starttag_h1);
	ekhtml_parser_startcb_add(ekparser, "SPAN", hdu_starttag_span);
	ekhtml_parser_endcb_add(ekparser, NULL, hdu_endtag);
	ekhtml_parser_endcb_add(ekparser, "H1", hdu_endtag_h1);
	ekhtml_parser_endcb_add(ekparser, "SPAN", hdu_endtag_span);
	ekhtml_parser_endcb_add(ekparser, "DIV", hdu_endtag_div);

	// 执行解析
	ekhtml_string_t str;
	str.str = buf;
	str.len = strlen(buf);
	ekhtml_parser_feed(ekparser, &str);
	ekhtml_parser_flush(ekparser, 0);

	// 释放解析器资源
	cleanup_ekhtml(ekparser);
	return 0;
}
