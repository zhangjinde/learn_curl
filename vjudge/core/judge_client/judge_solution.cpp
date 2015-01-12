#include "judge_client.h"

void fix_java_mis_judge(void) {
	int comp_res = OJ_AC;
	comp_res = execute_cmd("/bin/grep 'Exception'  %s/error.out", work_dir);
	if (!comp_res) {
		write_log("Exception reported\n");
		solution->result = OJ_RE;
	}
	comp_res = execute_cmd("/bin/grep 'java.lang.OutOfMemoryError'  %s/error.out", work_dir);
	if (!comp_res) {
		write_log("JVM need more Memory!");
		solution->result = OJ_ML;
		solution->memory = solution->problem_info.memory_limit * STD_MB;
	}
	comp_res = execute_cmd("/bin/grep 'java.lang.OutOfMemoryError'  %s/user.out", work_dir);
	if (!comp_res) {
		write_log("JVM need more Memory or Threads!");
		solution->result = OJ_ML;
		solution->memory = solution->problem_info.memory_limit * STD_MB;
	}
	comp_res = execute_cmd("/bin/grep 'Could not create'  %s/error.out", work_dir);
	if (!comp_res) {
		write_log("jvm need more resource,tweak -Xmx(OJ_JAVA_BONUS) Settings");
		solution->result = OJ_RE;
	}
}

void judge_solution(char *infile, char *outfile, char *userfile, double num_of_test)
{
	write_log("judge solution %d.\n", solution->solution_id);
	int comp_res;
	if (!oi_mode) {
		num_of_test = 1.0;
	}
	if (solution->result == OJ_AC && solution->time
			> (solution->problem_info.time_limit * 1000
			* (use_max_time ? 1 : num_of_test))) {
		solution->result = OJ_TL;
	}
	if (solution->memory > solution->problem_info.memory_limit * STD_MB) {
		solution->result = OJ_ML;
	}

	// compare
	if (solution->result == OJ_AC) {
		if (solution->problem_info.spj) {
			comp_res = special_judge(infile, outfile, userfile);
			if (comp_res == 0)
				comp_res = OJ_AC;
			else {
				comp_res = OJ_WA;
			}
		} else {
			comp_res = compare(outfile, userfile);
		}
		if (comp_res == OJ_WA) {
			solution->result = OJ_WA;
		} else if (comp_res == OJ_PE) {
			solution->ispe = OJ_PE;
		}
		solution->result = comp_res;
	}
	//jvm popup messages, if don't consider them will get miss-WrongAnswer
	if (solution->language == 3) {
		fix_java_mis_judge();
	}
}
