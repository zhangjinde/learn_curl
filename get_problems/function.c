/*************************************************************************
	> File Name: function.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时22分35秒
 ************************************************************************/

#include "main.h"

extern int ojcnt;
extern char ojstr[OJMAX][BUFSIZE];
extern char ojurl[OJMAX][BUFSIZE];

/*
 * 回调函数
 * buffer，接收到的数据所在缓冲区
 * size，数据长度
 * nmenb，数据片数量
 * user_p，用户自定义指针
 */
size_t save_data(void *buffer, size_t size, size_t nmenb, void *userp)
{
	FILE *fp = (FILE *)userp;
	size_t ret = fwrite(buffer, size, nmenb, fp);
	if (DEBUG) {
		printf("%s\n", (char *)buffer);
	}
	return ret;
}

void init(void)
{
	FILE *fp = fopen("ojstr.conf", "r");
	if (fp == NULL) {
		fprintf(stderr, "打开文件ojstr.conf失败！\n");
		exit(EXIT_FAILURE);
	}
	int type;
	int cnt = 0;
	while (fscanf(fp, "%d%s", &type, ojstr[cnt]) != EOF) {
		++cnt;
	}
	fclose(fp);
	fp = fopen("ojurl.conf", "r");
	if (fp == NULL) {
		fprintf(stderr, "打开文件ojurl.conf失败！\n");
		exit(EXIT_FAILURE);
	}
	cnt = 0;
	while (fscanf(fp, "%d%s", &type, ojurl[cnt]) != EOF) {
		++cnt;
	}
	ojcnt = cnt;
	fclose(fp);
}

int execute_cmd(const char * fmt, ...) {
	char cmd[BUFSIZE];

	int ret = 0;
	va_list ap;

	va_start(ap, fmt);
	vsprintf(cmd, fmt, ap);
	ret = system(cmd);
	va_end(ap);
	return ret;
}

CURL *prepare_curl(void)
{
	CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
	if (ret != CURLE_OK) {
		return NULL;
	}
	CURL *curl = curl_easy_init();
	if (curl == NULL) {
		curl_global_cleanup();
	}
	return curl;
}

int preform_curl(CURL *curl)
{
	CURLcode ret = curl_easy_perform(curl);
	if (ret != CURLE_OK) {
		printf("执行失败！5秒后重试。。。\n");
		sleep(5);
		ret = curl_easy_perform(curl);
		if (ret != CURLE_OK) {
			fprintf(stderr, "执行失败！\n");
			return -1;
		}
	}

	return 0;
}

void cleanup_curl(CURL *curl)
{
	// 释放资源
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

FILE *get_file(CURL *curl, int type, int pid)
{
	char tmp_url[BUFSIZE];
	char tmp_filename[BUFSIZE];
	sprintf(tmp_url, "%s%d", ojurl[type], pid);
	sprintf(tmp_filename, "%d", pid);
	FILE *fp = fopen(tmp_filename, "w+");

	// 设置网址
	curl_easy_setopt(curl, CURLOPT_URL, tmp_url);
	// 设置回调函数
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_data);
	// 回调函数参数
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

	if (DEBUG) {
		// 输出通信过程中的一些细节，这可以用来调试
		// 如果使用的是http协议，请求头/响应头也会被输出
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	}

	// 执行数据请求
	if (preform_curl(curl) < 0) {
		fclose(fp);
		fp = NULL;
		execute_cmd("rm -f %s", pid);
	}

	return fp;
}

int add_problem(struct problem_info_t *problem_info)
{
	return 0;
}
