#ifndef _H_KONJOUR_
#define _H_KONJOUR_

#include <stdint.h>

#define LOWER_ASCII(x) (x > 96 && x < 123)
#define F_SIZE 13
#define CTOK_SIZE 999

typedef enum fields_t
{
	F_NAME, F_BINARY,
	F_OUT_DIR, F_INC_DIR, F_LIB_DIR,
	F_LIBS, F_DEFINES, F_SOURCES, F_CFLAGS,
	F_LFLAGS, F_C_STD, F_CXX_STD, F_BUILD
} fields_t;

typedef enum modes_t
{
	M_NORMAL, M_ARTIFACT, M_TEXT, M_VAR, M_VALUE, M_COMMENT
} modes_t;

typedef enum token_t
{
	T_LEFT_BRACE, T_RIGHT_BRACE,
	T_EQUALS, T_QUOTES, T_POUND
} token_t;

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

typedef struct artifact_t
{
	int8_t **fields;
} artifact_t;

typedef struct cfg_obj_t
{
	artifact_t **table;
	int32_t index;
	int8_t *src;
} cfg_obj_t;

typedef struct args_t
{
	artifact_t *art;
	int8_t *src;
	int32_t *cflag;
	int32_t count;
	int8_t *inc_defs;

} args_t;

//CFG PARSER
artifact_t *gen_artifact(const int8_t *name);
void destroy_artifact(artifact_t *art);
int32_t lookup_field(artifact_t *art, const int8_t *name);
int8_t *get_field_name(int32_t field);
int32_t lookup_binary(int8_t *str);
cfg_obj_t *new_config(int8_t *src);
void destroy_config(cfg_obj_t *cfg);
uint64_t lookup_artifact(cfg_obj_t *cfg, int8_t *name);
int32_t parse_config(cfg_obj_t *cfg);
void populate_global(cfg_obj_t *cfg);
int8_t *set_heap_str(int8_t *str);
void validate_artifacts(cfg_obj_t *cfg);

//GCC BUILDER
void gcc_exec_config(cfg_obj_t *cfg);
int8_t *set_compiler(int32_t *cflag, int8_t *str);
void *gcc_gen_build(void *argpr);

//ERROR HANDLER
void add_err_handler(err_t err, uint64_t i1, uint64_t i2, uint8_t *tok);
void del_err_handler(err_handler_t *err);
void query_err_handles();
void print_artifacts(cfg_obj_t *cfg);
void throw_error(err_t err, uint64_t i1, uint64_t i2, int8_t *token);

#endif