/*************************************************************************
	> File Name: get_problem.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时37分44秒
 ************************************************************************/

#include "get_problem.h"

int DEBUG = 0;
int pid;
int oj_cnt;
int oj_type;
int db_port;
int sleep_time;
int db_timeout;
char db_user[BUFSIZE];
char db_passwd[BUFSIZE];
char db_host[BUFSIZE];
char db_name[BUFSIZE];
char oj_name[BUFSIZE];
CURL *curl;
MYSQL *conn;
struct problem_info_t *problem_info;

/*
 * oj_type 0 for hduoj.
 * oj_type 1 for poj.
 * oj_type 1 for zoj.
 */
char oj_str[OJMAX][BUFSIZE] = {"hduoj", "poj", "zoj"};
char oj_url[OJMAX][BUFSIZE] = {
	"http://acm.hdu.edu.cn/showproblem.php?pid=",
	"http://poj.org/problem?id=",
	"http://acm.zju.edu.cn/onlinejudge/showProblem.do?problemCode="
};
char oj_imgurl[OJMAX][BUFSIZE] = {
	"http://acm.hdu.edu.cn/data/images",
};


// read the configue file
void init_conf()
{
	write_log("init mysql config.\n");
	char buf[BUFSIZE];
	db_port = 3306;
	sleep_time = 2;
	db_timeout = 120;
	FILE *fp = fopen("./get_problem.conf", "r");
	if (fp != NULL) {
		while (fgets(buf, BUFSIZE - 1, fp) != NULL) {
			read_buf(buf, "DB_HOST", db_host);
			read_buf(buf, "DB_USER", db_user);
			read_buf(buf, "DB_PASSWD", db_passwd);
			read_buf(buf, "DB_NAME", db_name);
			read_int(buf, "DB_PORT", &db_port);
			read_int(buf, "OJ_CNT", &oj_cnt);
			read_int(buf, "DB_TIMEOUT", &db_timeout);
			read_int(buf, "SLEEP_TIME", &sleep_time);
		}
		fclose(fp);
	} else {
		write_log("init mysql config error:%s.\n", strerror(errno));
	}
}

/*
 * Usage: ./get_problem oj_name from_problem_id to_problem_id [work_dir] [debug]
 */
int main(int argc, char *argv[])
{
	if (argc < 4 || argc > 5) {
		fprintf(stderr, "Usage: %s oj_name from_problem_id to_problem_id [debug]\n", argv[0]);
		exit(EXIT_SUCCESS);
	}

	DEBUG = (argc == 5);

	// read config file
	init_conf();

	int i = 0;
	int from = atoi(argv[2]);
	int to = atoi(argv[3]);
	strcpy(oj_name, argv[1]);
	oj_type = -1;
	for (i = 0; i < oj_cnt; ++i) {
		if (strcmp(oj_name, oj_str[i]) == 0) {
			oj_type = i;
			break;
		}
	}

	if (oj_type < 0) {
		write_log("%s: unsupported oj\n", oj_name);
		exit(EXIT_FAILURE);
	}

	write_log("ojname = %s\n", oj_name);
	write_log("ojurl = %s\n", oj_url[oj_type]);
	write_log("type = %d\n", oj_type);
	write_log("from = %d\n", from);
	write_log("to = %d\n", to);

	curl = prepare_curl();
	if (curl == NULL) {
		write_log("prepare curl handle error.\n");
		exit(EXIT_FAILURE);
	}
	conn = prepare_mysql();
	if (curl == NULL) {
		write_log("prepare mysql handle error.\n");
		cleanup_curl();
		exit(EXIT_FAILURE);
	}
	problem_info = (struct problem_info_t *)malloc(sizeof(struct problem_info_t));
	if (problem_info == NULL) {
		write_log("alloc problem_info memory error.\n");
		cleanup_mysql();
		cleanup_curl();
		exit(EXIT_FAILURE);
	}

	for (pid = from; pid <= to; ++pid) {
		memset(problem_info, 0, sizeof(struct problem_info_t));
		int ret = get_problem();
		if (ret < 0) {
			write_log("get %s problem %d error.\n", oj_name, pid);
		} else if (ret == 0) {
			write_log("get %s problem %d success.\n", oj_name, pid);

			ret = add_problem();
			if (ret < 0) {
				write_log("add %s problem %d to mysql database error.\n", oj_name, pid);
			} else {
				if (ret != 2) {
					write_log("add %s problem %d to mysql database success.\n", oj_name, pid);
				} else {
					write_log("%s problem %d already exists.\n", oj_name, pid);
				}
			}
		} else {
			write_log("%s no problem %d.\n", oj_name, pid);
		}
		if (!DEBUG) {
			execute_cmd("rm -f %d", pid);
		}
		if (pid != to) {
			write_log("get next problem after %d seconds.\n", sleep_time);
		}
		sleep(sleep_time);
	}


	free(problem_info);
	cleanup_curl();
	cleanup_mysql();

	return 0;
}
