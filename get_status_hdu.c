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
#include <unistd.h>
#include <iconv.h>
#include <curl/curl.h>

#define BUFSIZE 1024

char cookiename[] = "cookie";
char filename[] = "status.txt";
CURL *curl;

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
	while (fgets(tmp, BUFSIZE * BUFSIZE, fp) != NULL) {
		strcat(buf, tmp);
	}
	free(tmp);
	return 0;
}

const char *curl_error(CURLcode errornum)
{
	return curl_easy_strerror(errornum);
}

CURL *prepare_curl(void)
{
	printf("prepare curl handle.\n");
	CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
	if (ret != CURLE_OK) {
		printf("init curl error:%s.\n", curl_error(ret));
		return NULL;
	}
	CURL *curl = curl_easy_init();
	if (curl == NULL) {
		curl_global_cleanup();
		printf("get easy curl handle error.\n");
		return NULL;
	}
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 120);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "zzuoj, curl");
	// 不认证ssl证书
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	// 跟踪重定向的信息
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	// 设置cookie信息，否则就不能保存登陆信息
	// 设置读取cookie的文件名
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiename);
	// 设置写入cookie的文件名
	curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookiename);

	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

	return curl;
}

int perform_curl(const char *filename)
{
	FILE *fp = fopen(filename, "w");
	if (fp == NULL) {
		printf("can create file %s.\n", filename);
	}
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	CURLcode ret = curl_easy_perform(curl);
	if (ret != CURLE_OK) {
		printf("curl perform error:%s.\n", curl_error(ret));
		printf("try again 5 seconds later.\n");
		sleep(5);
		ret = curl_easy_perform(curl);
		if (ret != CURLE_OK) {
			fclose(fp);
			printf("curl perform error:%s.\n", curl_error(ret));
			return -1;
		}
	}

	int http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	printf("http_code = %d\n", http_code);
	if (http_code >= 400) {
		printf("http server error.\n");
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

void clear_cookie(void)
{
	FILE *fp = fopen(cookiename, "w");
	fclose(fp);
}

void cleanup_curl(void)
{
	// 释放资源
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

int main(int argc, char *argv[])
{
	int status, i, j;
	regmatch_t pmatch[10];
	const size_t nmatch = 10;
	regex_t reg;
	const char *pattern = "<td height=22px>([0-9]*)</td>"
		"<td>([: 0-9-]*)</td>"
		"<td><font color=[ a-zA-Z]*>([^/]*)</font></td>"
		"<td><a[ 0-9a-zA-Z\\./\\?\\\"=]*>([0-9]*)</a></td>"
		"<td>([0-9]*)MS</td>"
		"<td>([0-9]*)K</td>"
		"<td>([0-9]*)B</td>"
		;
	char *buf = (char *)malloc(BUFSIZE * BUFSIZE);
	char err[BUFSIZE];
	if (buf == NULL) {
		printf("alloc memory error.\n");
		exit(EXIT_FAILURE);
	}


	curl = prepare_curl();
	if (curl == NULL) {
		printf("prepare curl handle error, try again 5 seconds later.\n");
		sleep(5);
		curl = prepare_curl();
		if (curl == NULL) {
			printf("prepare curl handle error.\n");
			return -1;
		}
	}

	char url[BUFSIZE];
	sprintf(url, "http://acm.hdu.edu.cn/status.php?first=&pid=1403&user=zzuvjudge"
			"&lang=0&status=0");
	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_URL, url);

	perform_curl(filename);
	memset(buf, 0, BUFSIZE * BUFSIZE);
	load_file(filename, buf);
	gbk2utf8(buf, strlen(buf));
	printf("%s\n%lu\n", buf, strlen(buf));
	int ret = regcomp(&reg, pattern, REG_EXTENDED);
	if (ret) {
		regerror(ret, &reg, err, BUFSIZE);
		printf("compile regex error: %s.\n", err);
		free(buf);
		cleanup_curl();
		exit(EXIT_FAILURE);
	}
	status = regexec(&reg, buf, nmatch, pmatch, 0);
	if (status == REG_NOMATCH) {
		printf("no match.\n");
	} else if (status == 0) {
		printf("match.\n");
		for (i = 0; i < nmatch; ++i) {
			printf("i = %d\n", i);
			for (j = pmatch[i].rm_so; j < pmatch[i].rm_eo; ++j) {
				printf("%c", buf[j]);
			}
			printf("\n");
		}
	}
	cleanup_curl();
	free(buf);
	regfree(&reg);

	return 0;
}
