/*************************************************************************
	> File Name: function.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2015年01月06日 星期二 22时03分37秒
 ************************************************************************/

#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <mysql/mysql.h>

#include "ekhtml.h"

#define DEBUG 1
#define BUFSIZE 512

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
	char sample_input[BUFSIZE];	// 样例输入
	char sample_output[BUFSIZE];	// 样例输出
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
 * 记录一次提交的信息
 */
struct solution_t {
	int solution_id;		// 运行id
	// 题目信息
	struct problem_info_t problem_info;
	char user_id[BUFSIZE];		// 用户id
	int time;			// 用时（秒）
	int memory;			// 所用空间
	int result;			// 结果（4：AC）
	char in_date[BUFSIZE];		// 加入时间
	int language;			// 语言
	char ip[BUFSIZE];		// 用户ip
	int contest_id;			// 所属比赛id
	int valid;			// 是否有效？
	int num;			// 题目在比赛中的序号
	int code_length;		// 代码长度
	char judgetime[BUFSIZE];	// 判题时间
	double pass_rate;		// 通过百分比（OI模式下可用）
	char src[BUFSIZE * BUFSIZE];	// 源代码
	// 运行错误信息
	int isre;
	char runtimeinfo[BUFSIZE * BUFSIZE];
	// 编译错误信息
	int isce;
	char compileinfo[BUFSIZE * BUFSIZE];
};

char dbhost[] = "127.0.0.1";
char dbuser[] = "root";
char dbpasswd[] = "1234";
char dbname[] = "jol";

int hex2dec(char c)
{
	if ('0' <= c && c <= '9') {
		return c - '0';
	} else if ('a' <= c && c <= 'f') {
		return c - 'a' + 10;
	} else if ('A' <= c && c <= 'F') {
		return c - 'A' + 10;
	} else {
		return -1;
	}
}

char dec2hex(short int c)
{
	if (0 <= c && c <= 9) {
		return c + '0';
	} else if (10 <= c && c <= 15) {
		return c + 'A' - 10;
	} else {
		return -1;
	}
}

/*
 * 编码一个url
 */
void urlencode(char url[])
{
	int i = 0;
	int len = strlen(url);
	int res_len = 0;
	size_t sz = BUFSIZE * BUFSIZE;
	char *res = (char *)malloc(sz);
	if (res == NULL) {
		fprintf(stderr, "分配内存失败！\n");
		return;
	}
	for (i = 0; i < len; ++i) {
		char c = url[i];
		if (('0' <= c && c <= '9') ||
				('a' <= c && c <= 'z') ||
				('A' <= c && c <= 'Z') || c == '/' || c == '.') {
			res[res_len++] = c;
		} else {
			int j = (short int)c;
			if (j < 0)
				j += 256;
			int i1, i0;
			i1 = j / 16;
			i0 = j - i1 * 16;
			res[res_len++] = '%';
			res[res_len++] = dec2hex(i1);
			res[res_len++] = dec2hex(i0);
		}
	}
	res[res_len] = '\0';
	strcpy(url, res);
}

/*
 * 解码url
 */
void urldecode(char url[])
{
	int i = 0;
	int len = strlen(url);
	int res_len = 0;
	size_t sz = BUFSIZE * BUFSIZE;
	char *res = (char *)malloc(sz);
	if (res == NULL) {
		fprintf(stderr, "分配内存失败！\n");
		return;
	}
	for (i = 0; i < len; ++i) {
		char c = url[i];
		if (c != '%') {
			res[res_len++] = c;
		} else {
			char c1 = url[++i];
			char c0 = url[++i];
			int num = 0;
			num = hex2dec(c1) * 16 + hex2dec(c0);
			res[res_len++] = num;
		}
	}
	res[res_len] = '\0';
	strcpy(url, res);
}

int convert(char *buf, size_t len, const char *from, const char *to)
{
	iconv_t cd = iconv_open(to, from);
	if (cd == (iconv_t)-1) {
		perror("获取字符转换描述符失败！\n");
		return -1;
	}
	size_t sz = BUFSIZE * BUFSIZE;
	char *tmp_str = (char *)malloc(sz);
	if (tmp_str == NULL) {
		iconv_close(cd);
		fprintf(stderr, "分配内存失败！\n");
		return -1;
	}
	// 传进去的一定得是别的东西，原来的地址不能被改变
	char *in = buf;
	char *out = tmp_str;
	size_t inlen = len;
	size_t outlen = sz;
	memset(tmp_str, 0, sz);
	if (iconv(cd, &in, &inlen, &out, &outlen) == (size_t)-1) {
		iconv_close(cd);
		free(tmp_str);
		return -1;
	}
	iconv_close(cd);
	strcpy(buf, tmp_str);
	free(tmp_str);
	return 0;
}

int utf2gbk(char *buf, size_t len)
{
	return convert(buf, len, "UTF-8", "GBK");
}

int gbk2utf8(char *buf, size_t len)
{
	return convert(buf, len, "GBK", "UTF-8");
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
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 120);
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

int get_problem_info(MYSQL *conn, struct problem_info_t *problem_info)
{
	char sql[BUFSIZE];
	MYSQL_RES *result;
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select spj, time_limit, memory_limit, accepted, submit, "
			"solved from problem where problem_id = %d",
			problem_info->problem_id);
	if (mysql_real_query(conn, sql, strlen(sql))) {
		fprintf(stderr, "sql语句执行失败！:%s\n", mysql_error(conn));
		return -1;
	}
	result = mysql_store_result(conn);
	if (result == NULL) {
		fprintf(stderr, "读取数据失败！:%s\n", mysql_error(conn));
		return -1;
	}
	my_ulonglong cnt = mysql_num_rows(result);
	if (cnt > 0) {
		MYSQL_ROW row = mysql_fetch_row(result);
		if (row == NULL) {
			fprintf(stderr, "获取数据失败！:%s\n", mysql_error(conn));
			return -1;
		}
		problem_info->spj = atoi(row[0]);
		problem_info->time_limit = atoi(row[1]);
		problem_info->memory_limit = atoi(row[2]);
		problem_info->accepted = atoi(row[3]);
		problem_info->submit = atoi(row[4]);
		problem_info->solved = atoi(row[5]);
		int i = 0;
		int field_cnt = mysql_field_count(conn);
		for (i = 0; i < field_cnt; ++i) {
			printf("row[%d] = %s\n", i, row[i]);
		}
	} else {
			fprintf(stderr, "没有数据！\n");
			return -1;
	}
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select ojtype, origin_id from vjudge where "
			"problem_id = %d", problem_info->problem_id);
	if (mysql_real_query(conn, sql, strlen(sql))) {
		fprintf(stderr, "sql语句执行失败！:%s\n", mysql_error(conn));
		return -1;
	}
	result = mysql_store_result(conn);
	if (result == NULL) {
		fprintf(stderr, "读取数据失败！:%s\n", mysql_error(conn));
		return -1;
	}
	cnt = mysql_num_rows(result);
	if (cnt > 0) {
		MYSQL_ROW row = mysql_fetch_row(result);
		if (row == NULL) {
			fprintf(stderr, "获取数据失败！:%s\n", mysql_error(conn));
			return -1;
		}
		problem_info->ojtype = atoi(row[0]);
		problem_info->origin_id = atoi(row[1]);
		int i = 0;
		int field_cnt = mysql_field_count(conn);
		for (i = 0; i < field_cnt; ++i) {
			printf("row[%d] = %s\n", i, row[i]);
		}
	} else {
			fprintf(stderr, "没有数据！\n");
			return -1;
	}
	return 0;
}

struct solution_t *get_solution(MYSQL *conn, int sid)
{
	struct solution_t *solution = (struct solution_t *)malloc(sizeof(struct solution_t));
	if (solution == NULL) {
		fprintf(stderr, "分配内存失败！\n");
		return NULL;
	}
	char sql[BUFSIZE];
	int pid = 1000;
	memset(solution, 0, sizeof(struct solution_t));
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "select solution.solution_id, problem_id, user_id, time, memory, "
			"in_date, result, language, ip, contest_id, valid, "
			"num, code_length, judgetime, pass_rate, source from solution,source_code "
			"where solution.solution_id = source_code.solution_id and solution.solution_id = %d", sid);
	if (mysql_real_query(conn, sql, strlen(sql))) {
		fprintf(stderr, "sql语句执行失败！:%s\n", mysql_error(conn));
		free(solution);
		return NULL;
	}
	MYSQL_RES *result = mysql_store_result(conn);
	if (result == NULL) {
		fprintf(stderr, "读取数据失败！:%s\n", mysql_error(conn));
		free(solution);
		return NULL;
	}
	my_ulonglong cnt = mysql_num_rows(result);
	if (cnt > 0) {
		MYSQL_ROW row = mysql_fetch_row(result);
		if (row == NULL) {
			free(solution);
			fprintf(stderr, "获取数据失败！:%s\n", mysql_error(conn));
			return NULL;
		}
		solution->solution_id = sid;
		solution->problem_info.problem_id = atoi(row[1]);
		strcpy(solution->user_id, row[2]);
		solution->time = atoi(row[3]);
		solution->memory = atoi(row[4]);
		solution->result = atoi(row[5]);
		strcpy(solution->in_date, row[6]);
		solution->language = atoi(row[7]);
		strcpy(solution->ip, row[8]);
		if (row[9] != NULL) {
			solution->contest_id = atoi(row[9]);
		} else {
			solution->contest_id = -1;
		}
		solution->valid = atoi(row[10]);
		solution->num = atoi(row[11]);
		solution->code_length = atoi(row[12]);
		if (row[13] != NULL) {
			strcpy(solution->judgetime, row[13]);
		}
		solution->pass_rate = atof(row[14]);
		strcpy(solution->src, row[15]);
		int i = 0;
		int field_cnt = mysql_field_count(conn);
		for (i = 0; i < field_cnt; ++i) {
			printf("row[%d] = %s\n", i, row[i]);
		}
		get_problem_info(conn, &solution->problem_info);
	} else {
			fprintf(stderr, "没有数据！\n");
			free(solution);
			return NULL;
	}
	return solution;
}

int update_solution(MYSQL *conn, struct solution_t *solution)
{
	char *sql = (char *)malloc(BUFSIZE * BUFSIZE);
	if (sql == NULL) {
		fprintf(stderr, "分配内存失败！\n");
		return -1;
	}
	memset(sql, 0, BUFSIZE * BUFSIZE);
	sprintf(sql, "update solution set time=%d, memory=%d, result=%d, "
			"pass_rate=%f, judgetime=now() where solution_id = %d",
			solution->time, solution->memory, solution->result,
			solution->pass_rate, solution->solution_id);
	if (mysql_real_query(conn, sql, strlen(sql))) {
		fprintf(stderr, "sql语句执行失败！:%s\n", mysql_error(conn));
		free(sql);
		return -1;
	}
	char *end;
	if (solution->isce) {
		memset(sql, 0, BUFSIZE * BUFSIZE);
		sprintf(sql, "delete from compileinfo where solution_id = %d",
				solution->solution_id);
		if (mysql_real_query(conn, sql, strlen(sql))) {
			fprintf(stderr, "sql语句执行失败！:%s\n", mysql_error(conn));
			free(sql);
			return -1;
		}
		memset(sql, 0, BUFSIZE * BUFSIZE);
		strcpy(sql, "insert into compileinfo (solution_id, error) values(");
		end = sql + strlen(sql);
		*end++ = '\'';
		end += sprintf(end, "%d", solution->solution_id);
		*end++ = '\'';
		*end++ = ',';
		*end++ = '\'';
		end += mysql_real_escape_string(conn, end, solution->compileinfo,
				strlen(solution->compileinfo));
		*end++ = '\'';
		*end++ = ')';
		*end++ = '\0';
		if (mysql_real_query(conn, sql, strlen(sql))) {
			fprintf(stderr, "sql语句执行失败！:%s\n", mysql_error(conn));
			free(sql);
			return -1;
		}
	}
	if (solution->isre) {
		memset(sql, 0, BUFSIZE * BUFSIZE);
		sprintf(sql, "delete from runtimeinfo where solution_id = %d",
				solution->solution_id);
		if (mysql_real_query(conn, sql, strlen(sql))) {
			fprintf(stderr, "sql语句执行失败！:%s\n", mysql_error(conn));
			free(sql);
			return -1;
		}
		memset(sql, 0, BUFSIZE * BUFSIZE);
		strcpy(sql, "insert into runtimeinfo (solution_id, error) values(");
		end = sql + strlen(sql);
		*end++ = '\'';
		end += sprintf(end, "%d", solution->solution_id);
		*end++ = '\'';
		*end++ = ',';
		*end++ = '\'';
		end += mysql_real_escape_string(conn, end, solution->runtimeinfo,
				strlen(solution->runtimeinfo));
		*end++ = '\'';
		*end++ = ')';
		*end++ = '\0';
		if (mysql_real_query(conn, sql, strlen(sql))) {
			fprintf(stderr, "sql语句执行失败！:%s\n", mysql_error(conn));
			free(sql);
			return -1;
		}
	}
	return 0;
}

MYSQL *prepare_mysql(void)
{
	MYSQL *conn = mysql_init(NULL);
	if (conn == NULL) {
		fprintf(stderr, "初始化数据库失败！:%s\n", mysql_error(conn));
		exit(EXIT_FAILURE);
	}
	unsigned int timeout = 120;
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
	if (mysql_set_character_set(conn, "utf8")) {
		fprintf(stderr, "设置数据库编码失败！:%s\n", mysql_error(conn));
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

int submit(void)
{
	CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
	if (ret != CURLE_OK) {
		error(EXIT_FAILURE, 0, "初始化curl失败！\n");
	}
	CURL *curl = curl_easy_init();
	if (curl == NULL) {
		curl_global_cleanup();
		error(EXIT_FAILURE, 0, "获取curl失败！\n");
	}

	FILE *fp = fopen("login.html", "w");
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	// 不认证ssl证书
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	// 跟踪重定向的信息
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	// 设置cookie信息，否则就不能保存登陆信息
	// 设置读取cookie的文件名
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "hducookie");
	// 设置写入cookie的文件名
	curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "hducookie");

	// 调试
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_URL, "http://acm.hdu.edu.cn/userloginex.php?action=login");
	// 设置参数
	// 不需要这个login参数
	//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "username=username&userpass=password8&login=Sign+In");
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "username=zzuvjudge&userpass=zzuacmlab");

	// 登陆
	ret = curl_easy_perform(curl);
	if (ret != CURLE_OK) {
		printf("登陆失败!5秒后尝试重新登陆。。。\n");
		sleep(5);
		ret = curl_easy_perform(curl);
		if (ret != CURLE_OK) {
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			fclose(fp);
			error(EXIT_FAILURE, 0, "登陆失败！\n");
		}
	}
	printf("登陆成功。。。5秒后提交\n");

	int ch = 0;
	char src[BUFSIZE];
	char fields[BUFSIZE] = "check=0&problemid=1000&language=2&usercode=";
	int len = 0;
	FILE *fp_src = fopen("src.c", "r");
	if (fp_src == NULL) {
		error(EXIT_FAILURE, 0, "打开文件src.c失败！\n");
	}
	while ((ch = fgetc(fp_src)) != EOF) {
		src[len++] = ch;
	}
	src[len] = '\0';
	fclose(fp_src);

	// 对其进行url编码
	utf2gbk(src, len);
	urlencode(src);
	strcat(fields, src);

	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_URL, "http://acm.hdu.edu.cn/submit.php?action=submit");
	// 设置参数
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields);
	sleep(5);

	fclose(fp);

	fp = fopen("submit.html", "w");
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	// 执行提交
	ret = curl_easy_perform(curl);
	if (ret != CURLE_OK) {
		printf("提交失败！\n");
	} else {
		printf("提交成功！\n");
	}

	// 释放资源
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	fclose(fp);

	return 0;
}

void get_info(int sid)
{
	MYSQL *conn = prepare_mysql();
	struct solution_t *solution = get_solution(conn, sid);
	solution->result = 6;
	update_solution(conn, solution);
	cleanup_mysql(conn);
}

int main(int argc, char *argv[])
{
	if (argc == 2) {
		get_info(atoi(argv[1]));
	}
	return 0;
}
