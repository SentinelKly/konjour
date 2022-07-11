#ifndef _H_KONJOUR_
#define _H_KONJOUR_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "../vendor/tomlc99/toml.h"

#define ARTEFACT_NATIVE  0
#define ARTEFACT_CMAKE   6
#define ARTEFACT_MAKE    5

#define COMPILER_GCC     0
#define COMPILER_CLANG   1

#define TYPE_EXECUTABLE 0
#define TYPE_SHARED     1
#define TYPE_STATIC     2

#define INVALID_ENUM    0xFF

typedef enum {false, true} bool;

/*=======================================
 *            ERROR HANDLER
 *=======================================
*/

typedef enum error_type_t
{
	E_NULL_FILE, E_INVALID_FILE, E_NO_ARTEFACT_ARRAY, E_NO_ARTEFACTS,
	E_NO_ARTEFACT,
} error_type_t;

typedef struct error_t
{
	error_type_t type;
	int8_t *tok;
} error_t;

void add_error(error_type_t type, uint8_t *token);
void delete_error(error_t *err);
void throw_error(error_type_t type, int8_t *token);
void query_errors(void);

/*=======================================
 *             BUILD TABLE
 *=======================================
*/

typedef enum fields_t
{
	F_C_STD, F_CXX_STD, F_OUTPUT, F_CFLAGS, F_LFLAGS,
	F_TYPE, F_MODE, 
	F_INC_PATHS, F_LIB_PATHS, F_SOURCES, F_DEFINES, F_LIBS
} fields_t;

typedef struct art_t
{
	uint8_t *c_std;
	uint8_t *cxx_std;
	uint8_t *output;
	uint8_t *cflags;
	uint8_t *lflags;

	uint8_t type;
	uint8_t mode;

	toml_array_t *inc_paths;
	toml_array_t *lib_paths;
	toml_array_t *sources;
	toml_array_t *defines;
	toml_array_t *libs;

	bool cpp_mode;
	bool rebuild;
} art_t;

typedef struct cmake_art_t
{
	uint8_t *source;
	uint8_t *build;
} cmake_art_t;

typedef struct make_art_t
{
	uint8_t *flags;
} make_art_t;

typedef struct artefact_t
{
	uint8_t *name;
	uint8_t type;

	union 
	{
		art_t native;
		cmake_art_t cmake;
		make_art_t make;
	};
	
} artefact_t;

typedef struct build_table_t
{
	artefact_t **arts;
	uint64_t count;

	uint8_t *make_prefix;
	uint8_t compiler;
	bool verbose;
} build_table_t;

build_table_t *new_build_table(uint8_t *path);
void delete_build_table(build_table_t *table);
void append_new_artefact(build_table_t *table, artefact_t *art);
uint8_t resolve_compiler(uint8_t *compiler);
void parse_and_validate_config(build_table_t *table, uint8_t *path);

uint8_t resolve_artefact_type(uint8_t *new, uint8_t *name);
artefact_t *new_artefact(uint8_t *name, uint8_t type);
void delete_artefact(artefact_t *art);

/*=======================================
 *              UTILITIES
 *=======================================
*/

uint8_t *set_heap_string(uint8_t *str);
bool contains_prefix(const uint8_t *str, const uint8_t *prefix);
uint8_t *string_to_lower(uint8_t *str);

#endif