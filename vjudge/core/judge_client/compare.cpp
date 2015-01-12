#include "judge_client.h"

const char *getFileNameFromPath(const char *path)
{
	int i = 0;
	for (i = strlen(path); i >= 0; i--) {
		if (path[i] == '/')
			return &path[i];
	}
	return path;
}

void make_diff_out(FILE * f1, FILE * f2, int c1, int c2, const char *path)
{
	FILE *out;
	char buf[45];
	out = fopen("diff.out", "a+");
	fprintf(out, "=================%s\n", getFileNameFromPath(path));
	fprintf(out, "Right:\n%c", c1);
	if (fgets(buf, 44, f1)) {
		fprintf(out, "%s", buf);
	}
	fprintf(out, "\n-----------------\n");
	fprintf(out, "Your:\n%c", c2);
	if (fgets(buf, 44, f2)) {
		fprintf(out, "%s", buf);
	}
	fprintf(out, "\n=================\n");
	fclose(out);
}

void find_next_nonspace(int *c1, int *c2, FILE **f1, FILE **f2, int *ret)
{
	// Find the next non-space character or \n.
	while ((isspace(*c1)) || (isspace(*c2))) {
		if (*c1 != *c2) {
			if (*c2 == EOF) {
				do {
					*c1 = fgetc(*f1);
				} while (isspace(*c1));
				continue;
			} else if (*c1 == EOF) {
				do {
					*c2 = fgetc(*f2);
				} while (isspace(*c2));
				continue;
			} else if ((*c1 == '\r' && *c2 == '\n')) {
				*c1 = fgetc(*f1);
			} else if ((*c2 == '\r' && *c1 == '\n')) {
				*c2 = fgetc(*f2);
			} else {
				if (DEBUG)
					printf("%d=%c\t%d=%c", *c1, *c1, *c2, *c2);
				;
				*ret = OJ_PE;
			}
		}
		if (isspace(*c1)) {
			*c1 = fgetc(*f1);
		}
		if (isspace(*c2)) {
			*c2 = fgetc(*f2);
		}
	}
}

/*
 * translated from ZOJ judger r367
 * http://code.google.com/p/zoj/source/browse/trunk/judge_client/client/text_checker.cc#25
 *
 */
int compare_zoj(const char *file1, const char *file2)
{
	int ret = OJ_AC;
	int c1, c2;
	FILE *f1, *f2;
	f1 = fopen(file1, "r");
	f2 = fopen(file2, "r");
	if (!f1 || !f2) {
		ret = OJ_RE;
	} else {
		for (;;) {
			// Find the first non-space character at the beginning of line.
			// Blank lines are skipped.
			c1 = fgetc(f1);
			c2 = fgetc(f2);
			find_next_nonspace(&c1, &c2, &f1, &f2, &ret);
			// Compare the current line.
			for (;;) {
				// Read until 2 files return a space or 0 together.
				while ((!isspace(c1) && c1)
				       || (!isspace(c2) && c2)) {
					if (c1 == EOF && c2 == EOF) {
						goto end;
					}
					if (c1 == EOF || c2 == EOF) {
						break;
					}
					if (c1 != c2) {
						// Consecutive non-space characters should be all exactly the same
						ret = OJ_WA;
						goto end;
					}
					c1 = fgetc(f1);
					c2 = fgetc(f2);
				}
				find_next_nonspace(&c1, &c2, &f1, &f2, &ret);
				if (c1 == EOF && c2 == EOF) {
					goto end;
				}
				if (c1 == EOF || c2 == EOF) {
					ret = OJ_WA;
					goto end;
				}

				if ((c1 == '\n' || !c1) && (c2 == '\n' || !c2)) {
					break;
				}
			}
		}
	}
end:	if (ret == OJ_WA)
		make_diff_out(f1, f2, c1, c2, file1);
	if (f1)
		fclose(f1);
	if (f2)
		fclose(f2);
	return ret;
}

void delnextline(char s[])
{
	int L;
	L = strlen(s);
	while (L > 0 && (s[L - 1] == '\n' || s[L - 1] == '\r'))
		s[--L] = 0;
}

int compare(const char *file1, const char *file2)
{
#ifdef ZOJ_COM
	//compare ported and improved from zoj don't limit file size
	return compare_zoj(file1, file2);
#endif
#ifndef ZOJ_COM
	//the original compare from the first version of hustoj has file size limit
	//and waste memory
	FILE *f1, *f2;
	char *s1, *s2, *p1, *p2;
	int PEflg;
	s1 = new char[STD_F_LIM + 512];
	s2 = new char[STD_F_LIM + 512];
	if (!(f1 = fopen(file1, "r")))
		return OJ_AC;
	for (p1 = s1; EOF != fscanf(f1, "%s", p1);)
		while (*p1)
			p1++;
	fclose(f1);
	if (!(f2 = fopen(file2, "r")))
		return OJ_RE;
	for (p2 = s2; EOF != fscanf(f2, "%s", p2);)
		while (*p2)
			p2++;
	fclose(f2);
	if (strcmp(s1, s2) != 0) {
		//printf("A:%s\nB:%s\n",s1,s2);
		delete[]s1;
		delete[]s2;

		return OJ_WA;
	} else {
		f1 = fopen(file1, "r");
		f2 = fopen(file2, "r");
		PEflg = 0;
		while (PEflg == 0 && fgets(s1, STD_F_LIM, f1)
		       && fgets(s2, STD_F_LIM, f2)) {
			delnextline(s1);
			delnextline(s2);
			if (strcmp(s1, s2) == 0)
				continue;
			else
				PEflg = 1;
		}
		delete[]s1;
		delete[]s2;
		fclose(f1);
		fclose(f2);
		if (PEflg)
			return OJ_PE;
		else
			return OJ_AC;
	}
#endif
}
