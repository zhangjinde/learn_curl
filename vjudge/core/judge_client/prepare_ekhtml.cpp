#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mysql/mysql.h>

#include "judge_client.h"
#include "ekhtml.h"

ekhtml_parser_t *prepare_ekhtml(void *cbdata)
{
	ekhtml_parser_t *ekparser = ekhtml_parser_new(NULL);
	ekhtml_parser_cbdata_set(ekparser, cbdata);
	return ekparser;
}

void cleanup_ekhtml(ekhtml_parser_t *ekparser)
{
	ekhtml_parser_flush(ekparser, 1);
	ekhtml_parser_destroy(ekparser);
}
