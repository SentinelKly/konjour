#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "konjour.h"

static int8_t tokens[4] = {'[', ']', '=', '"'};
static int8_t binaries[3][11] = {"executable", "shared", "static"};
static int8_t tok_names[13][8] = 
{
	"name", "binary", "out_dir", "inc_dir", "lib_dir", "libs",
	"defines", "sources", "cflags", "lflags", "c_std", "cxx_std", "build"
};

artifact_t *gen_artifact(const int8_t *name)
{
	artifact_t *art = malloc(sizeof(artifact_t));
	art->fields = malloc(sizeof(int8_t*) * F_SIZE);

	art->fields[F_NAME] = malloc(sizeof(int8_t) * strlen(name));
	strcpy(art->fields[F_NAME], name);

	for (int32_t i = 1; i < F_SIZE; i++)
	{
		art->fields[i] = malloc(sizeof(int8_t) * 1);
		art->fields[i] = 0;
	}

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
	cfg->index = 0;

	cfg->src = src;
	return cfg;
}

void destroy_config(cfg_obj_t *cfg)
{
	for (uint32_t i = 0; i < cfg->index + 1; i++)
	{
	   destroy_artifact(cfg->table[i]);
	}

	free(cfg->src);
	free(cfg);
}

uint64_t lookup_artifact(cfg_obj_t *cfg, int8_t *name)
{
	for (uint64_t i = 0; i < cfg->index + 1; i++)
	{
		if (!strcmp(cfg->table[i]->fields[0], name))
			return i; 
	}

	cfg->index ++;
	cfg->table = realloc(cfg->table, sizeof(artifact_t*) * (cfg->index + 1));
	cfg->table[cfg->index] = gen_artifact(name);
	return cfg->index;
}

int32_t lookup_field(artifact_t *art, const int8_t *name)
{
	for (int32_t i = 0; i < F_SIZE; i++)
	{
		if (!strcmp(tok_names[i], name)) return i;
	}

	return -1;
}

int8_t *get_field_name(int32_t field)
{
	return tok_names[field];
}

int32_t lookup_binary(int8_t *str)
{
	for (int32_t i = 0; i < 3; i++)
	{
		if (!strcmp(str, binaries[i])) return i;
	}

	return -1;
}

uint64_t reset_token(int8_t *tok)
{
	memset(tok, 0, CTOK_SIZE);
	return 0;
}

int32_t parse_config(cfg_obj_t *cfg)
{
	//Token buffer and buffer index
	int8_t *ctok = malloc(sizeof(int8_t) * CTOK_SIZE);
	uint64_t tokpos = 0;

	//Artifact and field index
	uint64_t artifact = 0;
	int32_t field = 0;

	//File positions
	uint64_t line = 1;
	uint64_t chars = 0;

	//Parsing mode and current char
	modes_t mode = M_NORMAL;
	int8_t cchar = 0;

	tokpos = reset_token(ctok);

	for (uint64_t i = 0; cfg->src[i] != '\0'; i++)
	{   
		cchar = cfg->src[i];
		chars ++;

		//debugging output
		//printf("artifact %d | field %d: %s -> mode %d: %s\n", artifact, field, cfg->table[artifact]->fields[0], mode, ctok);
		//printf("cchar: %c\n\n", cchar);

		if (isalpha(cchar) == 1 && (mode == M_VAR || mode == M_ARTIFACT || mode == M_NORMAL))
		{
			cchar = tolower(cchar);
		}

		if (cchar == '\n')
		{
			//Error: line ended without artifact declaration terminator
			if (mode == M_ARTIFACT) add_err_handler(E_INCOMPLETE_ARTIFACT, line, chars, NULL);

			line ++;
			chars = 0;
		}

		else if (cchar == '\r' || cchar == '\t') continue;

		else if (mode == M_TEXT && cchar != tokens[T_QUOTES])
		{
			ctok[tokpos] = cchar;
			tokpos ++;
		}

		else if (cchar == ' ') continue;

		else if (mode == M_ARTIFACT && cchar != tokens[T_RIGHT_BRACE])
		{
			ctok[tokpos] = cchar;
			tokpos ++;

			//Error: artifact name begins with non alphabetic character
			if (!isalpha(ctok[0])) add_err_handler(E_NULL_ARTIFACT, line, chars, NULL);
		}

		else if (mode == M_VAR && cchar != tokens[T_EQUALS])
		{
			ctok[tokpos] = cchar;
			tokpos ++;
		}

		else if (cchar == tokens[T_LEFT_BRACE])
		{
			if (mode == M_NORMAL)
			{
				mode = M_ARTIFACT;
				tokpos = reset_token(ctok);
			}

			else
			{
				//Error: left brace outside of text or artifact scope declaration
				add_err_handler(E_UNEXPECTED_TOK, line, chars, "[");
			}
		}

		else if (cchar == tokens[T_RIGHT_BRACE])
		{
			if (mode == M_ARTIFACT)
			{
				//Error: Artifact declaration ended without a valid name
				if (!tokpos || !strcmp(ctok, "")) 
				{
					add_err_handler(E_NULL_ARTIFACT, line, chars, NULL);
				}

				else 
				{
					tokpos ++;
					ctok[tokpos] = '\0';
					artifact = lookup_artifact(cfg, ctok);
				}

				mode = M_NORMAL;
				tokpos = reset_token(ctok);
			}

			else
			{
				//Error: right brace outside of text or artifact scope declaration
				add_err_handler(E_UNEXPECTED_TOK, line, chars, "]");
			}
		}

		else if (cchar == tokens[T_EQUALS])
		{
			if (mode == M_VAR)
			{
				mode = M_VALUE;
				field = lookup_field(cfg->table[artifact], ctok);

				//Error: field lookup yielded no valid results
				if (field == -1) add_err_handler(E_INVALID_FIELD, line, chars - 1, ctok);

				tokpos = reset_token(ctok);
			}

			else
			{
				//Error: equals token outside of field assign
				add_err_handler(E_UNEXPECTED_TOK, line, chars, "=");
			}
		}

		else if (cchar == tokens[T_QUOTES])
		{
			if (mode == M_VALUE)
			{
				mode = M_TEXT;
				tokpos = reset_token(ctok);
			}

			else if (mode == M_TEXT)
			{
				if (ctok[0] != 0)
				{
					tokpos ++;
					ctok[tokpos] = '\0';

					//cfg->table[artifact]->fields[field] = malloc(sizeof(int8_t) * tokpos);
					//strcpy(cfg->table[artifact]->fields[field], ctok);

					cfg->table[artifact]->fields[field] = set_heap_str(ctok);
				}

				mode = M_NORMAL;
				tokpos = reset_token(ctok);
			}

			else
			{
				//Error: quotes outside of value start or ending
				add_err_handler(E_UNEXPECTED_TOK, line, chars, "\"");
			}
		}

		else if (mode == M_NORMAL && LOWER_ASCII(cchar))
		{
			mode = M_VAR;
			tokpos = reset_token(ctok);
			ctok[0] = cchar;
			tokpos ++;
		}
	}

	free(ctok);
}

int8_t *set_heap_str(int8_t *str)
{
	if (!str) return NULL;
	int8_t *hstr = NULL;

	hstr = malloc(sizeof(int8_t) * strlen(str));
	strcpy(hstr, str);

	return hstr;
}

void populate_global(cfg_obj_t *cfg)
{
	artifact_t *global = cfg->table[0];

	for (int32_t i = 1; i < F_SIZE; i++)
	{
		switch (i)
		{
			case F_BINARY: 
				global->fields[F_BINARY] = set_heap_str("executable");
				break;

			case F_OUT_DIR:
				global->fields[F_OUT_DIR] = set_heap_str("out");
				break;

			case F_C_STD:
				global->fields[F_C_STD] = set_heap_str("11");
				break;

			case F_CXX_STD:
				global->fields[F_CXX_STD] = set_heap_str("14");
				break;

			case F_BUILD:
				global->fields[F_BUILD] = set_heap_str("debug");

			case F_SOURCES: break;

			default: 
				break;
		}

	}
}

void validate_artifacts(cfg_obj_t *cfg)
{
	//Error: config file has no valid artifacts
	if (cfg->index < 1) add_err_handler(E_NO_ARTIFACTS, 0, 0, NULL);

	for (uint64_t i = 1; i < cfg->index + 1; i++)
	{
		artifact_t *art = cfg->table[i];

		for (int32_t ii = 1; ii < F_SIZE; ii++)
		{
			if (!art->fields[ii])
			{
				//Error: Artifact has no sources present
				if (ii == F_SOURCES) add_err_handler(E_NO_SOURCES, 0, 0, art->fields[0]);

				if (cfg->table[0]->fields[ii]) art->fields[ii] = set_heap_str(cfg->table[0]->fields[ii]);
				else art->fields[ii] = set_heap_str(" ");
			}

			switch (ii)
			{
				case F_BINARY:

					//Error: Artifact binary is of invalid type
					if (strcmp(art->fields[F_BINARY], binaries[0]) && strcmp(art->fields[F_BINARY], binaries[1]) && strcmp(art->fields[F_BINARY], binaries[2]))
						add_err_handler(E_INVALID_BINARY, 0, 0, art->fields[F_BINARY]);
					break;

				case F_BUILD:

					//Error: Artifact build is of invalid type
					if (strcmp(art->fields[F_BUILD], "debug") && strcmp(art->fields[F_BUILD], "release"))
						add_err_handler(E_INVALID_BUILD, 0, 0, art->fields[F_BUILD]);
					break;
			}
		}
	}
}