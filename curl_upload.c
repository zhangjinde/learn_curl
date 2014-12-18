/*************************************************************************
	> File Name: curl_upload.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月18日 星期四 16时56分49秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

size_t readfile(char *buffer, size_t size, size_t nmemb, void *userp)
{
	return fread(buffer, size, nmemb, (FILE *)userp);
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("用法：./curl_upload 网址 文件名\n");
		return 0;
	}
	const char *url = argv[1];
	const char *filename = argv[2];
	CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
	if (ret != CURLE_OK) {
		fprintf(stderr, "初始化curl失败！\n");
		exit(EXIT_FAILURE);
	}

	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "打开文件%s失败！\n", filename);
		exit(EXIT_FAILURE);
	}

	// 获取文件大小
	fseek(fp, 0L, SEEK_END);
	int filesize = ftell(fp);
	rewind(fp);

	// 获取easy handle
	CURL *easy_handle = curl_easy_init();
	if (easy_handle == NULL) {
		fprintf(stderr, "获取easy_handle失败！\n");
		fclose(fp);
		curl_global_cleanup();
		exit(EXIT_FAILURE);
	}

	// 设置的url代表远端存储文件的名字
	curl_easy_setopt(easy_handle, CURLOPT_URL, url);
	// 上传
	curl_easy_setopt(easy_handle, CURLOPT_UPLOAD, 1L);
	// 设置读取回调函数
	curl_easy_setopt(easy_handle, CURLOPT_READFUNCTION, readfile);
	curl_easy_setopt(easy_handle, CURLOPT_READDATA, fp);
	// 设置文件大小，确保协议知道文件已经传输成功
	curl_easy_setopt(easy_handle, CURLOPT_INFILESIZE, filesize);
	// 设置ftp的密码，不设置也可以使用匿名用户登陆，匿名用户要求有写权限
	curl_easy_setopt(easy_handle, CURLOPT_USERPWD, "gwq5210:pwd");
	// 访问代理服务器的账号密码
	//curl_easy_setopt(easy_handle, CURLOPT_PROXYUSERPWD, "gwq5210:pwd");
	// 使用ssl时，提供一个私钥用于数据安全通道
	//curl_easy_setopt(easy_handle, CURLOPT_KEYPASSWD, "keypassword");
	// 调试
	//curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 1);
	//curl_easy_setopt(easy_handle, CURLOPT_HEADER, 1);
	printf("网址为：%s\n", url);
	printf("文件名为：%s\n", filename);
	printf("文件大小为：%d\n", filesize);

	ret = curl_easy_perform(easy_handle);
	if (ret == CURLE_OK) {
		printf("上传成功！\n");
	} else {
		printf("上传失败！\n");
	}

	fclose(fp);
	curl_easy_cleanup(easy_handle);
	curl_global_cleanup();

	return 0;
}
