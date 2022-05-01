#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "konjour.h"

static const int8_t *errstrings[10] = 
{
	"FILE IO ERROR: '%s' is not a valid config path!\n",
	"PARSE ERROR: (%d:%d) -> unexpected token '%s'\n",
	"PARSE ERROR: (%d:%d) -> invalid artifact definition!\n",
	"PARSE ERROR: (%d:%d) -> incomplete artifact declaration!\n",
	"PARSE ERROR: (%d:%d) -> '%s' is not a valid field!\n",
	"VALIDATION ERROR: artifact '%s' has no sources!\n",
	"VALIDATION ERROR: no compilation artifacts!\n",
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

void print_artifacts(cfg_obj_t *cfg)
{
	printf("\nPreparing to summon the following artifacts: \n\n");

    for (int32_t i = 1; i < cfg->index + 1; i++)
    {
        printf("Artifact: %s\n", cfg->table[i]->fields[0]);

        for (int32_t ii = 1; ii < F_SIZE; ii++)
        {
            int8_t *fname = get_field_name(ii);

            if (!cfg->table[i]->fields[ii] || !strcmp(cfg->table[i]->fields[ii], " ") || (ii > F_BINARY && ii < F_BUILD))
                continue;

            printf("%s: %s\n", fname, cfg->table[i]->fields[ii]);
        }

        printf("\n");
    }
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