/*************************************************************************
	> File Name: main.h
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时18分21秒
 ************************************************************************/
#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>

#include "ekhtml.h"

#define DEBUG 0
#define BUFSIZE 512
#define OJMAX 20

/*
 * 题目信息结构体
 */
struct problem_info_t {
	int problem_id;		// 题目编号
	int origin_id;			// 题目在原oj上的题目编号
	char title[BUFSIZE];		// 题目标题
	char description[2 * BUFSIZE];	// 题目描述
	char input[2 * BUFSIZE];	// 输入说明
	char output[2 * BUFSIZE];	// 输出说明
	char sample_input[BUFSIZE];	// 样例输入
	char sample_output[BUFSIZE];	// 样例输出
	char hint[BUFSIZE];		// 提示
	char source[BUFSIZE];		// 题目来源，为抓取题目的ojname
	int time_limit;			// 时限（秒）
	int memory_limit;		// 内存限制（兆）
	int ojtype;			// oj的类型
};

/*
 * 记录html中的状态，从而获得题目描述
 */
struct html_state_t {
	int isdescription;		// 是否是题目描述
	int istitle;			// 是否是题目标题
	int isinput;			// 是否是题目输入说明
	int isoutput;			// 是否是题目输出说明
	int issinput;			// 是否是题目样例输入
	int issoutput;			// 是否是题目样例输出
	int ishint;			// 是否是题目提示
	int islimit;			// 是否是题目限制
	// 题目信息
	struct problem_info_t *problem_info;
};

extern void init(void);
int gbk2utf8(char *buf, size_t len);
extern CURL *prepare_curl(void);
extern void cleanup_curl(CURL *curl);
extern int perform_curl(CURL *curl);
extern void cleanup_mysql(MYSQL *conn);
extern MYSQL *prepare_mysql(void);
extern int load_file(FILE *fp, char *buf);
extern ekhtml_parser_t *prepare_ekhtml(void *cbdata);
extern int execute_cmd(const char * fmt, ...);
extern FILE *get_file(CURL *curl, int type, int pid);
extern void cleanup_ekhtml(ekhtml_parser_t *ekparser);
extern int add_problem(MYSQL *conn, struct problem_info_t *problem_info);
extern size_t save_data(void *buffer, size_t size, size_t nmenb, void *userp);
extern int parse_html(char *buf, struct problem_info_t *problem_info, int type, int pid);
extern int get_problem(CURL *curl, struct problem_info_t *problem_info, int type, int pid);
extern int parse_html_hdu(char *buf, struct problem_info_t *problem_info, int type, int pid);

#endif	// _MAIN_H
