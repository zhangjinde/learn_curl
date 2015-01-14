/*************************************************************************
	> File Name: json.cpp
	> Author: gwq
	> Mail: gwq5210@qq.com 
	> Created Time: 2015年01月14日 星期三 15时14分36秒
 ************************************************************************/

#include "judge_client.h"

// path format is obj.key or array[i]
json_object *json_get_obj(json_object *obj, const char *path)
{
	char key[BUFSIZE];
	const char *end = path;
	while (1) {
		int len = 0;
		while (*end != '.' && *end != '[' && *end != '\0') {
			key[len++] = *end++;
		}
		key[len] = '\0';
		if (len > 0) {
			obj = json_object_object_get(obj, key);
		}
		if (obj == NULL) {
			return NULL;
		}
		if (*end == '\0') {
			break;
		}
		if (*end == '.') {
			++end;
		}
		if (*end == '[') {
			int idx = 0;
			++end;
			while (*end != ']') {
				idx = idx * 10 + *end - '0';
				++end;
			}
			obj = json_object_array_get_idx(obj, idx);
			if (obj == NULL) {
				return NULL;
			}
			if (*(end + 1) == '.') {
				end += 2;
			} else {
				++end;
			}
		}
	}
	return obj;
}

const char *json_get_str(json_object *obj, const char *path)
{
	obj = json_get_obj(obj, path);
	if (obj == NULL) {
		return NULL;
	} else {
		return json_object_get_string(obj);
	}
}

int json_get_int(json_object *obj, const char *path)
{
	const char *p = json_get_str(obj, path);
	if (p == NULL) {
		return 0;
	} else {
		return atoi(p);
	}
}
