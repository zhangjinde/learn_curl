#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "ekhtml.h"
#include "probleminfo.h"

#define MAGIC_DOODIE 0xf9d33bc1

typedef struct {
	unsigned int n_starttags;
	unsigned int n_endtags;
	unsigned int n_comments;
	unsigned int n_data;
	unsigned int magic_doodie;
	unsigned int only_parse;
	unsigned int isdescription;
} tester_cbdata;

static void handle_starttag_way(void *cbdata, ekhtml_string_t * tag,
				ekhtml_attr_t * attrs)
{
	printf("GOT WAY START!\n");
}

// 标签开始
static void handle_starttag(void *cbdata, ekhtml_string_t * tag,
			    ekhtml_attr_t * attrs)
{
	ekhtml_attr_t *attr;
	tester_cbdata *tdata = cbdata;

	assert(tdata->magic_doodie == MAGIC_DOODIE);
	tdata->n_starttags++;
	if (tdata->only_parse)
		return;

	if (strncmp("DIV", tag->str, 3) == 0) {
		if (!tdata->isdescription) {
			return;
		}
		printf("START: \"%.*s\"\n", (int)tag->len, tag->str);
		for (attr = attrs; attr; attr = attr->next) {
			if (strncmp("style", attr->name.str, 5) == 0) {
				printf("ATTRIBUTE: \"%.*s\" = ", (int)attr->name.len,
				       attr->name.str);
				if (!attr->isBoolean) {
					printf("\"%.*s\"\n", (int)attr->val.len, attr->val.str);
				} else {
					printf("\"%.*s\"\n", (int)attr->name.len,
					       attr->name.str);
				}
			}
		}
	}
}

// 标签结束
static void handle_endtag(void *cbdata, ekhtml_string_t * str)
{
	tester_cbdata *tdata = cbdata;

	assert(tdata->magic_doodie == MAGIC_DOODIE);
	tdata->n_endtags++;
	if (tdata->only_parse)
		return;

	if (strncmp("DIV", str->str, 3) == 0) {
		if (tdata->isdescription) {
			tdata->isdescription--;
		}
		printf("END: \"%.*s\"\n", (int)str->len, str->str);
	}
}

// 处理html中的注释
static void handle_comment(void *cbdata, ekhtml_string_t * str)
{
	tester_cbdata *tdata = cbdata;

	assert(tdata->magic_doodie == MAGIC_DOODIE);
	tdata->n_comments++;
	if (tdata->only_parse)
		return;

	//printf("COMMENT: \"%.*s\"\n", (int)str->len, str->str);
}

// 处理标签中的数据
static void handle_data(void *cbdata, ekhtml_string_t * str)
{
	tester_cbdata *tdata = cbdata;

	assert(tdata->magic_doodie == MAGIC_DOODIE);
	tdata->n_data++;
	if (tdata->only_parse)
		return;

	if (tdata->isdescription) {
		fwrite(str->str, str->len, 1, stdout);
	}
	if (strncmp("Problem Description", str->str, 19) == 0) {
		tdata->isdescription = 2;
	}
}

int main(int argc, char *argv[])
{
	tester_cbdata cbdata;
	ekhtml_parser_t *ekparser;
	char *buf;
	size_t nbuf;
	int feedsize;

	if (argc < 2) {
		fprintf(stderr,
			"Syntax: %s <feedsize> [1|0 (to print debug)]\n",
			argv[0]);
		return -1;
	}

	feedsize = atoi(argv[1]);

	ekparser = ekhtml_parser_new(NULL);

	cbdata.n_starttags = 0;
	cbdata.n_endtags = 0;
	cbdata.n_comments = 0;
	cbdata.n_data = 0;
	cbdata.magic_doodie = MAGIC_DOODIE;
	cbdata.only_parse = argc == 3;

	ekhtml_parser_datacb_set(ekparser, handle_data);
	ekhtml_parser_commentcb_set(ekparser, handle_comment);
	ekhtml_parser_startcb_add(ekparser, "WAY", handle_starttag_way);
	ekhtml_parser_startcb_add(ekparser, NULL, handle_starttag);
	ekhtml_parser_endcb_add(ekparser, NULL, handle_endtag);
	ekhtml_parser_cbdata_set(ekparser, &cbdata);
	buf = malloc(feedsize);

	while ((nbuf = fread(buf, 1, feedsize, stdin))) {
		ekhtml_string_t str;

		str.str = buf;
		str.len = nbuf;
		ekhtml_parser_feed(ekparser, &str);
		ekhtml_parser_flush(ekparser, 0);
	}
	ekhtml_parser_flush(ekparser, 1);
	ekhtml_parser_destroy(ekparser);
	free(buf);

	if (argc == 3) {
		fprintf(stderr,
			"# starttags: %u\n"
			"# endtags:   %u\n"
			"# comments:  %u\n"
			"# data:      %u\n", cbdata.n_starttags,
			cbdata.n_endtags, cbdata.n_comments, cbdata.n_data);
	}

	return 0;
}
