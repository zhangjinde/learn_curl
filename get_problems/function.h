/*************************************************************************
	> File Name: function.h
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时15分48秒
 ************************************************************************/

#ifndef _FUNCTION_H
#define _FUNCTION_H

void init(void);
CURL *prepare_curl(void);
void cleanup_curl(CURL *curl);
int preform_curl(CURL *curl);
int execute_cmd(const char * fmt, ...);
FILE *get_file(CURL *curl, int type, int pid);
int add_problem(struct problem_info_t *problem_info);
size_t save_data(void *buffer, size_t size, size_t nmenb, void *userp);
void parse_html(FILE *fp, struct problem_info_t *problem_info, int type, int pid);
int get_problem(CURL *curl, struct problem_info_t *problem_info, int type, int pid);

#endif	// _FUNCTION_H
