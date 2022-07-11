#include "konjour.h"

static const int8_t *ERROR_STRINGS[10] = 
{
	"FILE IO ERROR: '%s' is not a valid config path!\n",
	"PARSING ERROR: %s\n",
	"PARSING ERROR: must have 'artefacts' array defined!\n",
	"PARSING ERROR: nothing defined in 'artefacts'!\n",
	"PARSING ERROR: no artefact definition for '%s'!\n",
};

static const int8_t BINARY_LIST[3][11] = {"executable", "shared", "static"};
static const int8_t FIELDS_NATIVE[12][10] = 
{
	"binary", "out_dir", "inc_dir", "lib_dir", "libs",
	"defines", "sources", "cflags", "lflags", "c_std", "cxx_std", "build"
};

/*=======================================
 *            ERROR HANDLER
 *=======================================
*/

static error_t **errors;
static uint64_t error_count = 0;

void add_error(error_type_t type, uint8_t *token)
{
	errors = realloc(errors, sizeof(error_t*) * (error_count + 1));

	errors[error_count] = malloc(sizeof(error_t));
	errors[error_count]->tok = set_heap_string(token);
	errors[error_count]->type = type;

	error_count ++;
}

void delete_error(error_t *err)
{
	free(err->tok);
	free(err);
}

void query_errors(void)
{
	for (uint64_t i = 0; i < error_count; i++)
	{
		int8_t errmsg[9999] = {0};
		error_t *handle = errors[i];

		sprintf(errmsg, ERROR_STRINGS[handle->type], handle->tok);
		printf(errmsg);

		delete_error(errors[i]);
	}

	if (error_count) exit(-1);
}

void throw_error(error_type_t type, int8_t *token)
{
	int8_t errmsg[9999] = {0};
	sprintf(errmsg, ERROR_STRINGS[type], token);
	
	printf(errmsg);
	exit(type);
}

/*=======================================
 *             BUILD TABLE
 *=======================================
*/

build_table_t *new_build_table(uint8_t *path)
{
	build_table_t *table = malloc(sizeof(build_table_t) * 1);

	table->count = 0;
	table->compiler = COMPILER_GCC;
	table->arts = NULL;
	table->make_prefix = NULL;
	table->verbose = false;

	return table;
}

void delete_build_table(build_table_t *table)
{
	for (uint64_t i = 0; i < table->count; i++) delete_artefact(table->arts[i]);

	free(table->arts);
	free(table->make_prefix);
	free(table);
}

void append_new_artefact(build_table_t *table, artefact_t *art)
{
	table->count ++;
	table->arts = realloc(table->arts, table->count);
	table->arts[table->count - 1] = art;
}

uint8_t resolve_compiler(uint8_t *compiler)
{
	if (!strcmp(compiler, "gcc")) return COMPILER_GCC;
	else if (!strcmp(compiler, "clang")) return COMPILER_CLANG;
	else return INVALID_ENUM;
}

void parse_and_validate_config(build_table_t *table, uint8_t *path)
{
	FILE *fp = fopen(path, "r");
	if (!fp) throw_error(E_NULL_FILE, path);

	uint8_t error_buffer[200] = {0};
	toml_table_t* conf = toml_parse_file(fp, error_buffer, sizeof(error_buffer));
	fclose(fp);

	if (!conf) throw_error(E_INVALID_FILE, error_buffer);

	toml_array_t *artefacts = toml_array_in(conf, "artefacts");
	toml_datum_t global_compiler = toml_string_in(conf, "KONJOUR_COMPILER");
	toml_datum_t global_make_prefix = toml_string_in(conf, "KONJOUR_MAKE_PREFIX");
	toml_datum_t global_verbose = toml_bool_in(conf, "KONJOUR_VERBOSE");

	if (global_compiler.ok) table->compiler = resolve_compiler(string_to_lower(global_compiler.u.s));
	if (global_make_prefix.ok) table->make_prefix = set_heap_string(global_make_prefix.u.s);
	if (global_verbose.ok) table->verbose = global_verbose.u.b;

	if (artefacts)
	{
		for (uint64_t i = 0;; i++) 
		{
			toml_datum_t art_str = toml_string_at(artefacts, i);

			if (!art_str.ok)
			{
				if (i < 1) add_error(E_NO_ARTEFACTS, "");
				break;
			}

			uint8_t new_name[999] = {0};
			artefact_t *art = new_artefact(new_name, resolve_artefact_type(new_name, art_str.u.s));

			toml_table_t *art_table = toml_table_in(conf, new_name);

			if (!art_table)
			{
				add_error(E_NO_ARTEFACT, art_str.u.s);
				continue;
			}

			append_new_artefact(table, art);
		}
	}

	else add_error(E_NO_ARTEFACT_ARRAY, "");
	toml_free(conf);
}

uint8_t resolve_artefact_type(uint8_t *new, uint8_t *name)
{
	uint64_t offset = ARTEFACT_NATIVE;

	if (contains_prefix(name, "cmake@")) offset = ARTEFACT_CMAKE;
	else if (contains_prefix(name, "make@")) offset = ARTEFACT_MAKE;

	strcpy(new, name + offset);
	return offset;
}

artefact_t *new_artefact(uint8_t *name, uint8_t type)
{
	artefact_t *art = malloc(sizeof(artefact_t) * 1);
	art->name = set_heap_string(name);
	art->type = type;

	switch (type)
	{
		case ARTEFACT_NATIVE:
			art->native = (art_t) {};
			break;
		
		case ARTEFACT_CMAKE:
			art->cmake = (cmake_art_t) {};
			break;
		
		case ARTEFACT_MAKE:
			art->make = (make_art_t) {};
			break;
	
		default: break;
	}

	return art;
}

void delete_artefact(artefact_t *art)
{
	free(art->name);
	free(art);
}

/*=======================================
 *              UTILITIES
 *=======================================
*/

uint8_t *set_heap_string(uint8_t *str)
{
	uint8_t *hstr = malloc(sizeof(uint8_t) * strlen(str));
	hstr = strcpy(hstr, str);
	return hstr;
}

bool contains_prefix(const uint8_t *str, const uint8_t *prefix)
{
    return strncmp(prefix, str, strlen(prefix)) == 0;
}

uint8_t *string_to_lower(uint8_t *str)
{
	for (int i = 0; str[i]; i++) str[i] = tolower(str[i]);
	return str;
}

/*=======================================
 *           THE ENTRY POINT
 *=======================================
*/

int32_t main(int32_t argc, const int8_t **argv)
{
	uint8_t *config_path = NULL;

	if (argc < 2) config_path = "./konjour.toml";
	else config_path = (uint8_t *) argv[1];

	build_table_t *table = new_build_table(config_path);
	parse_and_validate_config(table, config_path);
	query_errors();
}