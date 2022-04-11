#ifndef _H_KONJOUR_
#define _H_KONJOUR_

#include <stdint.h>

#define LOWER_ASCII(x) (x > 96 && x < 123)
#define F_SIZE 11
#define CTOK_SIZE 999

typedef enum fields_t
{
	F_NAME, F_BINARY,
	F_OUT_DIR, F_INC_DIR, F_LIB_DIR,
	F_LIBS, F_SOURCES, F_FLAGS,
	F_C_STD, F_CXX_STD, F_BUILD
} fields_t;

typedef enum mode_t
{
	M_NORMAL, M_ARTIFACT, M_TEXT, M_VAR, M_VALUE
} mode_t;

typedef enum token_t
{
	T_LEFT_BRACE, T_RIGHT_BRACE,
	T_EQUALS, T_QUOTES
} token_t;

typedef enum err_t
{
	//Initialisation errors
	E_NULL_FILE, 

	//Parsing errors
	E_UNEXPECTED_TOK, E_NULL_ARTIFACT, E_INCOMPLETE_ARTIFACT, E_INVALID_FIELD, 
	E_NULL_FIELD, E_INVALID_VALUE, 

	//Validation errors
	E_NO_SOURCES, E_NO_ARTIFACTS, E_INVALID_BUILD
} err_t;

typedef struct err_handler_t
{
	err_t type;
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
int8_t *set_compiler(int8_t *str);
void gcc_gen_build(artifact_t *art);

//ERROR HANDLER
void print_artifacts(cfg_obj_t *cfg);
void throw_parsing_error(uint64_t line, uint64_t charpos, uint8_t *token, err_t err);

#endif