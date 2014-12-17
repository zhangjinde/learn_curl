/*************************************************************************
	> File Name: curl_version.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月17日 星期三 21时29分05秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

int main(int argc, char *argv[])
{
	CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
	if (ret != CURLE_OK) {
		fprintf(stderr, "初始化curl失败！\n");
		exit(EXIT_FAILURE);
	}

	// 获取到版本信息的字符串
	printf("%s\n", curl_version());
	// 这个方法可以获取到版本的详细信息
	//curl_version_info_data *p_version = curl_version_info(CURLVERSION_NOW);

	curl_global_cleanup();
	return 0;
}
