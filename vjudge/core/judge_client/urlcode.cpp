#include "judge_client.h"

//copy http://www.geekhideout.com/urlcode.shtml

/* Converts a hex character to its integer value */
char from_hex(char ch)
{
	return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code)
{
	static char hex[] = "0123456789ABCDEF";
	return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str)
{
	char *pstr = str;
	char *buf = (char *)malloc(strlen(str) * 3 + 1);
	char *pbuf = buf;
	if (buf == NULL) {
		write_log("alloc url_encode buf memory error!\n");
		return NULL;
	}
	while (*pstr) {
		if (isalnum(*pstr) || *pstr == '-' || *pstr == '_'
		    || *pstr == '.' || *pstr == '~') {
			*pbuf++ = *pstr;
		} else if (*pstr == ' ') {
			*pbuf++ = '+';
		} else {
			*pbuf++ = '%';
			*pbuf++ = to_hex(*pstr >> 4);
			*pbuf++ = to_hex(*pstr & 15);
		}
		pstr++;
	}
	*pbuf = '\0';
	return buf;
}

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_decode(char *str)
{
	char *pstr = str;
	char *buf = (char *)malloc(strlen(str) + 1);
	char *pbuf = buf;
	if (buf == NULL) {
		write_log("alloc url_decode buf memory error!\n");
		return NULL;
	}
	while (*pstr) {
		if (*pstr == '%') {
			if (pstr[1] && pstr[2]) {
				*pbuf++ = (from_hex(pstr[1]) << 4) | from_hex(pstr[2]);
				pstr += 2;
			}
		} else if (*pstr == '+') {
			*pbuf++ = ' ';
		} else {
			*pbuf++ = *pstr;
		}
		pstr++;
	}
	*pbuf = '\0';
	return buf;
}
