/*************************************************************************
	> File Name: probleminfo.h
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时15分48秒
 ************************************************************************/

#ifndef _PROBLEMINFO_H
#define _PROBLEMINFO_H

#include "main.h"

/*
 * 题目信息结构体
 */
struct problem_info_t {
	int proble_id;			// 题目编号
	int origin_id;			// 题目在原oj上的题目编号
	char title[BUFSIZ];		// 题目标题
	char description[2 * BUFSIZ];	// 题目描述
	char input[2 * BUFSIZ];		// 输入说明
	char output[2 * BUFSIZ];	// 输出说明
	char sample_input[BUFSIZ];	// 样例输入
	char sample_output[BUFSIZ];	// 样例输出
	int spj;			// 是否时spj
	char hint[BUFSIZ];		// 提示
	char source[BUFSIZ];		// 题目来源
	int time_limit;			// 时限（秒）
	int memory_limit;		// 内存限制（兆）
	int defunct;			// 是否屏蔽
	int accepted;			// 总ac次数
	int submit;			// 总提交次数
	int solved;			// 解答（未用）
};

#endif // _PROBLEMINFO_H
