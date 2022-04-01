#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "konjour.h"

static int8_t inserts[3][3] = {"-I", "-L", "-l"};
static uint64_t offset = 0;

void gcc_exec_config(cfg_obj_t *cfg)
{
	for (uint64_t i = 1; i < cfg->count + 1; i++)
	{
		gcc_gen_build(cfg->table[i]);
	}
}

void gcc_gen_build(artifact_t *art)
{
	int8_t *buffer = malloc(sizeof(int8_t) * 1);
	uint64_t size = 0;

	for (fields_t i = 3; i < F_SOURCES; i++)
	{
		for (int8_t *stok = strtok(art->fields[i], " "); stok != NULL; stok = strtok(NULL, " "))
		{
			if (i == F_SOURCES)
			{
				//compile each translation unit
			}

			//Build linking arguments
			int8_t celement[99];
			size += strlen(inserts[i - 3]) + strlen(stok) + 2;
			buffer = realloc(buffer, sizeof(int8_t) * size);
			sprintf(celement, "%s%s ", inserts[i - 3], stok);
			buffer = strcat(buffer, celement);
		}

		printf("%s\n", buffer);
	}
}