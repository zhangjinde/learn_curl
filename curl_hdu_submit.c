/*************************************************************************
	> File Name: curl_hdu_submit.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月18日 星期四 21时36分47秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <curl/curl.h>

#define BUFSIZE 1024

/*
 * 模拟登陆要设置cookie，保存登陆信息。
 * post的参数要将url的转义字符转换一下
 */
int main(int argc, char *argv[])
{
	CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
	if (ret != CURLE_OK) {
		error(EXIT_FAILURE, 0, "初始化curl失败！\n");
	}
	CURL *curl = curl_easy_init();
	if (curl == NULL) {
		curl_global_cleanup();
		error(EXIT_FAILURE, 0, "获取curl失败！\n");
	}

	FILE *fp = fopen("login.html", "w");
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	// 不认证ssl证书
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	// 跟踪重定向的信息
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	// 设置cookie信息，否则就不能保存登陆信息
	// 设置读取cookie的文件名
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "hducookie");
	// 设置写入cookie的文件名
	curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "hducookie");

	// 调试
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_URL, "http://acm.hdu.edu.cn/userloginex.php?action=login");
	// 设置参数
	// 不需要这个login参数
	//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "username=username&userpass=password8&login=Sign+In");
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "username=username&userpass=password");

	// 登陆
	ret = curl_easy_perform(curl);
	if (ret != CURLE_OK) {
		printf("登陆失败!5秒后尝试重新登陆。。。\n");
		sleep(5);
		ret = curl_easy_perform(curl);
		if (ret != CURLE_OK) {
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			fclose(fp);
			error(EXIT_FAILURE, 0, "登陆失败！\n");
		}
	}
	printf("登陆成功。。。5秒后提交\n");

	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_URL, "http://acm.hdu.edu.cn/submit.php?action=submit");
	// 设置参数
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "check=0&problemid=1000&language=2&usercode=#include<iostream>aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
	sleep(5);

	fclose(fp);
	fp = fopen("submit.html", "w");
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	// 执行提交
	ret = curl_easy_perform(curl);
	if (ret != CURLE_OK) {
		printf("提交失败！\n");
	} else {
		printf("提交成功！\n");
	}

	// 释放资源
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	fclose(fp);

	return 0;
}
