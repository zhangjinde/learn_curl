/*************************************************************************
	> File Name: main.h
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时18分21秒
 ************************************************************************/
#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>

#define DEBUG 1
#define BUFSIZE 1024
#define OJMAX 20

/*
 * 题目信息结构体
 */
struct problem_info_t {
	int proble_id;			// 题目编号
	int origin_id;			// 题目在原oj上的题目编号
	char title[BUFSIZE];		// 题目标题
	char description[2 * BUFSIZE];	// 题目描述
	char input[2 * BUFSIZE];	// 输入说明
	char output[2 * BUFSIZE];	// 输出说明
	char sample_input[BUFSIZE];	// 样例输入
	char sample_output[BUFSIZE];	// 样例输出
	char hint[BUFSIZE];		// 提示
	char source[BUFSIZE];		// 题目来源
	int time_limit;			// 时限（秒）
	int memory_limit;		// 内存限制（兆）
	char ojtype;			// oj的类型
};

typedef struct {
	unsigned int n_starttags;
	unsigned int n_endtags;
	unsigned int n_comments;
	unsigned int n_data;
	unsigned int magic_doodie;
	unsigned int only_parse;
	unsigned int ish1;
	unsigned int isdescription;
} tester_cbdata;

extern void init(void);
extern CURL *prepare_curl(void);
extern void cleanup_curl(CURL *curl);
extern int preform_curl(CURL *curl);
extern void cleanup_mysql(MYSQL *conn);
extern MYSQL *prepare_mysql(MYSQL *conn);
extern int load_file(FILE *fp, char *buf);
extern ekhtml_parser_t *prepare_ekhtml(void);
extern int execute_cmd(const char * fmt, ...);
extern FILE *get_file(CURL *curl, int type, int pid);
extern void cleanup_ekhtml(ekhtml_parser_t *ekparser);
extern int add_problem(MYSQL *conn, struct problem_info_t *problem_info);
extern size_t save_data(void *buffer, size_t size, size_t nmenb, void *userp);
extern int parse_html(char *buf, struct problem_info_t *problem_info, int type, int pid);
extern int get_problem(CURL *curl, struct problem_info_t *problem_info, int type, int pid);
extern int parse_html_hdu(char *buf, struct problem_info_t *problem_info, int pid);

#endif	// _MAIN_H
