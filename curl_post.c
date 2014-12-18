/*************************************************************************
	> File Name: curl_post.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月18日 星期四 19时44分36秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

/*
 * http表单提交
 */
int main(int argc, char *argv[])
{
	CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
	if (ret != CURLE_OK) {
		fprintf(stderr, "初始化curl失败！\n");
		exit(EXIT_FAILURE);
	}

	CURL *easy_handle = curl_easy_init();
	if (easy_handle == NULL) {
		fprintf(stderr, "获取easy_handle失败！\n");
		curl_global_cleanup();
		exit(EXIT_FAILURE);
	}

	// 设置网址，这个url是form表单action指向的网址
	curl_easy_setopt(easy_handle, CURLOPT_URL, "http://gwq:8080/Demo/checklogin.jsp");
	// 设置post的参数
	curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS, "username=gwq5210&passwd=123456789");

	// 执行post
	ret = curl_easy_perform(easy_handle);
	if (ret != CURLE_OK) {
		printf("提交数据失败！\n");
	} else {
		printf("提交数据成功！\n");
	}

	// 释放资源
	curl_easy_cleanup(easy_handle);
	curl_global_cleanup();

	return 0;
}
