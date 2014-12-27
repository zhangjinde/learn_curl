/*************************************************************************
	> File Name: get_problem_hdu.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月27日 星期六 14时02分35秒
 ************************************************************************/

/*
 * 获取杭电的题目描述，并将其插入到数据库中
 */
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

/*
 * 回调函数
 * buffer，接收到的数据所在缓冲区
 * size，数据长度
 * nmenb，数据片数量
 * user_p，用户自定义指针
 */
size_t save_data(void *buffer, size_t size, size_t nmenb, void *userp)
{
	FILE *fp = (FILE *)userp;
	size_t ret = fwrite(buffer, size, nmenb, fp);
	printf("%s\n", (char *)buffer);
	return ret;
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("用法：./get_data.c 文件名 网址\n");
		return 0;
	}

	CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
	if (ret != CURLE_OK) {
		fprintf(stderr, "curl初始化失败！\n");
		exit(EXIT_FAILURE);
	}
	CURL *easy_handle = curl_easy_init();
	if (easy_handle == NULL) {
		fprintf(stderr, "获取easy_handle错误！\n");
		curl_global_cleanup();
		exit(EXIT_FAILURE);
	}
	FILE *fp = fopen(argv[1], "w+");
	// 设置网址
	curl_easy_setopt(easy_handle, CURLOPT_URL, argv[2]);
	// 设置回调函数
	curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, save_data);
	// 回调函数参数
	curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, fp);
	// 输出通信过程中的一些细节，这可以用来调试
	// 如果使用的是http协议，请求头/响应头也会被输出
	//curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 1);
	// 将消息头设置为出现在内容中
	//curl_easy_setopt(easy_handle, CURLOPT_HEADER, 1);

	// 执行数据请求
	curl_easy_perform(easy_handle);

	// 释放资源
	fclose(fp);
	curl_easy_cleanup(easy_handle);
	curl_global_cleanup();

	return 0;
}
