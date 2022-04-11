#include <stdio.h>
#include <stdlib.h>

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

void throw_parsing_error(uint64_t line, uint64_t charpos, uint8_t *token, err_t err)
{
	int8_t errmsg[9999] = {0};

	if (err == 0 || err == 7) sprintf(errmsg, errstrings[err], token);
	else if (err < 7) sprintf(errmsg, errstrings[err], line, charpos, token);
	printf(errmsg);
	exit(err);
}