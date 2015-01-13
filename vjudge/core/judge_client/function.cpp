#include "judge_client.h"

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
	char *buffer = (char *)malloc(BUFSIZE * BUFSIZE);
	if (buffer == NULL) {
		fprintf(stderr, "alloc memory error.\n");
		return 0;
	}
	time_t t = time(NULL);
	struct tm *date = localtime(&t);
	sprintf(buffer, "%s", asctime(date));
	string timestr(buffer, strlen(buffer) - 1);
	sprintf(buffer, "%s/log/client%04d%02d%02d_%d.log", oj_home, date->tm_year + 1900,
			date->tm_mon + 1, date->tm_mday, runner_id);
	FILE *fp = fopen(buffer, "a+");
	if (fp == NULL) {
		fprintf(stderr, "open log file error:%s.\n", strerror(errno));
		free(buffer);
		return 0;
	}
	va_start(ap, fmt);
	vsprintf(buffer, fmt, ap);
	int ret = fprintf(fp, "[%s]:%s", timestr.c_str(), buffer);
	va_end(ap);
	fclose(fp);
	free(buffer);
	return ret;
}

int execute_cmd(const char *fmt, ...)
{
	char *cmd = (char *)malloc(BUFSIZE * BUFSIZE);
	if (cmd == NULL) {
		write_log("alloc execute_cmd cmd buf memory error.\n");
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
		return 0;
	}
	MYSQL_RES *result;
	if (execute_sql("select spj, time_limit, memory_limit, "
			"accepted, submit from problem where "
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
			mysql_free_result(result);
			write_log("fetch mysql row error:%s.\n", mysql_error(conn));
			return -1;
		}
		problem_info->spj = atoi(row[0]);
		problem_info->time_limit += atoi(row[1]);
		problem_info->memory_limit += atoi(row[2]);
		problem_info->accepted = atoi(row[3]);
		problem_info->submit = atoi(row[4]);
	} else {
		mysql_free_result(result);
		write_log("no problem %d.", problem_info->problem_id);
		return -1;
	}
	mysql_free_result(result);
	if (execute_sql("select ojtype, origin_id from vjudge where "
			"problem_id = %d", problem_info->problem_id) < 0) {
		mysql_free_result(result);
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
			mysql_free_result(result);
			write_log("fetch mysql row error:%s.\n", mysql_error(conn));
			return -1;
		}
		problem_info->ojtype = atoi(row[0]);
		problem_info->origin_id = atoi(row[1]);
	} else {
		problem_info->ojtype = -1;
		problem_info->origin_id = 0;
	}
	if (execute_sql("select ischa from cha where "
			"problem_id = %d", problem_info->problem_id) < 0) {
		mysql_free_result(result);
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
			mysql_free_result(result);
			write_log("fetch mysql row error:%s.\n", mysql_error(conn));
			return -1;
		}
		problem_info->ischa = atoi(row[0]);
	} else {
		problem_info->ischa = 0;
	}
	mysql_free_result(result);
	// never bigger than judged set value
	if (problem_info->time_limit > 300 || problem_info->time_limit < 1) {
		problem_info->time_limit = 300;
	}
	if (problem_info->memory_limit > 1024 || problem_info->memory_limit < 1) {
		problem_info->memory_limit = 1024;
	}
	write_log("problem_id = %d\n", problem_info->problem_id);
	write_log("spj = %d\n", problem_info->spj);
	write_log("time_limit = %d\n", problem_info->time_limit);
	write_log("memory_limit = %d\n", problem_info->memory_limit);
	write_log("accepted = %d\n", problem_info->accepted);
	write_log("submit = %d\n", problem_info->submit);
	write_log("ischa = %d\n", problem_info->ischa);
	write_log("ojtype = %d\n", problem_info->ojtype);
	write_log("origin_id = %d\n", problem_info->origin_id);
	return 0;
}

int update_user(void)
{
	write_log("update user %s statistics.\n", solution->user_id);
	if (execute_sql("update users set solved=(select count(distinct "
			"problem_id) from solution where user_id=\'%s\' "
			"and result=\'4\') where user_id=\'%s\'",
		solution->user_id, solution->user_id) < 0) {
		return -1;
	}
	if (execute_sql("update users set submit=(select count(*) from "
			"solution where user_id=\'%s\') where user_id=\'%s\'",
			solution->user_id, solution->user_id) < 0) {
		return -1;
	}
	return 0;
}

int update_problem(void)
{
	int pid = solution->problem_info.problem_id;
	write_log("update problem %d statistics.\n", pid);
	if (execute_sql("update problem set accepted=(select count(*) from "
			"solution where problem_id=\'%d\' and result=\'4\') "
			"where `problem_id`=\'%d\'", pid, pid) < 0) {
		return -1;
	}
	if (execute_sql("update problem set submit=(select count(*) from "
			"solution where problem_id=\'%d\') where "
			"problem_id=\'%d\'", pid, pid) < 0) {
		return -1;
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
				"user_id, time, memory, result, "
				"language, code_length, pass_rate, source "
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
			mysql_free_result(result);
			write_log("fetch mysql row error:%s\n", mysql_error(conn));
			return NULL;
		}
		solution->solution_id = sid;
		solution->problem_info.problem_id = atoi(row[1]);
		strcpy(solution->user_id, row[2]);
		solution->time = atoi(row[3]);
		solution->memory = atoi(row[4]);
		solution->result = atoi(row[5]);
		solution->language = atoi(row[6]);
		solution->code_length = atoi(row[7]);
		solution->pass_rate = atof(row[8]);
		strcpy(solution->src, row[9]);
		// java is luck
		if (solution->language >= 3) {
			solution->problem_info.time_limit = java_time_bonus;
			solution->problem_info.memory_limit = java_memory_bonus;
			// copy java.policy
			execute_cmd("/bin/cp %s/etc/java0.policy %s/java.policy",
				    oj_home, work_dir);
		}
		write_log("solution_id = %d\n", solution->solution_id);
		write_log("user_id = %s\n", solution->user_id);
		write_log("time = %d\n", solution->time);
		write_log("memory = %d\n", solution->memory);
		write_log("result = %d\n", solution->result);
		write_log("language = %d\n", solution->language);
		write_log("code_length = %d\n", solution->code_length);
		write_log("pass_rate = %f\n", solution->pass_rate);
		write_log("src = %s\n", solution->src);
		if (get_problem_info(&solution->problem_info) < 0) {
			write_log("get probelm %d info error.\n", solution->problem_info.problem_id);
			free(solution);
			mysql_free_result(result);
			return NULL;
		}
	} else {
		write_log("no solution %d.\n", sid);
		mysql_free_result(result);
		free(solution);
		return NULL;
	}
	mysql_free_result(result);
	return solution;
}

int addceinfo(int solution_id, const char *filename)
{
	write_log("add solution %d compile error info from file %s.\n", solution_id, filename);
	char *end;
	char *sql = (char *)malloc(BUFSIZE * BUFSIZE);
	if (sql == NULL) {
		write_log("alloc addceinfo sql buf memory error!\n");
		return -1;
	}
	if (execute_sql("delete from compileinfo where "
				"solution_id = %d",
				solution->solution_id) < 0) {
		free(sql);
		return -1;
	}
	memset(sql, 0, BUFSIZE * BUFSIZE);
	load_file(filename, solution->compileinfo);
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
	free(sql);
	return 0;
}

int addreinfo(int solution_id, const char *filename)
{
	write_log("add solution %d runtime error info from file %s.\n", solution_id, filename);
	char *end;
	char *sql = (char *)malloc(BUFSIZE * BUFSIZE);
	if (sql == NULL) {
		write_log("alloc addreinfo sql buf memory error!\n");
		return -1;
	}
	if (execute_sql("delete from runtimeinfo where "
				"solution_id = %d",
				solution->solution_id) < 0) {
		free(sql);
		return -1;
	};
	memset(sql, 0, BUFSIZE * BUFSIZE);
	load_file(filename, solution->runtimeinfo);
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
	free(sql);
	return 0;
}

int update_solution(void)
{
	if (execute_sql("update solution set time=%d, memory=%d, "
			"result=%d, pass_rate=%f, judgetime=now() where "
			"solution_id = %d", solution->time, solution->memory,
			solution->result, solution->pass_rate,
			solution->solution_id) < 0) {
		return -1;
	}
	if (solution->isce && addceinfo(solution->solution_id, cefname) < 0) {
		return -1;
	}
	if (solution->isre && addreinfo(solution->solution_id, refname) < 0) {
		return -1;
	}
	return 0;
}

void save_solution_src(void)
{
	int lang = solution->language;
	char src_path[10];
	sprintf(src_path, "Main.%s", lang_ext[lang]);
	write_log("save solution source code in %s.\n", src_path);
	// create the src file
	FILE *fp_src = fopen(src_path, "w");
	if (fp_src == NULL) {
		write_log("create file %s error!\n", src_path);
	}
	fprintf(fp_src, "%s", solution->src);
	fclose(fp_src);
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
		write_log("alloc load_file tmp buf memory error.\n");
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
