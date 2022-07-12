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
static const int8_t NATIVE_FIELDS[12][10] = 
{
	"c_std", "cxx_std", "output", "cflags", "lflags", "binary", "mode",
	"inc_paths", "lib_paths", "sources", "defines", "libs"
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
 *              KSTRINGS
 *=======================================
*/

kstring_array_t *new_kstring_array()
{
	kstring_array_t *array = malloc(sizeof(kstring_array_t) * 1);
	array->elements = NULL;
	array->count = 0;

	return array;
}

kstring_array_t *append_kstring_array(kstring_array_t *array, kstring_t *str)
{
	if (!str) return array;

	array->count ++;
	array->elements = realloc(array->elements, sizeof(kstring_t *) * array->count);
	array->elements[array->count - 1] = str;
	return array;
}

kstring_array_t *new_kstring_array_from_toml(toml_array_t *tarray)
{
	kstring_array_t *array = new_kstring_array();

	if (tarray)
	{
		for (uint64_t i = 0;; i++)
		{
			toml_datum_t elem = toml_string_at(tarray, i);
			if (!elem.ok) break;
			array = append_kstring_array(array, new_kstring(elem.u.s));
		}
	}

	return array;
}

void delete_kstring_array(kstring_array_t *array)
{
    if (!array) return;
	for (uint64_t i = 0; i < array->count; i++) delete_kstring(array->elements[i]);
	free(array);
}

kstring_t *new_kstring(uint8_t *chars)
{
	kstring_t *str = malloc(sizeof(kstring_t) * 1);
	str->ptr = NULL;
	str->size = 0;

	if (chars) 
	{
		str->size = strlen(chars);
		str->ptr  = malloc(sizeof(uint8_t) * str->size);
		str->ptr  = strcpy(str->ptr, chars);
	}

	return str;
}

kstring_t *new_kstring_from_toml(toml_datum_t chars)
{
	kstring_t *str = NULL;
	if (chars.ok) str = new_kstring(chars.u.s);
	return str;
}

void delete_kstring(kstring_t *str)
{
    if (!str) return;
	free(str->ptr);
	free(str);
}

/*=======================================
 *             BUILD TABLE
 *=======================================
*/

build_table_t *new_build_table(uint8_t *path)
{
	build_table_t *table = malloc(sizeof(build_table_t) * 1);

	table->count       = 0;
	table->compiler    = COMPILER_GCC;
	table->arts        = NULL;
	table->make_prefix = NULL;
	table->verbose     = false;

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

void parse_and_validate_config(build_table_t *table, uint8_t *path)
{
	FILE *fp = fopen(path, "r");
	if (!fp) throw_error(E_NULL_FILE, path);

	uint8_t error_buffer[200] = {0};
	toml_table_t* conf = toml_parse_file(fp, error_buffer, sizeof(error_buffer));
	fclose(fp);

	if (!conf) throw_error(E_INVALID_FILE, error_buffer);

	toml_array_t *artefacts         = toml_array_in(conf, "artefacts");
	toml_datum_t global_compiler    = toml_string_in(conf, "KONJOUR_COMPILER");
	toml_datum_t global_make_prefix = toml_string_in(conf, "KONJOUR_MAKE_PREFIX");
	toml_datum_t global_verbose     = toml_bool_in(conf, "KONJOUR_VERBOSE");

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
				delete_artefact(art);
				continue;
			}

			if (art->type == ARTEFACT_CMAKE)
			{
				art->cmake.source    = new_kstring_from_toml(toml_string_in(art_table, "source"));
				art->cmake.generator = new_kstring_from_toml(toml_string_in(art_table, "generator"));
				art->cmake.output    = new_kstring_from_toml(toml_string_in(art_table, "output"));
			}

			else if (art->type == ARTEFACT_MAKE)
				art->make.flags = new_kstring_from_toml(toml_string_in(art_table, "flags"));

			else
			{
				art->native.c_std   = new_kstring_from_toml(toml_string_in(art_table, NATIVE_FIELDS[F_C_STD]));
				art->native.cxx_std = new_kstring_from_toml(toml_string_in(art_table, NATIVE_FIELDS[F_CXX_STD]));
				art->native.output  = new_kstring_from_toml(toml_string_in(art_table, NATIVE_FIELDS[F_OUTPUT]));
				art->native.cflags  = new_kstring_from_toml(toml_string_in(art_table, NATIVE_FIELDS[F_CFLAGS]));
				art->native.lflags  = new_kstring_from_toml(toml_string_in(art_table, NATIVE_FIELDS[F_LFLAGS]));

				toml_datum_t art_binary = toml_string_in(art_table, NATIVE_FIELDS[F_BINARY]);
				if (art_binary.ok) art->native.binary = resolve_artefact_binary(art_binary.u.s);

				toml_datum_t art_mode = toml_string_in(art_table, NATIVE_FIELDS[F_MODE]);
				if (art_mode.ok) art->native.mode = resolve_artefact_mode(art_mode.u.s);

				art->native.inc_paths = new_kstring_array_from_toml(toml_array_in(art_table, NATIVE_FIELDS[F_INC_PATHS]));
				art->native.lib_paths = new_kstring_array_from_toml(toml_array_in(art_table, NATIVE_FIELDS[F_LIB_PATHS]));
				art->native.sources   = new_kstring_array_from_toml(toml_array_in(art_table, NATIVE_FIELDS[F_SOURCES]));
				art->native.defines   = new_kstring_array_from_toml(toml_array_in(art_table, NATIVE_FIELDS[F_DEFINES]));
				art->native.libs      = new_kstring_array_from_toml(toml_array_in(art_table, NATIVE_FIELDS[F_LIBS]));
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

uint8_t resolve_artefact_binary(uint8_t *binary)
{
	if (!binary) return INVALID_ENUM;
	if (!strcmp(binary, "executable")) return BIN_EXECUTABLE;
	else if (!strcmp(binary, "shared")) return BIN_SHARED;
	else if (!strcmp(binary, "static")) return BIN_STATIC;
	else return INVALID_ENUM;
}

uint8_t resolve_artefact_mode(uint8_t *mode)
{
	if (!mode) return INVALID_ENUM;
	if (!strcmp(mode, "debug")) return MODE_DEBUG;
	else if (!strcmp(mode, "release")) return MODE_RELEASE;
	else return INVALID_ENUM;
}

uint8_t resolve_compiler(uint8_t *compiler)
{
	if (!strcmp(compiler, "gcc")) return COMPILER_GCC;
	else if (!strcmp(compiler, "clang")) return COMPILER_CLANG;
	else return INVALID_ENUM;
}

artefact_t *new_artefact(uint8_t *name, uint8_t type)
{
	artefact_t *art = malloc(sizeof(artefact_t) * 1);
	art->name = new_kstring(name);
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
	delete_kstring(art->name);

	if (art->type == ARTEFACT_NATIVE)
	{
		delete_kstring(art->native.c_std);
		delete_kstring(art->native.cxx_std);
		delete_kstring(art->native.output);
		delete_kstring(art->native.cflags);
		delete_kstring(art->native.lflags);

		delete_kstring_array(art->native.inc_paths);
		delete_kstring_array(art->native.lib_paths);
		delete_kstring_array(art->native.sources);
		delete_kstring_array(art->native.defines);
		delete_kstring_array(art->native.libs);
	}

	else if (art->type == ARTEFACT_CMAKE)
	{
		delete_kstring(art->cmake.source);
		delete_kstring(art->cmake.generator);
		delete_kstring(art->cmake.output);
	}

	else if (art->type == ARTEFACT_NATIVE) delete_kstring(art->make.flags);
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
	delete_build_table(table);
}