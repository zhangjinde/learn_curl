/*************************************************************************
	> File Name: function.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时22分35秒
 ************************************************************************/

#include "get_problem.h"

int write_log(const char *fmt, ...)
{
	va_list ap;
	char *buffer = (char *)malloc(BUFSIZE * BUFSIZE);
	if (buffer == NULL) {
		fprintf(stderr, "alloc log buf memory error.\n");
		return 0;
	}
	time_t t = time(NULL);
	struct tm *date = localtime(&t);
	char timestr[BUFSIZE];
	sprintf(timestr, "%s", asctime(date));
	sprintf(buffer, "get_problem%04d%02d%02d.log", date->tm_year + 1900,
			date->tm_mon + 1, date->tm_mday);
	FILE *fp = fopen(buffer, "a+");
	if (fp == NULL) {
		fprintf(stderr, "open log file error:%s.\n", strerror(errno));
		free(buffer);
		return 0;
	}
	if (DEBUG) {
		freopen("/dev/stdout", "w", fp);
	}
	va_start(ap, fmt);
	vsprintf(buffer, fmt, ap);
	int len = strlen(timestr);
	timestr[len - 1] = '\0';
	int ret = fprintf(fp, "[%s]:%s", timestr, buffer);
	va_end(ap);
	fclose(fp);
	free(buffer);
	return ret;
}

int execute_cmd(const char *fmt, ...)
{
	char *cmd = (char *)malloc(BUFSIZE * BUFSIZE);
	if (cmd == NULL) {
		write_log("alloc cmd memory error.\n");
		return 1;
	}
	int ret = 0;
	va_list ap;
	va_start(ap, fmt);
	vsprintf(cmd, fmt, ap);
	write_log("execute cmd: %s.\n", cmd);
	ret = system(cmd);
	va_end(ap);
	free(cmd);
	return ret;
}

int after_equal(char *c)
{
	int i = 0;
	for (; c[i] != '\0' && c[i] != '='; i++) ;
	return ++i;
}

void trim(char *c)
{
	char *buf = (char *)malloc(BUFSIZE * BUFSIZE);
	if (buf == NULL) {
		write_log("alloc trim buf memory error.\n");
		return;
	}
	char *start, *end;
	strcpy(buf, c);
	int len = strlen(buf);
	start = buf;
	while (isspace(*start))
		start++;
	end = start + len - 1;
	while (isspace(*end))
		end--;
	*(end + 1) = '\0';
	strcpy(c, start);
	free(buf);
}

void to_lowercase(char *c)
{
	int i = 0;
	int len = strlen(c);
	for (i = 0; i < len; ++i) {
		c[i] = tolower(c[i]);
	}
}

int read_buf(char *buf, const char *key, char *value)
{
	if (strncmp(buf, key, strlen(key)) == 0) {
		strcpy(value, buf + after_equal(buf));
		trim(value);
		write_log("%s = %s\n", key, value);
		return 1;
	}
	return 0;
}

void read_int(char *buf, const char *key, int *value)
{
	char buf2[10];
	if (read_buf(buf, key, buf2)) {
		sscanf(buf2, "%d", value);
	}
}

int load_file(const char *filename, char *buf)
{
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		write_log("cann't open file %s.\n", filename);
		return -1;
	}
	char *tmp = (char *)malloc(BUFSIZE * BUFSIZE);
	if (tmp == NULL) {
		write_log("alloc load_file buf memory error.\n");
		fclose(fp);
		return -1;
	}
	buf[0] = '\0';
	while (fgets(tmp, BUFSIZE * BUFSIZE, fp) != NULL) {
		strcat(buf, tmp);
	}
	free(tmp);
	fclose(fp);
	return 0;
}

int save_file(const char *filename, char *buf)
{
	FILE *fp = fopen(filename, "w");
	if (fp == NULL) {
		write_log("cann't open file %s.\n", filename);
		return -1;
	}
	int len = strlen(buf);
	fwrite(buf, len, 1, fp);
	fclose(fp);
	return 0;
}

int parse_html(char *buf)
{
	// 已知的题目描述
	problem_info->origin_id = pid;
	problem_info->ojtype = oj_type;
	strcpy(problem_info->source, oj_str[oj_type]);

	write_log("try to parse %s html.\n", oj_name);
	int ret = -1;
	switch (oj_type) {
		case 0:
			ret = parse_html_hdu(buf);
			break;
		case 1:
			ret = parse_html_poj(buf);
			break;
		case 2:
			ret = parse_html_cf(buf);
			break;
		case 3:
			ret = parse_html_zoj(buf);
			break;
	}
	return ret;
}

// memory should be free
char *get_problem_url(void)
{
	char *url = (char *)malloc(BUFSIZE);
	if (url == NULL) {
		write_log("alloc url buf memory error.\n");
		return NULL;
	}
	if (oj_type == 2) {		// codeforces
		sprintf(url, "%s%d/%c", oj_url[oj_type], pid / 10,
				pid % 10 + 'A');
	} else {
		sprintf(url, "%s%d", oj_url[oj_type], pid);
	}
	return url;
}

int get_problem(void)
{
	write_log("try to get %s problem %d.\n", oj_name, pid);

	char *url = get_problem_url();
	char filename[BUFSIZE];
	sprintf(filename, "%d", pid);

	// 设置网址
	curl_easy_setopt(curl, CURLOPT_URL, url);
	
	write_log("url = %s.\n", url);
	// 执行数据请求
	if (perform_curl(filename) < 0) {
		free(url);
		return -1;
	}

	char *buf = (char *)malloc(BUFSIZE * BUFSIZE);
	if (buf == NULL) {
		write_log("alloc get_problem buf memory error.\n");
		free(url);
		return -1;
	}

	load_file(filename, buf);

	// hdu shoule be convert character set
	if (oj_type == 0) {
		if (gbk2utf8(buf, strlen(buf)) < 0) {
			free(buf);
			free(url);
			write_log("convert character set error.\n");
			return -1;
		}
	}

	if (strstr(buf, "No such problem") != NULL
			|| strstr(buf, "Invalid Parameter") != NULL
			|| strstr(buf, "Can not find problem") != NULL) {
		free(buf);
		free(url);
		return 1;
	}
	int ret = parse_html(buf);
	if (ret < 0) {
		free(buf);
		free(url);
		write_log("parse html file error.\n");
		return -1;
	}

	write_log("origin_id = %d\n", problem_info->origin_id);
	write_log("title = %s\n", problem_info->title);
	write_log("description = %s\n", problem_info->description);
	write_log("input = %s\n", problem_info->input);
	write_log("output = %s\n", problem_info->output);
	write_log("sample_input = %s\n", problem_info->sample_input);
	write_log("sample_output = %s\n", problem_info->sample_output);
	write_log("hint = %s\n", problem_info->hint);
	write_log("source = %s\n", problem_info->source);
	write_log("time_limit = %d秒\n", problem_info->time_limit);
	write_log("memory_limit = %d兆\n", problem_info->memory_limit);
	write_log("ojtype = %d\n", problem_info->ojtype);
	write_log("spj = %d\n", problem_info->spj);
	write_log("accepted = %d\n", problem_info->accepted);
	write_log("submit = %d\n", problem_info->submit);
	write_log("solved(unuse) = %d\n", problem_info->solved);
	write_log("defunct = %d\n", problem_info->defunct);

	free(buf);
	free(url);
	return ret;
}

int isexist(void)
{
	write_log("test %s problem %d is or not exist.\n", oj_name, pid);
	if (execute_sql("SELECT problem_id,origin_id from vjudge where origin_id='%d' and ojtype='%d'",
			problem_info->origin_id, problem_info->ojtype) < 0) {
		return -1;
	}
	MYSQL_RES *result = mysql_store_result(conn);
	if (result == NULL) {
		write_log("read mysql result error:%s.\n", mysql_error(conn));
		return -1;
	}
	int old_pid = -1;
	int cnt = mysql_num_rows(result);
	if (cnt > 0) {
		MYSQL_ROW row = mysql_fetch_row(result);
		if (row == NULL) {
			mysql_free_result(result);
			write_log("fetch mysql row error:%s.\n", mysql_error(conn));
			return -1;
		}
		if (row[0] != NULL) {
			old_pid = atoi(row[0]);
		}
	} else {
		old_pid = 0;
	}
	mysql_free_result(result);
	return old_pid;
}

int add_problem(void)
{
	int old_pid = isexist();
	if (old_pid < 0) {
		return -1;
	}

	char *sql = (char *)malloc(BUFSIZE * BUFSIZE);
	if (sql == NULL) {
		write_log("alloc add_problem sql memory error.\n");
		return -1;
	}

	char *tmp_str = (char *)malloc(BUFSIZE * BUFSIZE);
	if (tmp_str == NULL) {
		write_log("alloc add_problem tmp_str memory error.\n");
		free(sql);
		return -1;
	}

	if (old_pid) {
		problem_info->problem_id = old_pid;
	} else {
		if (execute_sql("SELECT max(problem_id) from problem") < 0) {
			free(sql);
			free(tmp_str);
			return -1;
		}
		MYSQL_RES *result = mysql_store_result(conn);
		if (result == NULL) {
			write_log("read mysql result error:%s.\n", mysql_error(conn));
			free(sql);
			free(tmp_str);
			return -1;
		}
		int cnt = mysql_num_rows(result);
		if (cnt > 0) {
			MYSQL_ROW row = mysql_fetch_row(result);
			if (row == NULL) {
				mysql_free_result(result);
				free(sql);
				free(tmp_str);
				write_log("fetch mysql row error:%s.\n", mysql_error(conn));
				return -1;
			}
			if (row[0] == NULL) {
				problem_info->problem_id = 1000;
			} else {
				problem_info->problem_id = atoi(row[0]) + 1;
			}
		} else {
			problem_info->problem_id = 1000;
		}
		mysql_free_result(result);
	}

	char *end = sql;
	strcpy(sql, "INSERT INTO problem (problem_id, title, description, "
			"input, output, sample_input, sample_output, hint, "
			"source, time_limit, memory_limit, spj, accepted, "
			"submit, solved, defunct, in_date) values(");
	end += strlen(sql);
	end += sprintf(end, "'%d',", problem_info->problem_id);
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
	end += sprintf(end, ",'%s'", problem_info->source);
	end += sprintf(end, ",'%d'", problem_info->time_limit);
	end += sprintf(end, ",'%d'", problem_info->memory_limit);
	end += sprintf(end, ",'%d'", problem_info->spj);
	end += sprintf(end, ",'%d'", problem_info->accepted);
	end += sprintf(end, ",'%d'", problem_info->submit);
	end += sprintf(end, ",'%d'", problem_info->solved);
	end += sprintf(end, ",'%c'", problem_info->defunct ? 'Y' : 'N');
	end += sprintf(end, ",now())");
	end += sprintf(end, "%s", " on duplicate key update title='");
	sprintf(tmp_str, "%s", problem_info->title);
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	end += sprintf(end, "%s", ",description='");
	sprintf(tmp_str, "%s", problem_info->description);
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	end += sprintf(end, "%s", ",input='");
	sprintf(tmp_str, "%s", problem_info->input);
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	end += sprintf(end, "%s", ",output='");
	sprintf(tmp_str, "%s", problem_info->output);
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	end += sprintf(end, "%s", ",sample_input='");
	sprintf(tmp_str, "%s", problem_info->sample_input);
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	end += sprintf(end, "%s", ",sample_output='");
	sprintf(tmp_str, "%s", problem_info->sample_output);
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	end += sprintf(end, "%s", ",hint='");
	sprintf(tmp_str, "%s", problem_info->hint);
	end += mysql_real_escape_string(conn, end, tmp_str, strlen(tmp_str));
	*end++ = '\'';
	end += sprintf(end, ",source='%s'", problem_info->source);
	end += sprintf(end, ",time_limit='%d'", problem_info->time_limit);
	end += sprintf(end, ",memory_limit='%d'", problem_info->memory_limit);
	end += sprintf(end, ",spj='%d'", problem_info->spj);
	end += sprintf(end, ",accepted='%d'", problem_info->accepted);
	end += sprintf(end, ",submit='%d'", problem_info->submit);
	end += sprintf(end, ",solved='%d'", problem_info->solved);
	end += sprintf(end, ",defunct='%c'", problem_info->defunct ? 'Y' : 'N');
	end += sprintf(end, ",in_date=now()");
	*end = '\0';

	if (mysql_real_query(conn, sql, strlen(sql))) {
		write_log("execute sql error:%s.\n", mysql_error(conn));
		free(sql);
		free(tmp_str);
		return -1;
	}

	if (!old_pid) {
		if (execute_sql("INSERT INTO vjudge (problem_id, origin_id, ojtype) "
				"values('%d', '%d', '%d')", problem_info->problem_id,
				problem_info->origin_id, problem_info->ojtype) < 0) {
			free(sql);
			free(tmp_str);
			return -1;
		}
		if (execute_sql("INSERT INTO cha (problem_id) values('%d')",
				problem_info->problem_id) < 0) { 
			free(sql);
			free(tmp_str);
			return -1;
		}
	}

	free(sql);
	free(tmp_str);
	return old_pid;
}
