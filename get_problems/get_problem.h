/*************************************************************************
	> File Name: get_problem.h
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时18分21秒
 ************************************************************************/
#ifndef _MAIN_H
#define _MAIN_H

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <mysql/mysql.h>
#include <iconv.h>
#include <curl/curl.h>
#include <mysql/mysql.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <error.h>
#include <ctype.h>

#include "ekhtml.h"

#define BUFSIZE 512
#define OJMAX 20

/*
 * 题目信息结构体
 */
struct problem_info_t {
	int problem_id;		// 题目编号
	int origin_id;			// 题目在原oj上的题目编号
	char title[BUFSIZE];		// 题目标题
	char description[BUFSIZE * BUFSIZE];	// 题目描述
	char input[BUFSIZE * BUFSIZE];	// 输入说明
	char output[BUFSIZE * BUFSIZE];	// 输出说明
	char sample_input[BUFSIZE * BUFSIZE];	// 样例输入
	char sample_output[BUFSIZE * BUFSIZE];	// 样例输出
	char hint[BUFSIZE * BUFSIZE];	// 提示
	char source[BUFSIZE];		// 题目来源，为抓取题目的ojname
	int time_limit;			// 时限（秒）
	int memory_limit;		// 内存限制（兆）
	int ojtype;			// oj的类型
	int spj;			// 是否是spj
	int accepted;			// 通过的提交次数
	int submit;			// 总提交次数
	int solved;			// 未用
	int defunct;			// 是否屏蔽
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
	int isstat;			// 是否是题目通过统计
	int isspj;			// 是否是题目spj描述
	// 题目信息
	struct problem_info_t *problem_info;
};

extern int DEBUG;
extern int pid;
extern int oj_cnt;
extern int oj_type;
extern int db_port;
extern int sleep_time;
extern int db_timeout;
extern char db_user[BUFSIZE];
extern char db_passwd[BUFSIZE];
extern char db_host[BUFSIZE];
extern char db_name[BUFSIZE];
extern char oj_str[OJMAX][BUFSIZE];
extern char oj_url[OJMAX][BUFSIZE];
extern char oj_name[BUFSIZE];
extern CURL *curl;
extern MYSQL *conn;
extern struct problem_info_t *problem_info;

extern void init_conf(void);
extern int gbk2utf8(char *buf, size_t len);
extern CURL *prepare_curl(void);
extern void cleanup_curl(void);
extern int perform_curl(const char *filename);
extern void cleanup_mysql(void);
extern MYSQL *prepare_mysql(void);
extern ekhtml_parser_t *prepare_ekhtml(void *cbdata);
extern int execute_cmd(const char * fmt, ...);
extern FILE *get_file(CURL *curl, int type, int pid);
extern void cleanup_ekhtml(ekhtml_parser_t *ekparser);
extern int add_problem(void);
extern size_t save_data(void *buffer, size_t size, size_t nmenb, void *userp);
extern int parse_html(char *buf);
extern int get_problem(void);
extern int parse_html_hdu(char *buf);
extern int write_log(const char *fmt, ...);
extern int execute_cmd(const char *fmt, ...);
extern int after_equal(char *c);
extern void trim(char *c);
extern void to_lowercase(char *c);
extern int read_buf(char *buf, const char *key, char *value);
extern void read_int(char *buf, const char *key, int *value);
extern int load_file(const char *filename, char *buf);
extern int save_file(const char *filename, char *buf);
extern int parse_html(char *buf);
extern char *get_problem_url(void);
extern int get_problem(void);
extern int isexist(void);
extern int add_problem(void);
extern void ping(void);
extern int execute_sql(const char *fmt, ...);

#endif	// _MAIN_H
