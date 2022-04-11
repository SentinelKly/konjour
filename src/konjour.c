//KONJOUR BUILD SYSTEM 0.0.1

//TODO: Multithreaded compilation
//TODO: Add 'directives' variable
//TODO: Prevent rebuilding of unmodified sources
//TODO: implement 'flags'

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "konjour.h"

int8_t *load_file(const int8_t *filename)
{
    int8_t *src = NULL;
    FILE *fptr = fopen(filename, "rb");

    if (!fptr) return NULL;

    if (!fseek(fptr, 0L, SEEK_END))
    {
	   int64_t bsize = ftell(fptr);
	   if (bsize < 0) return NULL;

	   src = malloc(sizeof(int8_t) * (bsize + 1));

	   if (fseek(fptr, 0L, SEEK_SET)) return NULL;

	   uint64_t len = fread(src, sizeof(int8_t), bsize, fptr);
	   if (ferror(fptr)) return NULL;
	   src[len++] = 0;
    }

    fclose(fptr);
    return src;
}

int32_t main(int32_t argc, int8_t const **argv)
{
    //Get path to konjour config
    //Create config object
    //Parse config into artifacts
    //Execute compilation on artifacts
    //Deallocate all resources

    const int8_t *path = NULL;

    if (argc < 2) path = "./konjour.cfg";
    else path = argv[1];

    cfg_obj_t *cfg = new_config(load_file(path));
    if (!cfg->src) throw_parsing_error(0, 0, (int8_t *) path, E_NULL_FILE);

    populate_global(cfg);
    parse_config(cfg);
    validate_artifacts(cfg);
    print_artifacts(cfg);

    gcc_exec_config(cfg);
    destroy_config(cfg);
    return 0;
}