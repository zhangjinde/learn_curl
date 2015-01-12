/*************************************************************************
	> File Name: get_status_hdu.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2015年01月12日 星期一 11时20分44秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <iconv.h>

#define BUFSIZE 1024

int convert(char *buf, size_t len, const char *from, const char *to)
{
	iconv_t cd = iconv_open(to, from);
	if (cd == (iconv_t)-1) {
		perror("获取字符转换描述符失败！\n");
		return -1;
	}
	size_t sz = BUFSIZE * BUFSIZE;
	char *tmp_str = (char *)malloc(sz);
	if (tmp_str == NULL) {
		iconv_close(cd);
		fprintf(stderr, "分配内存失败！\n");
		return -1;
	}
	// 传进去的一定得是别的东西，原来的地址不能被改变
	char *in = buf;
	char *out = tmp_str;
	size_t inlen = len;
	size_t outlen = sz;
	memset(tmp_str, 0, sz);
	if (iconv(cd, &in, &inlen, &out, &outlen) == (size_t)-1) {
		iconv_close(cd);
		free(tmp_str);
		return -1;
	}
	iconv_close(cd);
	strcpy(buf, tmp_str);
	free(tmp_str);
	return 0;
}

int utf2gbk(char *buf, size_t len)
{
	return convert(buf, len, "UTF-8", "GBK");
}

int gbk2utf8(char *buf, size_t len)
{
	return convert(buf, len, "GBK", "UTF-8");
}

int load_file(const char *filename, char *buf)
{
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("cann't open file %s.\n", filename);
		return -1;
	}
	char *tmp = (char *)malloc(BUFSIZE * BUFSIZE);
	if (tmp == NULL) {
		printf("alloc memory error.\n");
		return -1;
	}
	buf[0] = '\0';
	while (fgets(tmp, BUFSIZE, fp) != NULL) {
		strcat(buf, tmp);
	}
	free(tmp);
	return 0;
}

int main(int argc, char *argv[])
{
	int status, i;
	regmatch_t pmatch[10];
	const size_t nmatch = 10;
	regex_t reg;
	const char *pattern = "<tr align=center ><td.*?>([0-9]*)</td>(<td>.*?</td>){8,8}";//<font.*?>(.*)</font></td><td>";
	char *buf = (char *)malloc(BUFSIZE * BUFSIZE);
	char err[BUFSIZE];
	if (buf == NULL) {
		printf("alloc memory error.\n");
		exit(EXIT_FAILURE);
	}

	memset(buf, 0, BUFSIZE * BUFSIZE);
	load_file("status.txt", buf);
	gbk2utf8(buf, strlen(buf));
	printf("%s\n%lu\n", buf, strlen(buf));
	int ret = regcomp(&reg, pattern, REG_EXTENDED);
	if (ret) {
		regerror(ret, &reg, err, BUFSIZE);
		printf("compile regex error: %s.\n", err);
		free(buf);
		exit(EXIT_FAILURE);
	}
	status = regexec(&reg, buf, nmatch, pmatch, 0);
	if (status == REG_NOMATCH) {
		printf("no match.\n");
	} else if (status == 0) {
		printf("match.\n");
		for (i = pmatch[0].rm_so; i < pmatch[0].rm_eo; ++i) {
			printf("%c", buf[i]);
		}
		printf("\n");
	}
	free(buf);
	regfree(&reg);

	return 0;
}
