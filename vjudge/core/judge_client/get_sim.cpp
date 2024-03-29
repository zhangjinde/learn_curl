#include "judge_client.h"

void copy_ac_src(void)
{
	int lang = solution->language;
	int solution_id = solution->solution_id;
	int problem_id = solution->problem_info.problem_id;
	char src_pth[20];

	write_log("copy solution %d ac src.\n", solution_id);
	sprintf(src_pth, "Main.%s", lang_ext[lang]);
	execute_cmd("/bin/mkdir -p ../data/%d/ac/", problem_id);
	execute_cmd("/bin/cp %s ../data/%d/ac/%d.%s", src_pth, problem_id,
		    solution_id, lang_ext[lang]);
	//c和cpp互相查重
	if (lang == 0) {
		execute_cmd
		    ("/bin/ln ../data/%d/ac/%d.%s ../data/%d/ac/%d.%s",
		     problem_id, solution_id, lang_ext[lang], problem_id, solution_id,
		     lang_ext[lang + 1]);
	}
	if (lang == 1) {
		execute_cmd
		    ("/bin/ln ../data/%d/ac/%d.%s ../data/%d/ac/%d.%s",
		     problem_id, solution_id, lang_ext[lang], problem_id, solution_id,
		     lang_ext[lang - 1]);
	}
}

int update_sim(void)
{
	write_log("update solution %d sim.\n", solution->solution_id);
	FILE *fp = fopen("sim", "r");
	if (fp == NULL) {
		write_log("open file sim error:%s.\n", strerror(errno));
		return -1;
	}
	int s_id = solution->solution_id;
	int sim_s_id, sim;
	while (fscanf(fp, "%d%d", &sim_s_id, &sim) != EOF) {
		if (execute_sql("insert into sim(s_id, sim_s_id, sim) "
				"values(%d,%d,%d) ", s_id, sim_s_id, sim) < 0) {
			return -1;
		}
	}
	fclose(fp);
	return 0;
}

int get_sim(void)
{
	int lang = solution->language;
	int problem_id = solution->problem_info.problem_id;
	char src_pth[20];

	write_log("get solution %d sim.\n", solution->solution_id);
	sprintf(src_pth, "Main.%s", lang_ext[lang]);

	int sim = execute_cmd("/usr/bin/sim.sh %s %d", src_pth, problem_id);
	if (sim) {
		if (update_sim() < 0) {
			return -1;
		}
	}

	return 0;
}
