/*************************************************************************
	> File Name: main.h
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时18分21秒
 ************************************************************************/
#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#include "function.h"

#define DEBUG 1
#define BUFSIZE 1024
#define OJMAX 20

/*
 * 题目信息结构体
 */
struct problem_info_t {
	int proble_id;			// 题目编号
	int origin_id;			// 题目在原oj上的题目编号
	char title[BUFSIZE];		// 题目标题
	char description[2 * BUFSIZE];	// 题目描述
	char input[2 * BUFSIZE];	// 输入说明
	char output[2 * BUFSIZE];	// 输出说明
	char sample_input[BUFSIZE];	// 样例输入
	char sample_output[BUFSIZE];	// 样例输出
	char hint[BUFSIZE];		// 提示
	char source[BUFSIZE];		// 题目来源
	int time_limit;			// 时限（秒）
	int memory_limit;		// 内存限制（兆）
	char ojtype;			// oj的类型
};

static int ojcnt;
static char ojstr[OJMAX][BUFSIZE];
static char ojurl[OJMAX][BUFSIZE];

#endif	// _MAIN_H
