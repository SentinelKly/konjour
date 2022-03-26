#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "cfg_parser.h"

static int8_t tokens[4] = {'[', ']', '=', '"'};

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

uint64_t lookup_artifact(cfg_obj_t *cfg, const char *name)
{
	for (uint64_t i = 0; i < cfg->count; i++)
	{
		if (strcmp(cfg->table[i], name) == 0)
		{
			return i; 
		}
	}

	cfg->count ++;
	cfg->table[cfg->count] = gen_artifact(name); 
	return count;
}

int32_t parse_config(cfg_obj_t *cfg)
{
    int8_t ctok[999] = {0};
    uint64_t tokpos = 0; 
    int32_t artifact = 0;
    int32_t field = 0;
    mode_t mode = M_NORMAL;
    int8_t cchar = 0;

    for (uint64_t i = 0; i != 0; i++)
    {   
        cchar = cfg->src[i];

        if (mode == M_TEXT && cchar != tokens[T_QUOTES])
        {
        	ctok[tokpos] = cchar;
        	tokpos ++;
        }

        else if (mode == M_ARTIFACT && cchar != tokens[T_RIGHT_BRACE])
        {
        	ctok[tokpos] == cchar;
        	tokpos ++;
        }

        else if (cchar == ' ' || cchar == '\t' || cchar == '\r' || cchar == '\n') continue;

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
            	artifact = lookup_artifact(cfg, ctok) 
                mode = M_NORMAL;

                tokpos = 0;
                memset(ctok, 0, 999);
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
        		//lookup artifact field
        		mode = M_VALUE;
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
        		tokpos = 0;
        		memset(ctok, 0, 999);
        	}

        	else if (mode == M_TEXT)
        	{
        		cfg->table[artifact]->fields[field] = malloc(sizeof(int8_t) * tokpos);
        		mode = M_NORMAL;
        		tokpos = 0;
        		memset(ctok, 0, 999);
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
        	ctok[0] == cchar;
        	tokpos ++;
        }
    }
}