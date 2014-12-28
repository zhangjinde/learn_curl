/*************************************************************************
	> File Name: regex_match.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月28日 星期日 12时32分39秒
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

#define BUFSIZE 1024

int main(int argc, char *argv[])
{
	int status, i;
	int cflags = REG_EXTENDED;
	regmatch_t pmatch[1];
	const size_t nmatch = 1;
	regex_t reg;
	const char *pattern = "^\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*.\\w+([-.]\\w+)*$";
	char buf[BUFSIZE] = "gwq5210@qq.com";

	regcomp(&reg, pattern, cflags);
	status = regexec(&reg, buf, nmatch, pmatch, 0);
	if (status == REG_NOMATCH) {
		printf("没有匹配！\n");
	} else if (status == 0) {
		printf("匹配成功：\n");
		for (i = pmatch[0].rm_so; i < pmatch[0].rm_eo; ++i) {
			putchar(buf[i]);
		}
		printf("\n");
	}
	regfree(&reg);

	return 0;
}
