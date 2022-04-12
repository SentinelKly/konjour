#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#include "konjour.h"

#if defined(_WIN64)
	#define SO_EXT "dll"
	#define EX_EXT "exe"
	#define RM_EXEC "del"
	#define DIR_SEP "\\"
#elif defined(_APPLE_)
	#define SO_EXT "dylib"
	#define EX_EXT ""
	#define RM_EXEC "rm"
	#define DIR_SEP "/"
#else
	#define SO_EXT "so"
	#define EX_EXT ""
	#define RM_EXEC "rm"
	#define DIR_SEP "/"
#endif

static int8_t compilers[2][4] = {"gcc", "g++"};
static int8_t inserts[3][3] = {"-I", "-L", "-l"};
static int32_t verbose = 0;

void gcc_exec_config(cfg_obj_t *cfg)
{
	pthread_t threads[cfg->index];
	for (uint64_t i = 1; i < cfg->index + 1; i++)
	{
		pthread_t thread_id;
		pthread_create(&threads[i - 1], NULL, gcc_gen_build, (cfg->table[i]));
	}

	for (int32_t i = 0; i < cfg->index + 1; i++)
	{
		pthread_join(threads[i], NULL);
	}
}

int8_t *set_compiler(int32_t *cflag, int8_t *str)
{
	int8_t tstr[99] = {0};
	uint64_t index = 0;

	for (uint64_t i = strlen(str) - 1; str[i] != '.'; i--)
	{
		tstr[index] = str[i];
		index ++;
	}

	if (strcmp(tstr, "c") == 0) return compilers[0];
	else *cflag = 1; return compilers[1];
}

void *gcc_gen_build(void *argpr)
{
	artifact_t *art = (artifact_t *) argpr;

	int8_t *buffer = malloc(sizeof(int8_t) * 1);
	int8_t *comp = compilers[0];
	uint64_t size = 0;
	int32_t srcs = 0;
	int32_t cflag = 0;

	int32_t bin_type = lookup_binary(art->fields[F_BINARY]);

	int8_t ndir[999] = {0};
	sprintf(ndir, "mkdir %s%s%s", art->fields[F_OUT_DIR], DIR_SEP, art->fields[F_NAME]);
	system(ndir);

	for (fields_t i = 3; i < F_FLAGS; i++)
	{
		for (int8_t *stok = strtok(art->fields[i], " "); stok != NULL; stok = strtok(NULL, " "))
		{
			if (stok[0] == ' ') continue;

			if (i == F_SOURCES)
			{
				int8_t exec[999] = {0};
				int8_t ext[9] = {0};
				int8_t std[99] = {0};

				comp = set_compiler(&cflag, stok);

				if (strcmp(comp, "gcc") == 0)
				{
					strcpy(std, "c");
					strcat(std, art->fields[F_C_STD]);
				}

				else
				{
					strcpy(std, "c++");
					strcat(std, art->fields[F_CXX_STD]);
				}

				if (!strcmp(art->fields[F_BUILD], "release")) strcat(std, "-Wpedantic -O2 -s");
				else strcat(std, " -g");

				if (bin_type == 1) sprintf(exec, "%s -std=%s -c -Wall -Werror -fPIC %s -o %s/%s/out%d.o", comp, std, stok, art->fields[F_OUT_DIR], art->fields[F_NAME], srcs);
				else sprintf(exec, "%s -std=%s -c %s -o %s/%s/out%d.o", comp, std, stok, art->fields[F_OUT_DIR], art->fields[F_NAME], srcs);

				//printf("Compiling %s of artifact %s\n", stok, art->fields[F_NAME]);
				if (verbose) printf("%s\n", exec);

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

	int8_t exec[9999] = {0};
	int8_t src_list[999] = {0};

	for (uint64_t i = 0; i < srcs; i++)
	{
		int8_t src[999] = {0};
		sprintf(src, "%s/%s/out%d.o ", art->fields[F_OUT_DIR], art->fields[F_NAME], i);
		strcat(src_list, src);
	}

	if (bin_type == 0) sprintf(exec, "%s %s %s %s -o %s/%s.%s", compilers[cflag], art->fields[F_FLAGS], buffer, src_list, art->fields[F_OUT_DIR], art->fields[F_NAME], EX_EXT);
	else if (bin_type == 1) sprintf(exec, "%s %s -shared %s %s -o %s/lib%s.%s", compilers[cflag], art->fields[F_FLAGS], buffer, src_list, art->fields[F_OUT_DIR], art->fields[F_NAME], SO_EXT);
	else if (bin_type == 2) sprintf(exec, "ar rcs %s/lib%s.a %s %s", art->fields[F_OUT_DIR], art->fields[F_NAME], art->fields[F_LIBS], src_list);

	if (verbose) printf("%s\n", exec);

	system(exec);
	memset(exec, 0, 9999);
	//sprintf(exec, "cd %s & %s *.o", art->fields[F_OUT_DIR], RM_EXEC);
	//system(exec);

	printf("Compilation of %s finished!\n", art->fields[F_NAME]);
	cflag = 0;

	pthread_exit(NULL);
	return NULL;
}