/*************************************************************************
	> File Name: function.h
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时15分48秒
 ************************************************************************/

#ifndef _FUNCTION_H
#define _FUNCTION_H

void preform_curl(CURL *curl);
FILE *get_file(CURL *curl, const char url[], int pid);
size_t save_data(void *buffer, size_t size, size_t nmenb, void *userp);
void get_problem_hdu(CURL *curl, struct problem_info_t *problem_info, int pid);

#endif	// _FUNCTION_H
