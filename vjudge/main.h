/*************************************************************************
	> File Name: main.h
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2015年01月06日 星期二 22时07分28秒
 ************************************************************************/

#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>

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
	int problem_id;			// 题目id
	char user_id[BUFSIZE];		// 用户id
	int time;			// 用时（秒）
	int memory;			// 所用空间
	int result;			// 结果（4：AC）
	char in_date[BUFSIZE];		// 加入时间
	int language;			// 语言
	char ip[BUFSIZE];		// 用户ip
	int contest_id;			// 所属比赛id
	int vaild;			// 是否有效？
	int num;			// 题目在比赛中的序号
	int code_lenght;		// 代码长度
	char judgetime[BUFSIZE];	// 判题时间
	double pass_rate;		// 通过百分比（OI模式下可用）
	char src[BUFSIZE * BUFSIZE];	// 源代码
	// 运行错误信息
	char runtimeinfo[BUFSIZE * BUFSIZE];
	// 编译错误信息
	char compileinfo[BUFSIZE * BUFSIZE];
};

extern void init(void);

#endif	// _MAIN_H
