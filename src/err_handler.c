#include <stdio.h>
#include <stdlib.h>

#include "konjour.h"

static const int8_t *errstrings[2] = 
{
	"PARSE ERROR: (%d:%d) -> unexpected token '%s'!",
	"PARSE ERROR: (%d:%d) -> '%s' is not a valid field!"
};

void throw_parsing_error(uint64_t line, uint64_t charpos, uint8_t *token, err_t err)
{
	int8_t errmsg[9999] = {0};
	sprintf(errmsg, errstrings[err - 1], line, charpos, token);
	printf(errmsg);
	exit(err);
}