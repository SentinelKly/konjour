#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "konjour.h"

static const int8_t *errstrings[10] = 
{
	"FILE IO ERROR: '%s' is not a valid config path!\n",
	"PARSE ERROR: (%d:%d) -> unexpected token '%s'\n",
	"PARSE ERROR: (%d:%d) -> invalid artefact definition!\n",
	"PARSE ERROR: (%d:%d) -> incomplete artefact declaration!\n",
	"PARSE ERROR: (%d:%d) -> '%s' is not a valid field!\n",
	"VALIDATION ERROR: artefact '%s' has no sources!\n",
	"VALIDATION ERROR: no compilation artefacts!\n",
	"VALIDATION ERROR: '%s' is not a valid build type!\n",
	"VALIDATION ERROR: '%s' is not a valid binary type!\n"
};

static err_handler_t **err_handles;
static uint64_t err_count = 0;

void add_err_handler(err_t err, uint64_t i1, uint64_t i2, uint8_t *tok)
{
	if (!err_handles) err_handles = malloc(sizeof(err_handler_t*) * 1);
	else err_handles = realloc(err_handles, sizeof(err_handler_t*) * (err_count + 1));

	err_handles[err_count] = malloc(sizeof(err_handler_t));
	err_handles[err_count]->tok = set_heap_str(tok);
	err_handles[err_count]->i1 = i1;
	err_handles[err_count]->i2 = i2;
	err_handles[err_count]->err = err;

	err_count ++;
}

void del_err_handler(err_handler_t *err)
{
	free(err->tok);
	free(err);
}

void query_err_handles()
{
	for (uint64_t i = 0; i < err_count; i++)
	{
		int8_t errmsg[9999] = {0};
		err_handler_t *handle = err_handles[i];

		if (handle->err == E_NULL_FILE) sprintf(errmsg, errstrings[handle->err], handle->tok);
		else if (handle->err < E_NO_SOURCES) sprintf(errmsg, errstrings[handle->err], handle->i1, handle->i2, handle->tok);
		else if (handle->err < E_INVALID_BINARY + 1) sprintf(errmsg, errstrings[handle->err], handle->tok);
	
		printf(errmsg);
		del_err_handler(err_handles[i]);
	}

	if (err_count) exit(-1);
}

void throw_error(err_t err, uint64_t i1, uint64_t i2, int8_t *token)
{
	int8_t errmsg[9999] = {0};

	if (err == E_NULL_FILE) sprintf(errmsg, errstrings[err], token);
	else if (err < E_NO_SOURCES) sprintf(errmsg, errstrings[err], i1, i2, token);
	else if (err < E_INVALID_BINARY + 1) sprintf(errmsg, errstrings[err], token);
	
	printf(errmsg);
	exit(err);
}