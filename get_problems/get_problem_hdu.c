/*************************************************************************
	> File Name: get_problem_hdu.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时02分35秒
 ************************************************************************/

/*
 * 获取杭电的题目描述，并将其插入到数据库中
 */
#include "main.h"
#include "function.h"

extern int ojcnt;
extern char ojstr[OJMAX][BUFSIZE];
extern char ojurl[OJMAX][BUFSIZE];

void get_problem_hdu(CURL *curl, struct problem_info_t *problem_info, int type, int pid)
{
	FILE *fp = NULL;
	fp = get_file(curl, type, pid);
	fclose(fp);
}
