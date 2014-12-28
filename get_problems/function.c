/*************************************************************************
	> File Name: function.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时22分35秒
 ************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <mysql/mysql.h>

#include "main.h"
#include "ekhtml.h"

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
		int http_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if (http_code >= 400) {
			fprintf(stderr, "服务器错误！\n");
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

int load_file(FILE *fp, char *buf)
{
	char tmp[BUFSIZE];
	buf[0] = '\0';
	while (fgets(tmp, BUFSIZE, fp) != NULL) {
		strcat(buf, tmp);
	}
	return 0;
}

int parse_html(char *buf, struct problem_info_t *problem_info, int type, int pid)
{
	// 已知的题目描述
	problem_info->origin_id = pid;
	problem_info->ojtype = type;
	strcpy(problem_info->source, ojstr[type]);

	int ret = -1;
	switch (type) {
		case 0:
			ret = parse_html_hdu(buf, problem_info, type, pid);
			break;
	}
	return ret;
}

ekhtml_parser_t *prepare_ekhtml(void *cbdata)
{
	ekhtml_parser_t *ekparser = ekhtml_parser_new(NULL);
	ekhtml_parser_cbdata_set(ekparser, cbdata);
	return ekparser;
}

void cleanup_ekhtml(ekhtml_parser_t *ekparser)
{
	ekhtml_parser_flush(ekparser, 1);
	ekhtml_parser_destroy(ekparser);
}

int get_problem(CURL *curl, struct problem_info_t *problem_info, int type, int pid)
{
	FILE *fp = NULL;
	fp = get_file(curl, type, pid);
	if (fp == NULL) {
		fprintf(stderr, "获取文件失败！\n");
		return -1;
	}
	char *buf = (char *)malloc(BUFSIZE * BUFSIZE);		// 1M
	if (buf == NULL) {
		fprintf(stderr, "分配内存失败！\n");
		return -1;
	}

	rewind(fp);
	load_file(fp, buf);
	int ret = parse_html(buf, problem_info, type, pid);
	if (ret < 0) {
		fprintf(stderr, "解析html失败！\n");
		return -1;
	}

	if (DEBUG) {
		printf("原题编号：%d\n", problem_info->origin_id);
		printf("题目标题：%s\n", problem_info->title);
		printf("题目描述：%s\n", problem_info->description);
		printf("输入说明：%s\n", problem_info->input);
		printf("输出说明：%s\n", problem_info->output);
		printf("样例输入：%s\n", problem_info->sample_input);
		printf("样例输出：%s\n", problem_info->sample_output);
		printf("题目提示：%s\n", problem_info->hint);
		printf("题目来源：%s\n", problem_info->source);
		printf("时间限制：%d秒\n", problem_info->time_limit);
		printf("内存限制：%d兆\n", problem_info->memory_limit);
		printf("OJ类型：%d\n", problem_info->ojtype);
	}

	fclose(fp);
	execute_cmd("rm -f %d", pid);
	return ret;
}

MYSQL *prepare_mysql(MYSQL *conn)
{
	return NULL;
}

void cleanup_mysql(MYSQL *conn)
{
}

int add_problem(MYSQL *conn, struct problem_info_t *problem_info)
{
	return 0;
}
