#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mysql/mysql.h>

#include "judge_client.h"
#include "ekhtml.h"

int login_hduoj(void)
{
	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_URL, "http://acm.hdu.edu.cn/userloginex.php?action=login");
	// 设置参数
	char post_str[BUFSIZE];
	char filename[BUFSIZE];
	char *html = (char *)malloc(BUFSIZE * BUFSIZE);
	if (html == NULL) {
		write_log("alloc memory error.\n");
		return -1;
	}
	sprintf(filename, "%dlogin.txt", solution->solution_id);
	sprintf(post_str, "username=%s&userpass=%s", vjudge_user, vjudge_passwd);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_str);
	perform_curl(filename);
	load_file(filename, html);

	// modified form bnuoj
	if (strstr(html, "No such user or wrong password.") != NULL
		|| strstr(html, "<b>One or more following ERROR(s) occurred.") != NULL
		|| strstr(html, "<h2>The requested URL could not be retrieved</h2>") != NULL
		|| strstr(html, "<H1 style=\"COLOR: #1A5CC8\" align=center>Sign In Your Account</H1>") != NULL
		|| strstr(html, "PHP: Maximum execution time of") != NULL) {
		free(html);
		return -1;
	}
	free(html);
	return 0;
}

int submit_hduoj()
{
	char *post_str = (char *)malloc(BUFSIZE * BUFSIZE);
	if (post_str == NULL) {
		write_log("alloc memory error.\n");
		return -1;
	}

	memset(post_str, 0, BUFSIZE * BUFSIZE);
	sprintf(post_str, "check=0&problemid=%d&language=%d&usercode=",
			solution->problem_info.origin_id,
			lang_table[solution->problem_info.ojtype][solution->language]);
	// 对其进行url编码
	utf2gbk(solution->src, solution->code_length);
	char *str = url_encode(solution->src);
	if (str == NULL) {
		write_log("encode url error.\n");
		free(str);
		free(post_str);
		return -1;
	}
	strcat(post_str, str);

	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_URL, "http://acm.hdu.edu.cn/submit.php?action=submit");
	// 设置参数
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_str);

	char filename[BUFSIZE];
	sprintf(filename, "%dsubmit.txt", solution->solution_id);
	perform_curl(filename);

	char *html = (char *)malloc(BUFSIZE * BUFSIZE);
	if (html == NULL) {
		write_log("alloc memory error.\n");
		free(str);
		free(post_str);
		return -1;
	}
	load_file(filename, html);

	// modified form bnuoj
	if (strstr(html, "Connect(0) to MySQL Server failed.") != NULL
		|| strstr(html, "<b>One or more following ERROR(s) occurred.") != NULL
		|| strstr(html, "<h2>The requested URL could not be retrieved</h2>") != NULL
		|| strstr(html, "<H1 style=\"COLOR: #1A5CC8\" align=center>Sign In Your Account</H1>") != NULL
		|| strstr(html, "PHP: Maximum execution time of") != NULL
		|| strstr(html, "<DIV>Exercise Is Closed Now!</DIV>") != NULL) {
		free(str);
		free(html);
		free(post_str);
		return -1;
	}

	free(str);
	free(html);
	free(post_str);

	return 0;
}

int get_result_hduoj(void)
{
	char url[BUFSIZE];
	sprintf("http://acm.hdu.edu.cn/status.php?first=&pid=%d&user=%s"
			"&lang=0&status=0", vjudge_user,
			solution->problem_info.origin_id);

	// 设置提交地址
	curl_easy_setopt(curl, CURLOPT_URL, url);

	char filename[BUFSIZE];
	sprintf(filename, "%dstatus.txt", solution->solution_id);

	time_t begin_time = time(NULL);
	int rid = 0;
	char *html = (char *)malloc(BUFSIZE * BUFSIZE);
	if (html == NULL) {
		write_log("alloc memory error.\n");
		return OJ_WT0;
	}
	while (1) {
		if (time(NULL) - begin_time > vj_max_wait_time) {
			write_log("judge time out.\n");
			free(html);
			return OJ_WT0;
		}
		perform_curl(filename);

		load_file(filename, html);
		// modified form bnuoj
		if (strstr(html, "Connect(0) to MySQL Server failed.") != NULL
			|| strstr(html, "<b>One or more following ERROR(s) occurred.") != NULL
			|| strstr(html, "<h2>The requested URL could not be retrieved</h2>") != NULL
			|| strstr(html, "<H1 style=\"COLOR: #1A5CC8\" align=center>Sign In Your Account</H1>") != NULL
			|| strstr(html, "PHP: Maximum execution time of") != NULL
			|| strstr(html, "<DIV>Exercise Is Closed Now!</DIV>") != NULL) {
			write_log("get solution %d status error.\n", solution->solution_id);
			free(html);
			return OJ_WT0;
		} else {
			<tr align=center >
				<td height=22px>12684021</td>
				<td>2015-01-12 02:32:30</td>
				<td><font color=green>Wrong Answer</font></td>
				<td><a href="/showproblem.php?pid=1000">1000</a></td>
				<td>0MS</td>
				<td>1052K</td>
				<td><a href="/viewcode.php?rid=12684021"  target=_blank>136 B</td>
				<td>GCC</td>
				<td class=fixedsize><a href="/userstatus.php?user=zzuvjudge">zzuvjudge</a></td>
			</tr>
		}
	}

	free(html);

	return OJ_AC;
}
