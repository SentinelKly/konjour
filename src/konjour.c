//KONJOUR BUILD SYSTEM 0.0.1

//TODO: replace cfg 'count' -> 'index'
//TODO: use refs to reduce typing out entire memory location
//TODO: rebust error handling
//TODO: copy global artifact fields to null child fields
//TODO: OS specific extensions
//TODO: Multithreaded compilation
//TODO: Automatically execute config in same directory
//TODO: add 'directives' variable
//TODO: Prevent rebuilding of unmodified sources

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "konjour.h"

#define F_SIZE 10

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

    if (argc < 2) return -1; //Show help

    cfg_obj_t *cfg = new_config(load_file(argv[1]));
    if (!cfg->src) return -1; // File error

    parse_config(cfg);

    printf("Preparing to build the following artifacts: \n\n");

    for (int32_t i = 1; i < cfg->count + 1; i++)
    {
        printf("Artifact: %s\n", cfg->table[i]->fields[0]);

        for (int32_t ii = 1; ii < F_SIZE; ii++)
        {
            int8_t *fname = get_field_name(ii);

            if (!cfg->table[i]->fields[ii])
            {
                printf("%s: null\n", fname);
                continue;
            }

            printf("%s: %s\n", fname, cfg->table[i]->fields[ii]);
        }

        printf("\n");
    }

    gcc_exec_config(cfg);
    destroy_config(cfg);
    return 0;
}
