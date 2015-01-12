/*************************************************************************
	> File Name: curl_handle.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2015年01月12日 星期一 22时32分53秒
 ************************************************************************/

#include "get_problem.h"

const char *curl_error(CURLcode errornum)
{
	return curl_easy_strerror(errornum);
}

CURL *prepare_curl(void)
{
	write_log("prepare curl handle.\n");
	CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
	if (ret != CURLE_OK) {
		write_log("init curl error:%s.\n", curl_error(ret));
		return NULL;
	}
	CURL *curl = curl_easy_init();
	if (curl == NULL) {
		curl_global_cleanup();
		write_log("get easy curl handle error.\n");
		return NULL;
	}
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, db_timeout);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, db_timeout);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "zzuoj, curl");
	// 不认证ssl证书
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	// 跟踪重定向的信息
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	if (DEBUG) {
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	}

	return curl;
}

int perform_curl(const char *filename)
{
	FILE *fp = fopen(filename, "w");
	if (fp == NULL) {
		write_log("can create file %s.\n", filename);
	}
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	CURLcode ret = curl_easy_perform(curl);
	if (ret != CURLE_OK) {
		write_log("curl perform error:%s.\n", curl_error(ret));
		write_log("try again 5 seconds later.\n");
		sleep(5);
		ret = curl_easy_perform(curl);
		if (ret != CURLE_OK) {
			fclose(fp);
			write_log("curl perform error:%s.\n", curl_error(ret));
			return -1;
		}
	}

	int http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	write_log("http_code = %d\n", http_code);
	if (http_code >= 400) {
		write_log("http server error.\n");
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

void cleanup_curl(void)
{
	if (curl != NULL) {
		// 释放资源
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		write_log("close curl connection.\n");
	}
}
