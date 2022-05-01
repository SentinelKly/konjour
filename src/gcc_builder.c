#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

#include "konjour.h"

#if defined(_WIN64)
	#define SO_EXT "dll"
	#define EX_EXT "exe"
	#define RM_EXEC "rmdir /Q /s "
	#define DIR_SEP "\\"
#elif defined(_APPLE_)
	#define SO_EXT "dylib"
	#define EX_EXT ""
	#define RM_EXEC "rm -rf ./"
	#define DIR_SEP "/"
#else
	#define SO_EXT "so"
	#define EX_EXT ""
	#define RM_EXEC "rm -rf ./"
	#define DIR_SEP "/"
#endif

static int8_t compilers[2][4] = {"gcc", "g++"};
static int8_t inserts[4][3] = {"-I", "-L", "-l", "-D"};
static int32_t verbose = 0;

void gcc_exec_config(cfg_obj_t *cfg)
{
	clock_t start, end;
	double cputime;
	
	for (uint64_t i = 1; i < cfg->index + 1; i++)
	{
		start = clock();
		gcc_gen_build(cfg->table[i]);
		end = clock();

		cputime = ((double) (end - start)) / CLOCKS_PER_SEC;
		printf("Build completed in [%.2f] secs.\n\n", cputime);
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

	if (!strcmp(tstr, "ppc") || !strcmp(tstr, "cc") || !strcmp(tstr, "xxc"))
	{
		*cflag = 1; 
		return compilers[1];
	} 

	else if (!strcmp(tstr, "c") || !strcmp(tstr, "s")) return compilers[0];

	//TODO: throw warning and default to C compiler
}

void *compile_object(void *vparg)
{
	args_t *arg = (args_t *) vparg;

	int32_t bin_type = lookup_binary(arg->art->fields[F_BINARY]);
	int8_t *comp = set_compiler((int *) arg->cflag, arg->src);

	int8_t exec[999] = {0};
	int8_t std[99] = {0};

	printf("Compiling '%s' from artifact %s\n", arg->src, arg->art->fields[F_NAME]);

	if (!strcmp(comp, "gcc"))
	{
		strcpy(std, "c");
		strcat(std, arg->art->fields[F_C_STD]);
	}

	else
	{
		strcpy(std, "c++");
		strcat(std, arg->art->fields[F_CXX_STD]);
	}

	if (!strcmp(arg->art->fields[F_BUILD], "release")) strcat(std, " -O2 -s");
	else strcat(std, " -g");

	if (bin_type == 1) sprintf(exec, "%s %s %s -std=%s -c -Wall -Werror -fPIC %s -o %s/%s/out%d.o", comp, arg->art->fields[F_CFLAGS], arg->inc_defs, std, arg->src, arg->art->fields[F_OUT_DIR], arg->art->fields[F_NAME], arg->count);
	else sprintf(exec, "%s %s %s -std=%s -c %s -o %s/%s/out%d.o", comp, arg->art->fields[F_CFLAGS], arg->inc_defs, std, arg->src, arg->art->fields[F_OUT_DIR], arg->art->fields[F_NAME], arg->count);

	if (verbose) printf("%s\n", exec);

	if (system(exec));

	pthread_exit(NULL);
	return NULL;
}

void gcc_gen_build(artifact_t *art)
{
	int8_t *buffer = malloc(sizeof(int8_t) * 1);
	uint64_t size = 0;
	int32_t srcs = 0;
	int32_t cflag = 0;
	int32_t success = 1;

	int8_t inc_defs[9999] = {0};
	int32_t bin_type = lookup_binary(art->fields[F_BINARY]);

	int8_t ndir[999] = {0};
	sprintf(ndir, "mkdir %s%s%s", art->fields[F_OUT_DIR], DIR_SEP, art->fields[F_NAME]);
	system(ndir);

	pthread_t threads[99];
	args_t args[99];

	for (fields_t i = 3; i < F_CFLAGS; i++)
	{
		for (int8_t *stok = strtok(art->fields[i], " "); stok != NULL; stok = strtok(NULL, " "))
		{
			if (stok[0] == ' ') continue;

			if (i == F_SOURCES)
			{
				args[srcs] = (args_t) {art, stok, &cflag, srcs, inc_defs};
				pthread_create(&threads[srcs], NULL, compile_object, (void *) &args[srcs]);
				srcs ++;
				continue;
			}

			if (i == F_INC_DIR || i == F_DEFINES)
			{
				int8_t inter[999] = {0};

				if (i == F_INC_DIR) sprintf(inter, "%s%s ", inserts[i - 3], stok);
				else sprintf(inter, "%s %s ", inserts[i - 3], stok);

				if (!inc_defs) strcpy(inc_defs, inter);
				else strcat(inc_defs, inter);
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

	for (int32_t i = 0; i < srcs + 1; i++) pthread_join(threads[i], NULL);

	int8_t exec[9999] = {0};
	int8_t src_list[999] = {0};

	for (uint64_t i = 0; i < srcs; i++)
	{
		int8_t src[999] = {0};
		sprintf(src, "%s/%s/out%d.o ", art->fields[F_OUT_DIR], art->fields[F_NAME], i);
		strcat(src_list, src);
	}

	if (bin_type == 0) sprintf(exec, "%s %s %s %s -o %s/%s.%s", compilers[cflag], art->fields[F_LFLAGS], src_list, buffer, art->fields[F_OUT_DIR], art->fields[F_NAME], EX_EXT);
	else if (bin_type == 1) sprintf(exec, "%s %s -shared %s %s -o %s/lib%s.%s", compilers[cflag], art->fields[F_LFLAGS], src_list, buffer, art->fields[F_OUT_DIR], art->fields[F_NAME], SO_EXT);
	else if (bin_type == 2) sprintf(exec, "ar rcs %s/lib%s.a %s %s", art->fields[F_OUT_DIR], art->fields[F_NAME], art->fields[F_LIBS], src_list);

	if (verbose) printf("%s\n", exec);

	if (system(exec)) success = 0;

	memset(exec, 0, 9999);
	sprintf(exec, "%s%s%s%s", RM_EXEC, art->fields[F_OUT_DIR], DIR_SEP, art->fields[F_NAME]);
	system(exec);

	if (success) printf("Compilation of %s finished!\n", art->fields[F_NAME]);
	else printf("Compilation of %s has failed!\n", art->fields[F_NAME]);

	cflag = 0;
}