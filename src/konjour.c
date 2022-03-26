//KONJOUR BUILD SYSTEM V0.1.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define F_SIZE 10

typedef enum fields_t
{
    F_NAME, F_BINARY,
    F_OUT_DIR, F_INC_DIR, F_LIB_DIR,
    F_SOURCES, F_LIBS, F_FLAGS,
    F_C_STD, F_CXX_STD
} fields_t;

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

artifact_t *gen_artifact(const int8_t *name)
{
    artifact_t *art = malloc(sizeof(artifact_t));
    art->fields = malloc(sizeof(int8_t*) * F_SIZE);

    art->fields[F_NAME] = malloc(sizeof(int8_t) * strlen(name));
    strcpy(art->fields[F_NAME], name);

    return art;
}

void destroy_artifact(artifact_t *art)
{
    for (uint16_t i = 0; i < F_SIZE; i++)
    {
	free(art->fields[i]);
    }

    free(art->fields);
    free(art);
}

cfg_obj_t *new_config(int8_t *src)
{
    cfg_obj_t *cfg = malloc(sizeof(cfg_obj_t));
    cfg->table = malloc(sizeof(artifact_t*) * 1);
    cfg->table[0] = gen_artifact("global");
    cfg->count = 1;

    cfg->src = src;

    return cfg;
}

void destroy_config(cfg_obj_t *cfg)
{
    for (uint32_t i; i < cfg->count; i++)
    {
	destroy_artifact(cfg->table[i]);
    }

    free(cfg->src);
    free(cfg);
}

int32_t main(int32_t argc, int8_t const **argv)
{
    //Get path to konjour config
    //Create config object
    //Parse config into artifacts
    //Execute compilation on artifacts
    //Deallocate all resources

    return 0;
}
