#ifndef _H_CFG_PARSER_
#define _H_CFG_PARSER_

#define LOWER_ASCII(x) (x > 64 && x < 91)

typedef enum fields_t
{
    F_NAME, F_BINARY,
    F_OUT_DIR, F_INC_DIR, F_LIB_DIR,
    F_SOURCES, F_LIBS, F_FLAGS,
    F_C_STD, F_CXX_STD
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

typedef struct artifact_t
{
    int8_t **fields;
} artifact_t;

typedef struct cfg_obj_t
{
    artifact_t **table;
    int32_t count;
    int8_t *src;
} cfg_obj_t;

static const int8_t tokens[4] = {'[', ']', '=', '"'};

artifact_t *gen_artifact(const int8_t *name);

void destroy_artifact(artifact_t *art);

cfg_obj_t *new_config(int8_t *src);
void destroy_config(cfg_obj_t *cfg);

#endif