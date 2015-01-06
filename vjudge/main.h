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
 * 记录html中的状态，从而获得题目描述
 */
struct html_state_t {
	int isdescription;		// 是否是题目描述
	int istitle;			// 是否是题目标题
	int isinput;			// 是否是题目输入说明
	int isoutput;			// 是否是题目输出说明
	int issinput;			// 是否是题目样例输入
	int issoutput;			// 是否是题目样例输出
	int ishint;			// 是否是题目提示
	int islimit;			// 是否是题目限制
	int isstat;			// 是否是题目通过统计
	int isspj;			// 是否是题目spj描述
	// 题目信息
	struct problem_info_t *problem_info;
};

extern void init(void);

#endif	// _MAIN_H
