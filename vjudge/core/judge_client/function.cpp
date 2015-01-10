#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mysql/mysql.h>

#include "judge_client.h"

extern int DEBUG;
extern int db_port;
extern int shm_run;
extern int max_running;
extern int db_timeout;
extern int sleep_time;
extern int java_time_bonus;
extern int java_memory_bonus;
extern int sim_enable;
extern int oi_mode;
extern int use_max_time;
extern char record_call;
extern char db_host[BUFSIZE];
extern char db_user[BUFSIZE];
extern char work_dir[BUFSIZE];
extern char db_passwd[BUFSIZE];
extern char db_name[BUFSIZE];
extern char oj_home[BUFSIZE];
extern char java_xms[BUFSIZE];
extern char java_xmx[BUFSIZE];
extern char LANG_NAME[BUFSIZE];
extern char lang_ext[15][8];
extern MYSQL *conn;
extern struct solution_t *solution;

void init_parameters(int argc, char **argv, int *solution_id, int *runner_id)
{
	if (argc < 3) {
		fprintf(stderr, "Usage:%s solution_id runner_id.\n", argv[0]);
		fprintf(stderr,
			"Multi:%s solution_id runner_id judge_base_path.\n",
			argv[0]);
		fprintf(stderr,
			"Debug:%s solution_id runner_id judge_base_path debug.\n",
			argv[0]);
		exit(EXIT_SUCCESS);
	}
	DEBUG = (argc > 4);
	record_call = (argc > 5);
	if (argc > 5) {
		strcpy(LANG_NAME, argv[5]);
	}
	if (argc > 3) {
		strcpy(oj_home, argv[3]);
	} else {
		strcpy(oj_home, "/home/judge");
	}

	chdir(oj_home);		// change the dir init our work

	*solution_id = atoi(argv[1]);
	*runner_id = atoi(argv[2]);
}

int write_log(const char *fmt, ...)
{
	va_list ap;
	char buffer[BUFSIZE * 4];
	sprintf(buffer, "%s/log/client.log", oj_home);
	FILE *fp = fopen(buffer, "a+");
	if (DEBUG) {
		freopen("/dev/stdout", "w", fp);
	}
	if (fp == NULL) {
		fprintf(stderr, "open log file error:%s.\n", strerror(errno));
		return 0;
	}
	va_start(ap, fmt);
	vsprintf(buffer, fmt, ap);
	time_t tm = time(NULL);
	char timestr[BUFSIZE];
	sprintf(timestr, "%s", ctime(&tm));
	int len = strlen(timestr);
	timestr[len - 1] = '\0';
	int ret = fprintf(fp, "[%s]:%s", timestr, buffer);
	va_end(ap);
	fclose(fp);
	return ret;
}

int execute_cmd(const char *fmt, ...)
{
	char cmd[BUFSIZE];
	int ret = 0;
	va_list ap;
	va_start(ap, fmt);
	vsprintf(cmd, fmt, ap);
	ret = system(cmd);
	va_end(ap);
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
	char buf[BUFSIZE];
	char *start, *end;
	strcpy(buf, c);
	start = buf;
	while (isspace(*start))
		start++;
	end = start;
	while (!isspace(*end))
		end++;
	*end = '\0';
	strcpy(c, start);
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
	char buf2[BUFSIZE];
	if (read_buf(buf, key, buf2)) {
		sscanf(buf2, "%d", value);
	}
}

long get_file_size(const char *filename)
{
	struct stat f_stat;
	if (stat(filename, &f_stat) == -1) {
		return 0;
	}
	return (long)f_stat.st_size;
}

int get_problem_info(struct problem_info_t *problem_info)
{
	write_log("get problem %d info.\n", problem_info->problem_id);
	if (problem_info->problem_id == 0) {
		problem_info->spj = 0;
		problem_info->time_limit = 5;
		problem_info->memory_limit = 128;
	}
	MYSQL_RES *result;
	if (execute_sql("select spj, time_limit, memory_limit, "
			"accepted, submit, solved from problem where "
			"problem_id = %d", problem_info->problem_id) < 0) {
		return -1;
	}
	result = mysql_store_result(conn);
	if (result == NULL) {
		write_log("read mysql result error:%s.\n", mysql_error(conn));
		return -1;
	}
	my_ulonglong cnt = mysql_num_rows(result);
	if (cnt > 0) {
		MYSQL_ROW row = mysql_fetch_row(result);
		if (row == NULL) {
			write_log("fetch mysql row error:%s.\n", mysql_error(conn));
			return -1;
		}
		problem_info->spj = atoi(row[0]);
		problem_info->time_limit += atoi(row[1]);
		problem_info->memory_limit += atoi(row[2]);
		problem_info->accepted = atoi(row[3]);
		problem_info->submit = atoi(row[4]);
		problem_info->solved = atoi(row[5]);
	} else {
		write_log("no problem %d.", problem_info->problem_id);
		return -1;
	}
	if (execute_sql("select ojtype, origin_id from vjudge where "
			"problem_id = %d", problem_info->problem_id) < 0) {
		return -1;
	}
	result = mysql_store_result(conn);
	if (result == NULL) {
		write_log("read mysql result error:%s.\n", mysql_error(conn));
		return -1;
	}
	cnt = mysql_num_rows(result);
	if (cnt > 0) {
		MYSQL_ROW row = mysql_fetch_row(result);
		if (row == NULL) {
			write_log("fetch mysql row error:%s.\n", mysql_error(conn));
			return -1;
		}
		problem_info->ojtype = atoi(row[0]);
		problem_info->origin_id = atoi(row[1]);
	} else {
		write_log("no problem %d.", problem_info->problem_id);
		return -1;
	}
	// never bigger than judged set value
	if (problem_info->time_limit > 300 || problem_info->time_limit < 1) {
		problem_info->time_limit = 300;
	}
	if (problem_info->memory_limit > 1024 || problem_info->memory_limit < 1) {
		problem_info->memory_limit = 1024;
	}
	if (DEBUG) {
		write_log("problem_id = %d\n", problem_info->problem_id);
		write_log("spj = %d\n", problem_info->spj);
		write_log("time_limit = %d\n", problem_info->time_limit);
		write_log("memory_limit = %d\n", problem_info->memory_limit);
		write_log("accepted = %d\n", problem_info->accepted);
		write_log("submit = %d\n", problem_info->submit);
		write_log("solved = %d\n", problem_info->solved);
		write_log("ojtype = %d\n", problem_info->ojtype);
		write_log("origin_id = %d\n", problem_info->origin_id);
	}
	return 0;
}

struct solution_t *get_solution(int sid)
{
	write_log("get solution %d info.\n", sid);
	struct solution_t *solution = (struct solution_t *)malloc(sizeof(struct solution_t));
	if (solution == NULL) {
		write_log("alloc memory error!\n");
		return NULL;
	}
	memset(solution, 0, sizeof(struct solution_t));
	if (execute_sql("select solution.solution_id, problem_id, "
				"user_id, time, memory, in_date, result, "
				"language, ip, contest_id, valid, num, "
				"code_length, judgetime, pass_rate, source "
				"from solution,source_code where "
				"solution.solution_id = source_code.solution_id "
				"and solution.solution_id = %d", sid) < 0) {
		free(solution);
		return NULL;
	}
	MYSQL_RES *result = mysql_store_result(conn);
	if (result == NULL) {
		write_log("read mysql result error:%s.\n", mysql_error(conn));
		free(solution);
		return NULL;
	}
	my_ulonglong cnt = mysql_num_rows(result);
	if (cnt > 0) {
		MYSQL_ROW row = mysql_fetch_row(result);
		if (row == NULL) {
			free(solution);
			write_log("fetch mysql row error:%s\n", mysql_error(conn));
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
		// java is luck
		if (solution->language >= 3) {
			solution->problem_info.time_limit = java_time_bonus;
			solution->problem_info.memory_limit = java_memory_bonus;
		}
		if (DEBUG) {
			write_log("solution_id = %d\n", solution->solution_id);
			write_log("user_id = %s\n", solution->user_id);
			write_log("time = %d\n", solution->time);
			write_log("memory = %d\n", solution->memory);
			write_log("result = %d\n", solution->result);
			write_log("in_date = %s\n", solution->in_date);
			write_log("language = %d\n", solution->language);
			write_log("ip = %s\n", solution->ip);
			write_log("contest_id = %d\n", solution->contest_id);
			write_log("valid = %d\n", solution->valid);
			write_log("num = %d\n", solution->num);
			write_log("code_length = %d\n", solution->code_length);
			write_log("judgetime = %s\n", solution->judgetime);
			write_log("pass_rate = %f\n", solution->pass_rate);
			write_log("src = %s\n", solution->src);
		}
		if (get_problem_info(&solution->problem_info) < 0) {
			free(solution);
			return NULL;
		}
	} else {
		write_log("no solution %d.\n", sid);
		free(solution);
		return NULL;
	}
	return solution;
}

int update_solution(void)
{
	char *sql = (char *)malloc(BUFSIZE * BUFSIZE);
	if (sql == NULL) {
		write_log("alloc memory error!\n");
		return -1;
	}
	memset(sql, 0, BUFSIZE * BUFSIZE);
	if (execute_sql("update solution set time=%d, memory=%d, "
			"result=%d, pass_rate=%f, judgetime=now() where "
			"solution_id = %d", solution->time, solution->memory,
			solution->result, solution->pass_rate,
			solution->solution_id) < 0) {
		free(sql);
		return -1;
	}
	char *end;
	if (solution->isce) {
		if (execute_sql("delete from compileinfo where "
					"solution_id = %d",
					solution->solution_id) < 0) {
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
		if (execute_sql(sql) < 0) {
			free(sql);
			return -1;
		}
	}
	if (solution->isre) {
		if (execute_sql("delete from runtimeinfo where "
					"solution_id = %d",
					solution->solution_id) < 0) {
			free(sql);
			return -1;
		};
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
		if (execute_sql(sql) < 0) {
			free(sql);
			return -1;
		}
	}
	return 0;
}

void save_solution_src(void)
{
	int lang = solution->language;
	char src_path[BUFSIZE];
	sprintf(src_path, "Main.%s", lang_ext[lang]);
	write_log("save solution source code in %s.\n", src_path);
	// create the src file
	FILE *fp_src = fopen(src_path, "w");
	if (fp_src == NULL) {
		write_log("create file error!\n");
	}
	fprintf(fp_src, "%s", solution->src);
	fclose(fp_src);
}
