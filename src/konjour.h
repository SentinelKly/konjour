#ifndef _H_KONJOUR_
#define _H_KONJOUR_

#include <stdint.h>

#define CTOK_SIZE 999

typedef enum fields_t
{
	F_NAME, F_BINARY,
	F_OUT_DIR, F_INC_DIR, F_LIB_DIR,
	F_LIBS, F_DEFINES, F_SOURCES, F_CFLAGS,
	F_LFLAGS, F_C_STD, F_CXX_STD, F_BUILD
} fields_t;

typedef enum err_t
{
	//Initialisation errors
	E_NULL_FILE, 

	//Parsing errors
	E_UNEXPECTED_TOK, E_NULL_ARTIFACT, E_INCOMPLETE_ARTIFACT, E_INVALID_FIELD, 

	//Validation errors
	E_NO_SOURCES, E_NO_ARTIFACTS, E_INVALID_BUILD, E_INVALID_BINARY
} err_t;

typedef struct err_handler_t
{
	err_t err;
	uint64_t i1, i2;
	int8_t *tok;
} err_handler_t;

//ERROR HANDLER
void add_err_handler(err_t err, uint64_t i1, uint64_t i2, uint8_t *tok);
void del_err_handler(err_handler_t *err);
void query_err_handles();
void throw_error(err_t err, uint64_t i1, uint64_t i2, int8_t *token);

#endif