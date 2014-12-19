/*************************************************************************
	> File Name: curl_ftp_list.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月19日 星期五 12时27分00秒
 ************************************************************************/

#include <stdio.h>
#include <error.h>
#include <stdlib.h>
#include <curl/curl.h>

int main(int argc, char **argv)
{
	CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
	if (ret != CURLE_OK) {
		error(EXIT_FAILURE, 0, "初始化curl失败！\n");
	}

	CURL *easy_handle = curl_easy_init();
	if (easy_handle == NULL) {
		curl_global_cleanup();
		error(EXIT_FAILURE, 0, "获取easy_handle失败！\n");
	}

	curl_easy_setopt(easy_handle, CURLOPT_URL, "ftp://localhost/");
	curl_easy_setopt(easy_handle, CURLOPT_USERPWD, "gwq5210:1234567890");
	curl_easy_setopt(easy_handle, CURLOPT_CUSTOMREQUEST, "NLST");

	ret = curl_easy_perform(easy_handle);
	if (ret != CURLE_OK) {
		curl_easy_cleanup(easy_handle);
		curl_global_cleanup();
		error(EXIT_FAILURE, 0, "初始化curl失败！\n");
	}

	curl_easy_cleanup(easy_handle);
	curl_global_cleanup();

	return 0;
}
