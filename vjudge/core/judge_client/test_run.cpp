#include "judge_client.h"

int save_custom_input(void)
{
	char src_path[20];
	// get the source code
	if (execute_sql("SELECT input_text FROM custominput WHERE "
			"solution_id=%d", solution->solution_id) < 0) {
		return -1;
	}
	MYSQL_RES *result = mysql_store_result(conn);
	if (result == NULL) {
		write_log("read mysql result error:%s.\n", mysql_error(conn));
		return -1;
	}
	my_ulonglong cnt = mysql_num_rows(result);
	if (cnt > 0) {
		MYSQL_ROW row = mysql_fetch_row(result);
		if (row == NULL) {
			mysql_free_result(result);
			write_log("fetch mysql row error:%s\n", mysql_error(conn));
			return -1;
		}
		// create the src file
		sprintf(src_path, "data.in");
		FILE *fp_src = fopen(src_path, "w");
		if (fp_src == NULL) {
			write_log("can't create file %s.\n", src_path);
			mysql_free_result(result);
			return -1;
		}
		fprintf(fp_src, "%s", row[0]);
		fclose(fp_src);
		write_log("solution_id = %d\n", solution->solution_id);
	} else {
		write_log("no custom intput %d.\n", solution->solution_id);
		mysql_free_result(result);
		return -1;
	}
	mysql_free_result(result);
	return 0;
}

int addcustomout(int solution_id)
{
	return addreinfo(solution_id, "user.out");
}

void test_run(void)
{
	save_custom_input();
	init_syscalls_limits(solution->language);

	pid_t pid = fork();
	if (pid < 0) {
		write_log("test_run fork error:%s.\n", strerror(errno));
		solution->result = OJ_JE;
	} else if (pid == 0) {		//在子进程中
		//运行编译后的程序,生成用户产生的结果user.out文件
		run_solution();
	} else {		//父进程中
		watch_solution(pid, NULL, NULL);
	}
	if (solution->result == OJ_TL) {
		solution->time = solution->problem_info.time_limit * 1000;
	}
	if (solution->result == OJ_RE) {
		addreinfo(solution->solution_id, "error.out");
	} else {
		addcustomout(solution->solution_id);
	}
	solution->result = OJ_TR;
	update_solution();
}
