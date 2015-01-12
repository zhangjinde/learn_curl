/*************************************************************************
	> File Name: judge_client.h
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2015年01月06日 星期二 22时07分28秒
 ************************************************************************/

#ifndef _JUDGE_CLIENT_H
#define _JUDGE_CLIENT_H

#include <stdio.h>
#include <mysql/mysql.h>
#include <curl/curl.h>

#define STD_MB 1048576		//1M
#define STD_T_LIM 2
#define STD_F_LIM (STD_MB<<5)	//32M
#define STD_M_LIM (STD_MB<<7)	//128M
#define BUFSIZE 512

#define OJ_WT0 0		//Pending
#define OJ_WT1 1		//Pending Rejudge
#define OJ_CI 2			//Compiling
#define OJ_RI 3			//Running & Judging
#define OJ_AC 4			//Accepted
#define OJ_PE 5			//Persentation Error
#define OJ_WA 6			//Wrong Answer
#define OJ_TL 7			//Time Limit Exceeded
#define OJ_ML 8			//Memory Limit Exceeded
#define OJ_OL 9			//Output Limit Exceeded
#define OJ_RE 10		//Runtime Error
#define OJ_CE 11		//Compile Error
#define OJ_CO 12		//Compile OK
#define OJ_TR 13		//Test Running
#define OJ_JE 14		//Judge Error


#define ZOJ_COM
/*copy from ZOJ
 http://code.google.com/p/zoj/source/browse/trunk/judge_client/client/tracer.cc?spec=svn367&r=367#39
 */
#ifdef __i386
#define REG_SYSCALL orig_eax
#define REG_RET eax
#define REG_ARG0 ebx
#define REG_ARG1 ecx
#else
#define REG_SYSCALL orig_rax
#define REG_RET rax
#define REG_ARG0 rdi
#define REG_ARG1 rsi
#endif

/*
 * 题目信息结构体
 */
struct problem_info_t {
	int problem_id;		// 题目编号
	int origin_id;			// 题目在原oj上的题目编号
	char source[BUFSIZE];		// 题目来源，为抓取题目的ojname
	int time_limit;			// 时限（秒）
	int memory_limit;		// 内存限制（兆）
	int ojtype;			// oj的类型
	int spj;			// 是否是spj
	int ischa;			// 是否查重
	int accepted;			// 通过的提交次数
	int submit;			// 总提交次数
};

/*
 * 记录一次提交的信息
 */
struct solution_t {
	int solution_id;		// 运行id
	struct problem_info_t problem_info;	// 题目信息
	char user_id[BUFSIZE];		// 用户id
	int time;			// 用时（秒）
	int memory;			// 所用空间
	int result;			// 结果（4：AC）
	int ispe;			// PE
	int language;			// 语言
	int code_length;		// 代码长度
	double pass_rate;		// 通过百分比（OI模式下可用）
	char src[BUFSIZE * BUFSIZE];	// 源代码
	// 运行错误信息
	int isre;
	char runtimeinfo[BUFSIZE * BUFSIZE];
	// 编译错误信息
	int isce;
	char compileinfo[BUFSIZE * BUFSIZE];
};


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
extern int vj_max_wait_time;
extern int islogin;
extern char record_call;
extern char db_host[BUFSIZE];
extern char db_user[BUFSIZE];
extern char db_passwd[BUFSIZE];
extern char db_name[BUFSIZE];
extern char vjudge_user[BUFSIZE];
extern char cefname[BUFSIZE];
extern char refname[BUFSIZE];
extern char vjudge_passwd[BUFSIZE];
extern char work_dir[BUFSIZE];
extern char cookiename[BUFSIZE];
extern char oj_home[BUFSIZE];
extern char java_xms[BUFSIZE];
extern char java_xmx[BUFSIZE];
extern char LANG_NAME[BUFSIZE];
extern char lang_ext[15][8];
extern char lang_table[20][20];
extern MYSQL *conn;
extern CURL *curl;
extern struct solution_t *solution;
extern int call_counter[BUFSIZE];
extern int call_array_size;

extern void trim(char *c);
extern void to_lowercase(char *c);
extern int after_equal(char *c);
extern MYSQL *prepare_mysql(void);
extern void cleanup_mysql(void);
extern int write_log(const char *fmt, ...);
extern long get_file_size(const char *filename);
extern void copy_runtime(void);
extern int execute_cmd(const char *fmt, ...);
extern int execute_sql(const char *fmt, ...);
extern int read_buf(char *buf, const char *key, char *value);
extern void read_int(char *buf, const char *key, int *value);
extern void init_parameters(int argc, char **argv, int *solution_id, int *runner_id);
extern int update_solution(void);
extern struct solution_t *get_solution(int sid);
extern int get_problem_info(struct problem_info_t *problem_info);
extern void save_solution_src(void);
extern int load_file(const char *filename, char *buf);
extern char *url_encode(char *str);
extern char *url_decode(char *str);
extern int addceinfo(int solution_id, const char *filename);
extern int addreinfo(int solution_id, const char *filename);
extern int update_user(void);
extern int update_problem(void);
extern int save_custom_input(void);
extern int compile(void);
extern int vjudge(void);
extern void init_syscalls_limits(int lang);
extern void run_solution(void);
extern int addcustomout(int solution_id);
extern void watch_solution(pid_t pid, char *outfile, char *userfile);
extern void test_run(void);
extern int compare(const char *file1, const char *file2);
extern void judge_solution(char *infile, char *outfile, char *userfile, double num_of_test);
extern int special_judge(char *infile, char *outfile, char *userfile);
extern int get_sim(void);
extern void copy_ac_src(void);
extern int perform_curl(const char *filename);
extern CURL *prepare_curl(void);
extern const char *curl_error(CURLcode errornum);
extern void cleanup_curl(void);
extern void clear_cookie(void);
extern int submit(void);
extern int login(void);
extern int get_status(void);
extern int submit_hduoj(void);
extern int login_hduoj(void);
extern int get_status_hduoj(void);
extern int utf2gbk(char *buf, size_t len);
extern int gbk2utf8(char *buf, size_t len);
extern int convert(char *buf, size_t len, const char *from, const char *to);
extern int get_reinfo(void);
extern int get_ceinfo(void);
extern int get_reinfo_hduoj(void);
extern int get_ceinfo_hduoj(void);
extern int is_final_result(char *buf);
extern int convert_result(char *buf);
extern int convert_result_hduoj(char *buf);

#endif	// _JUDGE_CLIENT_H
