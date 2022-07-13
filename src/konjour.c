#include "konjour.h"

static const uint8_t *ERROR_STRINGS[] = 
{
	"FILE IO ERROR: '%s' is not a valid config path!\n",

	"PARSING ERROR: %s\n",
	"PARSING ERROR: must have 'artefacts' array defined!\n",
	"PARSING ERROR: nothing defined in 'artefacts'!\n",
	"PARSING ERROR: no artefact definition for '%s'!\n",

	"VALIDATION ERROR: supported compilers: 'gcc' and 'clang'!\n",
	"VALIDATION ERROR: make/cmake artefacts require a global make prefix!\n",
	"VALIDATION ERROR: artefact '%s'; required source field!\n",
	"VALIDATION ERROR: artefact '%s'; required generator field!\n",
	"VALIDATION ERROR: make prefix could not be ran successfully!\n",
	"VALIDATION ERROR: make sure cmake is installed and added to path!\n",
	"VALIDATION ERROR: artefact '%s'; accepted binary types: 'executable', 'shared', and 'static'!\n",
	"VALIDATION ERROR: artefact '%s'; accepted modes: 'debug' and 'release'!\n",
	"VALIDATION ERROR: artefact '%s'; no compilation sources present!\n"
};

static const uint8_t *NATIVE_FIELDS[] = 
{
	"c_std", "cxx_std", "output", "cflags", "lflags", "binary", "mode",
	"inc_paths", "lib_paths", "sources", "defines", "libs"
};

static const uint8_t *BINARY_LIST[]    = {"executable", "shared", "static"};
static const uint8_t *COMPILER_NAMES[] = {"gcc", "g++", "clang", "clang++"};

static error_t **g_errors;
static uint64_t g_error_count = 0;

/*=======================================
 *            ERROR HANDLER
 *=======================================
*/

void add_error(error_type_t type, uint8_t *tok1, uint8_t *tok2)
{
	g_errors = realloc(g_errors, sizeof(error_t*) * (g_error_count + 1));

	g_errors[g_error_count] = malloc(sizeof(error_t));
	g_errors[g_error_count]->tok1 = set_heap_string(tok1);
	g_errors[g_error_count]->tok2 = set_heap_string(tok2);
	g_errors[g_error_count]->type = type;

	g_error_count ++;
}

void throw_error(error_type_t type, uint8_t *tok1, uint8_t *tok2)
{
	int8_t errmsg[9999] = {0};
	sprintf(errmsg, ERROR_STRINGS[type], tok1, tok2);
	
	printf(errmsg);
	exit(type);
}

void delete_error(error_t *err)
{
	if (!err) return;
	free(err->tok1);
	free(err->tok2);
	free(err);
}

uint64_t query_errors(void)
{
	for (uint64_t i = 0; i < g_error_count; i++)
	{
		int8_t errmsg[9999] = {0};
		error_t *handle = g_errors[i];

		sprintf(errmsg, ERROR_STRINGS[handle->type], handle->tok1, handle->tok2);
		printf(errmsg);

		delete_error(g_errors[i]);
	}

	return g_error_count;
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
	table->arts = realloc(table->arts, sizeof(artefact_t *) * table->count);
	table->arts[table->count - 1] = art;
}

void parse_config_into_table(build_table_t *table, uint8_t *path)
{
	FILE *fp = fopen(path, "r");
	if (!fp) throw_error(E_NULL_FILE, path, "");

	uint8_t error_buffer[200] = {0};
	toml_table_t* conf = toml_parse_file(fp, error_buffer, sizeof(error_buffer));
	fclose(fp);

	if (!conf) throw_error(E_INVALID_FILE, error_buffer, "");

	table->make_prefix = new_kstring_from_toml(toml_string_in(conf, "KONJOUR_MAKE_PREFIX"));

	toml_datum_t global_compiler            = toml_string_in(conf, "KONJOUR_COMPILER");
	if (global_compiler.ok) table->compiler = resolve_compiler(string_to_lower(global_compiler.u.s));
	else table->compiler                    = UNSET_ENUM;

	toml_datum_t global_verbose = toml_bool_in(conf, "KONJOUR_VERBOSE");
	if (global_verbose.ok)  table->verbose = global_verbose.u.b;

	toml_array_t *artefacts = toml_array_in(conf, "artefacts");

	if (artefacts)
	{
		for (uint64_t i = 0;; i++) 
		{
			toml_datum_t art_str = toml_string_at(artefacts, i);

			if (!art_str.ok)
			{
				if (i < 1) add_error(E_NO_ARTEFACTS, "", "");
				break;
			}

			uint8_t new_name[999] = {0};
			artefact_t *art = new_artefact(new_name, resolve_artefact_type(new_name, art_str.u.s));

			toml_table_t *art_table = toml_table_in(conf, new_name);

			if (!art_table)
			{
				add_error(E_NO_ARTEFACT, art_str.u.s, "");
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
			{
				art->make.source = new_kstring_from_toml(toml_string_in(art_table, "source"));
				art->make.flags  = new_kstring_from_toml(toml_string_in(art_table, "flags"));
			}

			else
			{
				art->native.c_std   = new_kstring_from_toml(toml_string_in(art_table, NATIVE_FIELDS[F_C_STD]));
				art->native.cxx_std = new_kstring_from_toml(toml_string_in(art_table, NATIVE_FIELDS[F_CXX_STD]));
				art->native.output  = new_kstring_from_toml(toml_string_in(art_table, NATIVE_FIELDS[F_OUTPUT]));
				art->native.cflags  = new_kstring_from_toml(toml_string_in(art_table, NATIVE_FIELDS[F_CFLAGS]));
				art->native.lflags  = new_kstring_from_toml(toml_string_in(art_table, NATIVE_FIELDS[F_LFLAGS]));

				toml_datum_t art_binary = toml_string_in(art_table, NATIVE_FIELDS[F_BINARY]);
				if (art_binary.ok) art->native.binary = resolve_artefact_binary(art_binary.u.s);
				else art->native.binary = UNSET_ENUM;

				toml_datum_t art_mode   = toml_string_in(art_table, NATIVE_FIELDS[F_MODE]);
				if (art_mode.ok) art->native.mode = resolve_artefact_mode(art_mode.u.s);
				else art->native.mode   = UNSET_ENUM;

				art->native.inc_paths = new_kstring_array_from_toml(toml_array_in(art_table, NATIVE_FIELDS[F_INC_PATHS]));
				art->native.lib_paths = new_kstring_array_from_toml(toml_array_in(art_table, NATIVE_FIELDS[F_LIB_PATHS]));
				art->native.sources   = new_kstring_array_from_toml(toml_array_in(art_table, NATIVE_FIELDS[F_SOURCES]));
				art->native.defines   = new_kstring_array_from_toml(toml_array_in(art_table, NATIVE_FIELDS[F_DEFINES]));
				art->native.libs      = new_kstring_array_from_toml(toml_array_in(art_table, NATIVE_FIELDS[F_LIBS]));
			}

			append_new_artefact(table, art);
		}
	}

	else add_error(E_NO_ARTEFACT_ARRAY, "", "");
	toml_free(conf);
}

void validate_table(build_table_t *table)
{
	if (table->compiler == INVALID_ENUM)    add_error(E_UNSUPPORTED_COMPILER, "", "");
	else if (table->compiler == UNSET_ENUM) table->compiler = COMPILER_GCC;

	bool make_flag, cmake_flag = false;

	for (uint64_t i = 0; i < table->count; i++)
	{
		if (table->arts[i]->type == ARTEFACT_CMAKE)
		{
			if (!table->arts[i]->cmake.source)    add_error(E_NO_MAKE_SOURCE,       table->arts[i]->name->ptr, "");
			if (!table->arts[i]->cmake.generator) add_error(E_NO_CMAKE_GENERATOR,   table->arts[i]->name->ptr, "");
			if (!table->arts[i]->cmake.output)    table->arts[i]->cmake.output = new_kstring("bin");

			cmake_flag = 1;
			make_flag  = 1;
		}

		else if (table->arts[i]->type == ARTEFACT_MAKE)
		{
			if (!table->arts[i]->make.source)    add_error(E_NO_MAKE_SOURCE,        table->arts[i]->name->ptr, "");
			if (!table->arts[i]->make.flags)     table->arts[i]->make.flags = new_kstring("");

			make_flag = 1;
		}

		else if (table->arts[i]->type == ARTEFACT_NATIVE)
		{
			if (!table->arts[i]->native.c_std)   table->arts[i]->native.c_std   = new_kstring("11");
			if (!table->arts[i]->native.cxx_std) table->arts[i]->native.cxx_std = new_kstring("11");
			if (!table->arts[i]->native.output)  table->arts[i]->native.output  = new_kstring("bin");
			if (!table->arts[i]->native.cflags)  table->arts[i]->native.cflags  = new_kstring("");
			if (!table->arts[i]->native.lflags)  table->arts[i]->native.lflags  = new_kstring("");

			if (table->arts[i]->native.binary == INVALID_ENUM)    add_error(E_INVALID_BINARY, table->arts[i]->name->ptr, "");
			else if (table->arts[i]->native.binary == UNSET_ENUM) table->arts[i]->native.binary = BIN_EXECUTABLE;

			if (table->arts[i]->native.mode == INVALID_ENUM)      add_error(E_INVALID_MODE,   table->arts[i]->name->ptr, "");
			else if (table->arts[i]->native.mode == UNSET_ENUM)   table->arts[i]->native.mode = MODE_DEBUG;

			if (table->arts[i]->native.sources->count < 1)        add_error(E_NO_SOURCES,     table->arts[i]->name->ptr, "");
		}
	}

	if (make_flag)
	{
		if (!table->make_prefix) add_error(E_REQUIRED_MAKE_PREFIX, "", "");

		else
		{
			uint8_t make_cmd[99] = {0};
			sprintf(make_cmd, "%s %s", table->make_prefix->ptr, "--version");

			printf("\nChecking for make...\n");
			if (system(make_cmd)) add_error(E_MAKE_NOT_FOUND, "", "");
		}
	}

	if (cmake_flag) 
	{
		printf("\nChecking for cmake...\n");
		if (system("cmake --version")) add_error(E_CMAKE_NOT_FOUND, "", "");
		printf("\n");
	}
}

uint8_t resolve_artefact_type(uint8_t *new, uint8_t *name)
{
	uint64_t offset = ARTEFACT_NATIVE;

	if (contains_prefix(name, "cmake@"))     offset = ARTEFACT_CMAKE;
	else if (contains_prefix(name, "make@")) offset = ARTEFACT_MAKE;

	strcpy(new, name + offset);
	return offset;
}

uint8_t resolve_artefact_binary(uint8_t *binary)
{
	if (!binary)                        return INVALID_ENUM;
	if (!strcmp(binary, "executable"))  return BIN_EXECUTABLE;
	else if (!strcmp(binary, "shared")) return BIN_SHARED;
	else if (!strcmp(binary, "static")) return BIN_STATIC;
	else                                return INVALID_ENUM;
}

uint8_t resolve_artefact_mode(uint8_t *mode)
{
	if (!mode)                         return INVALID_ENUM;
	if (!strcmp(mode, "debug"))        return MODE_DEBUG;
	else if (!strcmp(mode, "release")) return MODE_RELEASE;
	else                               return INVALID_ENUM;
}

uint8_t resolve_compiler(uint8_t *compiler)
{
	if (!compiler)                       return INVALID_ENUM;
	if (!strcmp(compiler, "gcc"))        return COMPILER_GCC;
	else if (!strcmp(compiler, "clang")) return COMPILER_CLANG;
	else                                 return INVALID_ENUM;
}

artefact_t *new_artefact(uint8_t *name, uint8_t type)
{
	artefact_t *art = malloc(sizeof(artefact_t) * 1);
	art->name       = new_kstring(name);
	art->type       = type;

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

	else if (art->type == ARTEFACT_NATIVE) 
	{
		delete_kstring(art->make.source);
		delete_kstring(art->make.flags);
	}

	free(art);
}

/*=======================================
 *             BUILD TABLE
 *=======================================
*/

void build_table_artefacts(build_table_t *table)
{

}

/*=======================================
 *              UTILITIES
 *=======================================
*/

uint8_t *set_heap_string(uint8_t *str)
{
	uint8_t *hstr = malloc(sizeof(uint8_t) * strlen(str));
	hstr          = strcpy(hstr, str);
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
	system(CLR_EXEC);

	build_table_t *table = new_build_table(config_path);
	parse_config_into_table(table, config_path);
	validate_table(table);

	if (!query_errors()) build_table_artefacts(table);
	delete_build_table(table);
}