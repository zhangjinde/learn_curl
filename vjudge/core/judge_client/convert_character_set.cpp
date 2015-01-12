#include "judge_client.h"

int convert(char *buf, size_t len, const char *from, const char *to)
{
	iconv_t cd = iconv_open(to, from);
	if (cd == (iconv_t)-1) {
		write_log("get character set converter error:%s.\n", strerror(errno));
		return -1;
	}
	size_t sz = BUFSIZE * BUFSIZE;
	char *tmp_str = (char *)malloc(sz);
	if (tmp_str == NULL) {
		iconv_close(cd);
		write_log("alloc convert tmp_str buf memory error.\n");
		return -1;
	}
	// 传进去的一定得是别的东西，原来的地址不能被改变
	char *in = buf;
	char *out = tmp_str;
	size_t inlen = len;
	size_t outlen = sz;
	memset(tmp_str, 0, sz);
	if (iconv(cd, &in, &inlen, &out, &outlen) == (size_t)-1) {
		iconv_close(cd);
		free(tmp_str);
		return -1;
	}
	iconv_close(cd);
	strcpy(buf, tmp_str);
	free(tmp_str);
	return 0;
}

int utf2gbk(char *buf, size_t len)
{
	return convert(buf, len, "UTF-8", "GBK");
}

int gbk2utf8(char *buf, size_t len)
{
	return convert(buf, len, "GBK", "UTF-8");
}
