/*************************************************************************
	> File Name: get_problem.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时37分44秒
 ************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <mysql/mysql.h>

#include "main.h"

int ojcnt;
wchar_t dbuser[BUFSIZE];
wchar_t dbpasswd[BUFSIZE];
wchar_t dbhost[BUFSIZE];
wchar_t dbname[BUFSIZE];
wchar_t ojstr[OJMAX][BUFSIZE];
wchar_t ojurl[OJMAX][BUFSIZE];

int main(int argc, char *argv[])
{
	if (argc != 4) {
		printf("用法：./get_problem ojname 起始题号 结束题号\n");
		return 0;
	}

	init();

	int i = 0;
	int type = -1;
	int from = atoi(argv[2]);
	int to = atoi(argv[3]);
	wchar_t ojname[BUFSIZE];
	strcpy(ojname, argv[1]);
	for (i = 0; i < ojcnt; ++i) {
		if (strcmp(ojname, ojstr[i]) == 0) {
			type = i;
			break;
		}
	}

	if (type == -1) {
		fprintf(stderr, "%s:没有对应的oj\n", ojname);
		exit(EXIT_FAILURE);
	}

	if (DEBUG) {
		printf("ojname = %s\n", ojname);
		printf("ojurl = %s\n", ojurl[type]);
		printf("type = %d\n", type);
		printf("from = %d\n", from);
		printf("to = %d\n", to);
	}

	CURL *curl = prepare_curl();
	MYSQL *conn = prepare_mysql();

	struct problem_info_t *problem_info = (struct problem_info_t *)malloc(sizeof(struct problem_info_t));
	if (problem_info == NULL) {
		fprintf(stderr, "分配内存失败！\n");
		exit(EXIT_FAILURE);
	}

	int pid = from;
	for (pid = from; pid <= to; ++pid) {
		printf("获取%s题目%d。。。\n", ojname, pid);
		memset(problem_info, 0, sizeof(struct problem_info_t));
		int ret = get_problem(curl, problem_info, type, pid);
		if (ret < 0) {
			printf("获取%s题目%d失败。。。", ojname, pid);
			if (pid != to) {
				printf("两秒钟后获取下一道题目。。。\n");
			} else {
				printf("\n");
			}
		} else {
			printf("获取%s题目%d成功。。。开始将题目添加进数据库。。。\n", ojname, pid);

			ret = add_problem(conn, problem_info);
			if (ret < 0) {
				printf("添加%s题目%d失败。。。\n", ojname, pid);
			} else {
				if (ret != 2) {
					printf("添加%s题目%d成功。。。", ojname, pid);
				}
				execute_cmd("echo %d >> %s", pid, ojname);
			}
			if (pid != to) {
				printf("两秒钟后获取下一道题目。。。\n");
			} else {
				printf("\n");
			}
		}
		sleep(2);
	}


	free(problem_info);
	cleanup_curl(curl);
	cleanup_mysql(conn);

	return 0;
}
