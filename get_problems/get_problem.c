/*************************************************************************
	> File Name: get_problem.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时37分44秒
 ************************************************************************/

#include "main.h"

int main(int argc, char *argv[])
{
	if (argc ! = 4) {
		printf("用法：./get_problem ojname 起始题号 结束题号\n");
		return 0;
	}

	init();

	int i = 0;
	int type = -1;
	int from = atoi(argv[2]);
	int to = atoi(argv[3]);
	char str[BUFSIZE];
	strcpy(str, argv[1]);
	for (i = 0; i < ojcnt; ++i) {
		if (strcmp(str, ojstr[i]) == 0) {
			type = i;
			break;
		}
	}

	if (type == -1) {
		fprintf(stderr, "%s:没有对应的oj\n", str);
		exit(EXIT_FAILURE);
	}

	CURL *curl = prepare_curl();
	if (curl == NULL) {
		fprintf(stderr, "初始化curl失败！");
		exit(EXIT_FAILURE);
	}

	struct problem_info_t *problem_info = (struct problem_info_t *)malloc(sizeof(struct problem_info_t));
	if (problem_info == NULL) {
		fprintf(strerr, "分配内存失败！\n");
		exit(EXIT_FAILURE);
	}

	int pid = from;
	for (pid = from, pid <= to; ++pid) {
		switch (type) {
			case 0:
				get_problem_hdu(curl, problem_info, pid);
				break;
		}

		add_problem(problem_info);
		sleep(5);
	}


	free(problem_info);
	cleanup_curl(curl);

	return 0;
}
