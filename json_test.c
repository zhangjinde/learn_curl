#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <string.h>

#include "json.h"

#define BUFSIZE 512

// path format is obj.key or array[i]
const char *json_get(json_object *obj, const char *path)
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
			printf("%d.\n", idx);
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
	return json_object_get_string(obj);
}

int json_get_int(json_object *obj, const char *path)
{
	return atoi(json_get(obj, path));
}

int main(int argc, char **argv)
{
	json_object *status, *problemset, *problems, *result, *problem, *contestid, *index, *stat, *count, *statistics;

	MC_SET_DEBUG(1);

	problemset = json_object_from_file("problemset.problems");
	status = json_object_object_get(problemset, "status");
	printf("status = %s\n", json_object_get_string(status));
	result = json_object_object_get(problemset, "result");
	problems = json_object_object_get(result, "problems");
	statistics = json_object_object_get(result, "problemStatistics");
	int i = 0;
	int len1 = json_object_array_length(problems);
	printf("len1 = %d.\n", len1);
	for (i = 0; i < len1; ++i) {
		problem = json_object_array_get_idx(problems, i);
		stat = json_object_array_get_idx(statistics, i);
		index = json_object_object_get(problem, "index");
		contestid = json_object_object_get(problem, "contestId");
		count = json_object_object_get(stat, "solvedCount");
		printf("problem_id = %3s", json_object_get_string(contestid));
		printf("%s\t", json_object_get_string(index));
		printf("%d\n", json_object_get_int(count));
	}
	printf("status = %s\n", json_get(problemset, "status"));
	printf("result = %s\n", json_get(problemset, "result"));
	printf("result.problems = %s\n", json_get(problemset, "result.problems"));
	printf("result.problems[0] = %s\n", json_get(problemset, "result.problems[0]"));
	printf("result.problems[0] = %s\n", json_get(problems, "[0]"));

	json_object_put(problemset);

	return 0;
}
