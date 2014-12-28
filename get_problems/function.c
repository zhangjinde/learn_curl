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
extern char dbuser[BUFSIZE];
extern char dbpasswd[BUFSIZE];
extern char dbhost[BUFSIZE];
extern char dbname[BUFSIZE];
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

	fp = fopen("db.conf", "r");
	if (fp == NULL) {
		fprintf(stderr, "打开文件db.conf失败！\n");
		exit(EXIT_FAILURE);
	}
	fscanf(fp, "%s%s%s%s", dbuser, dbpasswd, dbhost, dbname);
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
		fprintf(stderr, "初始化curl失败！\n");
		exit(EXIT_FAILURE);
	}
	CURL *curl = curl_easy_init();
	if (curl == NULL) {
		curl_global_cleanup();
		fprintf(stderr, "获取curl对象失败！\n");
		exit(EXIT_FAILURE);
	}
	return curl;
}

int perform_curl(CURL *curl)
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

	int http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	if (DEBUG) {
		printf("http_code = %d\n", http_code);
	}
	if (http_code >= 400) {
		fprintf(stderr, "服务器错误！\n");
		return -1;
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
	if (perform_curl(curl) < 0) {
		fclose(fp);
		fp = NULL;
		if (!DEBUG) {
			execute_cmd("rm -f %s", pid);
		}
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
	char *buf = (char *)malloc(BUFSIZE * BUFSIZE);
	if (buf == NULL) {
		fprintf(stderr, "分配内存失败！\n");
		return -1;
	}

	rewind(fp);
	load_file(fp, buf);
	printf("nihao\n");
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
	if (!DEBUG) {
		execute_cmd("rm -f %d", pid);
	}
	return ret;
}

MYSQL *prepare_mysql(void)
{
	MYSQL *conn = mysql_init(NULL);
	if (conn == NULL) {
		fprintf(stderr, "初始化数据库失败！:%s\n", mysql_error(conn));
		exit(EXIT_FAILURE);
	}
	unsigned int timeout = 7;
	int ret = mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT,
			(const char *)&timeout);
	if (ret) {
		fprintf(stderr, "设置数据库连接超时失败！:%s\n", mysql_error(conn));
		exit(EXIT_FAILURE);
	}
	conn = mysql_real_connect(conn, dbhost, dbuser, dbpasswd,
			dbname, 0, NULL, 0);
	if (conn == NULL) {
		fprintf(stderr, "连接数据库失败！:%s\n", mysql_error(conn));
		exit(EXIT_FAILURE);
	}
	return conn;
}

void cleanup_mysql(MYSQL *conn)
{
	if (conn != NULL) {
		mysql_close(conn);
	}
}

int add_problem(MYSQL *conn, struct problem_info_t *problem_info)
{
	char *sql = (char *)malloc(BUFSIZE * BUFSIZE);
	char *tmp_str = (char *)malloc(BUFSIZE * BUFSIZE);
	char *end;
	if (sql == NULL || tmp_str == NULL) {
		fprintf(stderr, "分配内存失败！\n");
		return -1;
	}

	MYSQL_RES *result = mysql_store_result(conn);
	sprintf(sql, "SELECT max(problem_id) from problem");
	if (mysql_real_query(conn, sql, strlen(sql))) {
		fprintf(stderr, "sql语句执行失败！:%s\n", mysql_error(conn));
		free(sql);
		free(tmp_str);
		return -1;
	}
	result = mysql_store_result(conn);
	if (result == NULL) {
		fprintf(stderr, "读取数据失败！:%s\n", mysql_error(conn));
		free(sql);
		free(tmp_str);
		return -1;
	}
	my_ulonglong cnt = mysql_num_rows(result);
	if (cnt) {
		MYSQL_ROW row = mysql_fetch_row(result);
		if (row == NULL) {
			free(sql);
			free(tmp_str);
			fprintf(stderr, "获取数据失败！:%s\n", mysql_error(conn));
			return -1;
		}
		problem_info->problem_id = atoi(row[0]);
	} else {
		problem_info->problem_id = 1000;
	}

	sprintf(sql, "SELECT problem_id from vjudge where problem_id='%d' and ojtype='%d'",
			problem_info->origin_id, problem_info->ojtype);
	if (DEBUG) {
		printf("sql = %s\n", sql);
	}
	if (mysql_real_query(conn, sql, strlen(sql))) {
		fprintf(stderr, "sql语句执行失败！:%s\n", mysql_error(conn));
		free(sql);
		free(tmp_str);
		return -1;
	}

	result = mysql_store_result(conn);
	if (result == NULL) {
		fprintf(stderr, "读取数据失败！:%s\n", mysql_error(conn));
		free(sql);
		free(tmp_str);
		return -1;
	}

	cnt = mysql_num_rows(result);
	mysql_free_result(result);
	if (cnt) {
		printf("%s题目%d已经存在！\n", problem_info->source,
				problem_info->origin_id);
		free(sql);
		free(tmp_str);
		return 2;
	}

	if (mysql_autocommit(conn, 0)) {
		fprintf(stderr, "设置手动提交失败！:%s\n", mysql_error(conn));
		free(sql);
		free(tmp_str);
		return -1;
	}
	end = sql;
	strcpy(sql, "INSERT INTO problem (problem_id, title, description, "
			"input, output, sample_input, sample_output, hint, "
			"source, time_limit, memory_limit) values(");
	end += strlen(sql);
	*end++ = '\'';
	end += sprintf(end, "%d", problem_info->problem_id);
	*end++ = '\'';
	*end++ = ',';
	sprintf(tmp_str, "%s", problem_info->title);
	*end++ = '\'';
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	*end++ = ',';
	sprintf(tmp_str, "%s", problem_info->description);
	*end++ = '\'';
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	*end++ = ',';
	sprintf(tmp_str, "%s", problem_info->input);
	*end++ = '\'';
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	*end++ = ',';
	sprintf(tmp_str, "%s", problem_info->output);
	*end++ = '\'';
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	*end++ = ',';
	sprintf(tmp_str, "%s", problem_info->sample_input);
	*end++ = '\'';
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	*end++ = ',';
	sprintf(tmp_str, "%s", problem_info->sample_output);
	*end++ = '\'';
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	*end++ = ',';
	sprintf(tmp_str, "%s", problem_info->hint);
	*end++ = '\'';
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	*end++ = ',';
	sprintf(tmp_str, "%s", problem_info->source);
	*end++ = '\'';
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	*end++ = ',';
	sprintf(tmp_str, "%d", problem_info->time_limit);
	*end++ = '\'';
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	*end++ = ',';
	sprintf(tmp_str, "%d", problem_info->memory_limit);
	*end++ = '\'';
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	*end++ = ')';
	*end = '\0';

	if (DEBUG) {
		printf("sql = %s\n", sql);
	}
	mysql_real_query(conn, sql, (unsigned int)(end - sql));
	sprintf(sql, "INSERT INTO vjudge (problem_id, origin_id, ojtype) "
			"values('%d', '%d', '%d')", problem_info->problem_id,
			problem_info->origin_id, problem_info->ojtype);
	mysql_real_query(conn, sql, strlen(sql));
	if (mysql_commit(conn)) {
		fprintf(stderr, "事务执行失败！:%s\n", mysql_error(conn));
		mysql_rollback(conn);
		free(sql);
		free(tmp_str);
		return -1;
	}
	if (mysql_autocommit(conn, 1)) {
		fprintf(stderr, "设置自动提交失败！:%s\n", mysql_error(conn));
	}
	free(sql);
	free(tmp_str);

	return 0;
}
