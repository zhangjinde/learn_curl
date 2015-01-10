#include <stdio.h>

#include "judge_client.h"

extern int DEBUG = 1;
extern int db_port;
extern int shm_run = 0;
extern int max_running;
extern int db_timeout;
extern int sleep_time;
extern int java_time_bonus = 5;
extern int java_memory_bonus = 512;
extern int sim_enable = 0;
extern int oi_mode = 0;
extern int use_max_time = 0;
extern char record_call = 0;
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
