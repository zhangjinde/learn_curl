#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "json.h"

int main(int argc, char **argv)
{
	json_object *status, *problemset, *problems, *result, *problem, *contestid, *index;

	MC_SET_DEBUG(1);

	problemset = json_object_from_file("problemset.problems");
	status = json_object_object_get(problemset, "status");
	printf("status = %s\n", json_object_get_string(status));
	result = json_object_object_get(problemset, "result");
	problems = json_object_object_get(result, "problems");
	int i = 0;
	int len = json_object_array_length(problems);
	printf("len = %d.\n", len);
	for (i = 0; i < len; ++i) {
		problem = json_object_array_get_idx(problems, i);
		index = json_object_object_get(problem, "index");
		contestid = json_object_object_get(problem, "contestId");
		printf("problem_id = %3s", json_object_get_string(contestid));
		printf("%s\t", json_object_get_string(index));
	}

	json_object_put(index);
	json_object_put(status);
	json_object_put(problem);
	json_object_put(problems);
	json_object_put(contestid);
	json_object_put(problemset);

	return 0;
}
