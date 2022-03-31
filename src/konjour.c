//KONJOUR BUILD SYSTEM V0.1.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "cfg_parser.h"

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

    for (int32_t i = 0; i < cfg->count + 1; i++)
    {
        printf("Artifact: %s\n", cfg->table[i]->fields[0]);

        for (int32_t ii = 0; ii < F_SIZE; ii++)
        {
            if (!cfg->table[i]->fields[ii])
            {
                printf("%d: null\n", ii);
                continue;
            }

            printf("%d: %s\n", ii, cfg->table[i]->fields[ii]);
        }
    }

    destroy_config(cfg);
    return 0;
}
