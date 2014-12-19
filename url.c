/*************************************************************************
	> File Name: url.c
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2014年12月19日 星期五 14时05分41秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>

#define BURSIZE 1024

/*
 * 编码一个url
 * 字符'a'-'z','A'-'Z','0'-'9','.','-','*'和'_' 都不被编码，维持原值
 * 空格' '被转换为加号'+'
 * 其他每个字节都被表示成"%XY"的格式，X和Y分别代表一个十六进制位。编码为UTF-8。
 */
void urlencode(char url[])
{
	int i = 0;
	int len = strlen(url);
	int res_len = 0;
	char res[BURSIZE];
	for (i = 0; i < len; ++) {
		char c = url[i];
		if (('0' <= c && c <= '9') ||
				('a' <= c && c <= 'z') ||
				('A' <= c && c <= 'Z') || c == '/' || c == '.') {
			res[res_len++] = c;
		} else {
			int j = (short int)c;
			if (j < 0)
				j += 256;
			int i1, i0;
			i1 = j / 16;
			i0 = j - i1 * 16;
			res[res_len++] = '%';
			res[res_len++] = char2hex(i1);
			res[res_len++] = char2hex(i0);
			result += dec2hexChar(i1);
			result += dec2hexChar(i0);
		}
	}
	return result;
}

/*
 * 解码url
 */
void urldecode(const string & URL)
{
	string result = "";
	for (unsigned int i = 0; i < URL.size(); i++) {
		char c = URL[i];
		if (c != '%') {
			result += c;
		} else {
			char c1 = URL[++i];
			char c0 = URL[++i];
			int num = 0;
			num += hexChar2dec(c1) * 16 + hexChar2dec(c0);
			result += char (num);
		}
	}
	return result;
}

int main(int argc, char *argv[])
{
	return 0;
}
