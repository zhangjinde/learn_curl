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
#define OJ_CO 12
#define OJ_TR 13		//Test Running


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

extern void trim(char *c);
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
extern int get_sim(int solution_id, int lang, int pid);

#endif	// _JUDGE_CLIENT_H
