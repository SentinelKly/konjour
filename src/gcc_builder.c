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
	int32_t srcs = 0;

	int32_t bin_type = lookup_binary(art->fields[F_BINARY]);

	for (fields_t i = 3; i < F_FLAGS; i++)
	{
		for (int8_t *stok = strtok(art->fields[i], " "); stok != NULL; stok = strtok(NULL, " "))
		{
			if (i == F_SOURCES)
			{
				int8_t exec[999] = {0};
				int8_t comp[4] = "gcc";
				int8_t ext[9] = {0};

				if (bin_type == 1) sprintf(exec, "gcc -c -Wall -Werror -fpic %s -o %s/out%d.o", stok, art->fields[F_OUT_DIR], srcs);
				else sprintf(exec, "gcc -c %s -o %s/out%d.o", stok, art->fields[F_OUT_DIR], srcs);
				printf("%s\n", exec);
				system(exec);
				srcs ++;
				continue;
			}

			//Build linking arguments
			int8_t celement[99];
			size += strlen(inserts[i - 3]) + strlen(stok) + 2;
			buffer = realloc(buffer, sizeof(int8_t) * size);
			sprintf(celement, "%s%s ", inserts[i - 3], stok);
			buffer = strcat(buffer, celement);
		}
	}

	int8_t link_exec[9999] = {0};
	int8_t src_list[999] = {0};

	for (uint64_t i = 0; i < srcs; i++)
	{
		int8_t src[999] = {0};
		sprintf(src, "%s/out%d.o ", art->fields[F_OUT_DIR], i);
		strcat(src_list, src);
	}

	if (bin_type == 0) sprintf(link_exec, "gcc %s %s -o %s/%s.exe", buffer, src_list, art->fields[F_OUT_DIR], art->fields[F_NAME]);
	else if (bin_type == 1) sprintf(link_exec, "gcc -shared %s %s -o %s/lib%s.so", buffer, src_list, art->fields[F_OUT_DIR], art->fields[F_NAME]);
	else if (bin_type == 2) sprintf(link_exec, "ar rcs %s/lib%s.a %s %s", art->fields[F_OUT_DIR], art->fields[F_NAME], art->fields[F_LIBS], src_list);
	printf("%s\n", link_exec);
	system(link_exec);
}
