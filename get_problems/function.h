/*************************************************************************
	> File Name: function.h
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时15分48秒
 ************************************************************************/

#ifndef _FUNCTION_H
#define _FUNCTION_H

extern void init(void);
extern CURL *prepare_curl(void);
extern void cleanup_curl(CURL *curl);
extern int preform_curl(CURL *curl);
extern void cleanup_mysql(MYSQL *conn);
extern MYSQL *prepare_mysql(MYSQL *conn);
extern int execute_cmd(const char * fmt, ...);
extern FILE *get_file(CURL *curl, int type, int pid);
extern int add_problem(MYSQL *conn, struct problem_info_t *problem_info);
extern size_t save_data(void *buffer, size_t size, size_t nmenb, void *userp);
extern void parse_html(FILE *fp, struct problem_info_t *problem_info, int type, int pid);
extern int get_problem(CURL *curl, struct problem_info_t *problem_info, int type, int pid);
extern void parse_html_hdu(FILE *fp, struct problem_info_t *problem_info, int type, int pid);

#endif	// _FUNCTION_H
