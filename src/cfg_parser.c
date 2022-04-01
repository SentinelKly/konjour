#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "konjour.h"

static int8_t tokens[4] = {'[', ']', '=', '"'};
static int8_t binaries[3][10] = {"executable", "shared", "static"};
static int8_t tok_names[10][8] = 
{
	"name", "binary", "out_dir", "inc_dir", "lib_dir", "libs",
	"sources", "flags", "c_std", "cxx_std"
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
	cfg->count = 0;

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

uint64_t lookup_artifact(cfg_obj_t *cfg, int8_t *name)
{
	for (uint64_t i = 0; i < cfg->count + 1; i++)
	{
		if (strcmp(cfg->table[i]->fields[0], name) == 0)
		{
			return i; 
		}
	}

	cfg->count ++;
	cfg->table = realloc(cfg->table, sizeof(artifact_t*) * (cfg->count + 1));
	cfg->table[cfg->count] = gen_artifact(name);
	return cfg->count;
}

int32_t lookup_field(artifact_t *art, const int8_t *name)
{
	for (int32_t i = 0; i < F_SIZE; i++)
	{
		if (strcmp(tok_names[i], name) == 0)
		{
			return i;
		}
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
		if (strcmp(str, binaries[i]) == 0) return i;
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

	//Parsing mode and current char
	mode_t mode = M_NORMAL;
	int8_t cchar = 0;

	tokpos = reset_token(ctok);

	for (uint64_t i = 0; cfg->src[i] != '\0'; i++)
	{   
		cchar = cfg->src[i];

		//debugging output
		//printf("artifact %d | field %d: %s -> mode %d: %s\n", artifact, field, cfg->table[artifact]->fields[0], mode, ctok);
		//printf("cchar: %c\n\n", cchar);

		if (mode == M_TEXT && cchar != tokens[T_QUOTES])
		{
			ctok[tokpos] = cchar;
			tokpos ++;
		}

		else if (cchar == ' ' || cchar == '\t' || cchar == '\r' || cchar == '\n') continue;

		else if (mode == M_ARTIFACT && cchar != tokens[T_RIGHT_BRACE])
		{
			ctok[tokpos] = cchar;
			tokpos ++;
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
				continue;
			}

			else
			{
				continue;
				//Throw error
			}
		}

		else if (cchar == tokens[T_RIGHT_BRACE])
		{
			if (mode == M_ARTIFACT)
			{
				tokpos ++;
				ctok[tokpos] = '\0';

				artifact = lookup_artifact(cfg, ctok);
				mode = M_NORMAL;

				tokpos = reset_token(ctok);
			}

			else
			{
				continue;
				//throw error
			}
		}

		else if (cchar == tokens[T_EQUALS])
		{
			if (mode == M_VAR)
			{
				mode = M_VALUE;
				field = lookup_field(cfg->table[artifact], ctok);
				tokpos = reset_token(ctok);
			}

			else
			{
				continue;
				//throw error
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

					cfg->table[artifact]->fields[field] = malloc(sizeof(int8_t) * tokpos);
					strcpy(cfg->table[artifact]->fields[field], ctok);
				}

				mode = M_NORMAL;
				tokpos = reset_token(ctok);
			}

			else
			{
				continue;
				//throw error;
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