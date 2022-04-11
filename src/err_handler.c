#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "konjour.h"

static const int8_t *errstrings[9] = 
{
	"FILE IO ERROR: '%s' is not a valid config path!",
	"PARSE ERROR: (%d:%d) -> unexpected token '%s'.",
	"PARSE ERROR: (%d:%d) -> invalid artifact definition!",
	"PARSE ERROR: (%d:%d) -> incomplete artifact declaration!",
	"PARSE ERROR: (%d:%d) -> '%s' is not a valid field!",
	"",
	"",
	"VALIDATION ERROR: artifact '%s' has no sources!",
	"VALIDATION ERROR: no compilation artifacts!"
};

void print_artifacts(cfg_obj_t *cfg)
{
	printf("\nPreparing to summon the following artifacts: \n\n");

    for (int32_t i = 1; i < cfg->index + 1; i++)
    {
        printf("Artifact: %s\n", cfg->table[i]->fields[0]);

        for (int32_t ii = 1; ii < F_SIZE; ii++)
        {
            int8_t *fname = get_field_name(ii);

            if (!cfg->table[i]->fields[ii] || !strcmp(cfg->table[i]->fields[ii], " ") || (ii > F_BINARY && ii < F_C_STD))
            {
                continue;
            }

            printf("%s: %s\n", fname, cfg->table[i]->fields[ii]);
        }

        printf("\n");
    }
}

void throw_parsing_error(uint64_t line, uint64_t charpos, uint8_t *token, err_t err)
{
	int8_t errmsg[9999] = {0};

	if (err == 0) sprintf(errmsg, errstrings[err], token);
	else if (err < 7) sprintf(errmsg, errstrings[err], line, charpos, token);
	else if (err < 9) sprintf(errmsg, errstrings[err], token);
	
	printf(errmsg);
	exit(err);
}