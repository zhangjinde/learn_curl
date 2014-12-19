/*************************************************************************
	> File Name: curl_send_cookie.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月19日 星期五 12时40分52秒
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

	curl_easy_setopt(easy_handle, CURLOPT_URL, "http://gwq:8080/Demo/rcookie.jsp");
	// 设置cookie
	curl_easy_setopt(easy_handle, CURLOPT_COOKIE, "name=gwq5210; address=Zhengzhou");

	// 发送
	ret = curl_easy_perform(easy_handle);
	if (ret != CURLE_OK) {
		printf("发送cookie失败！\n");
	} else {
		printf("发送cookie成功！\n");
	}


	// 释放资源
	curl_easy_cleanup(easy_handle);
	curl_global_cleanup();

	return 0;
}
