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
extern char db_passwd[BUFSIZE];
extern char db_name[BUFSIZE];
extern char oj_home[BUFSIZE];
extern char java_xms[BUFSIZE];
extern char java_xmx[BUFSIZE];

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

