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
static void hdu_starttag_b(void *cbdata, ekhtml_string_t * tag,
				ekhtml_attr_t * attrs)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->islimit = 1;
}

// 题目限制结束
static void hdu_endtag_b(void *cbdata, ekhtml_string_t * str)
{
	struct html_state_t *state = (struct html_state_t *)cbdata;
	state->islimit = 0;
}

// 标签开始
static void hdu_starttag(void *cbdata, ekhtml_string_t * tag,
			    ekhtml_attr_t * attrs)
{
	//ekhtml_attr_t *attr;
	//struct html_state_t *state = (struct html_state_t *)cbdata;

	//if (strncmp("DIV", tag->str, 3) == 0) {
		//if (!tdata->isdescription) {
			//return;
		//}
		//printf("START: \"%.*s\"\n", (int)tag->len, tag->str);
		//for (attr = attrs; attr; attr = attr->next) {
			//if (strncmp("style", attr->name.str, 5) == 0) {
				//printf("ATTRIBUTE: \"%.*s\" = ", (int)attr->name.len,
				       //attr->name.str);
				//if (!attr->isBoolean) {
					//printf("\"%.*s\"\n", (int)attr->val.len, attr->val.str);
				//} else {
					//printf("\"%.*s\"\n", (int)attr->name.len,
					       //attr->name.str);
				//}
			//}
		//}
	//}
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
	struct html_state_t *state = (struct html_state_t *)cbdata;
	// 获取标题
	if (state->istitle) {
		strncpy(state->problem_info->title, str->str, str->len);
	} else if (state->islimit) {
		printf("限制：%.*s\n", (int)str->len, str->str);
	}

	//if (strncmp("Problem Description", str->str, 19) == 0) {
		//tdata->isdescription = 2;
	//}
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
	ekhtml_parser_startcb_add(ekparser, "B", hdu_starttag_b);
	ekhtml_parser_endcb_add(ekparser, NULL, hdu_endtag);
	ekhtml_parser_endcb_add(ekparser, "H1", hdu_endtag_h1);
	ekhtml_parser_endcb_add(ekparser, "B", hdu_endtag_b);

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
