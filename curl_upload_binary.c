/*************************************************************************
	> File Name: curl_upload_binary.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月19日 星期五 10时29分22秒
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

	// 上传二进制数据
	char data[] = {1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0};

	// 设置消息头
	struct curl_slist *http_headers = NULL;
	http_headers = curl_slist_append(http_headers, "Content-Type: text/xml");

	curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, http_headers);
	curl_easy_setopt(easy_handle, CURLOPT_URL, "http://localhost:8080/Demo/upload.jsp");
	curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS, data);
	// 设置上传参数大小
	curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDSIZE, sizeof(data));
	// 调试
	curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, sizeof(data));

	ret = curl_easy_perform(easy_handle);
	if (ret != CURLE_OK) {
		printf("上传失败！\n");
	} else {
		printf("上传成功！\n");
	}

	// 释放资源
	curl_slist_free_all(http_headers);
	curl_easy_cleanup(easy_handle);
	curl_global_cleanup();
	return 0;
}
