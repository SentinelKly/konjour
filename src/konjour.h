#ifndef _H_KONJOUR_
#define _H_KONJOUR_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "../vendor/tomlc99/toml.h"

#define ARTEFACT_NATIVE 0x00
#define ARTEFACT_CMAKE  0x06
#define ARTEFACT_MAKE   0x05

#define COMPILER_GCC    0x00
#define COMPILER_CLANG  0x01

#define BIN_EXECUTABLE  0x00
#define BIN_SHARED      0x01
#define BIN_STATIC      0x02

#define MODE_RELEASE    0x00
#define MODE_DEBUG      0x01

#define INVALID_ENUM    0xFF

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
 *              KSTRINGS
 *=======================================
*/

typedef struct kstring_t
{
	uint8_t *ptr;
	uint64_t size;
} kstring_t;

typedef struct kstring_array_t
{
	kstring_t **elements;
	uint64_t count;
} kstring_array_t;

kstring_t *new_kstring(uint8_t *chars);
void delete_kstring(kstring_t *str);

/*=======================================
 *             BUILD TABLE
 *=======================================
*/

typedef enum fields_t
{
	F_C_STD, F_CXX_STD, F_OUTPUT, F_CFLAGS, F_LFLAGS,
	F_BINARY, F_MODE, 
	F_INC_PATHS, F_LIB_PATHS, F_SOURCES, F_DEFINES, F_LIBS
} fields_t;

typedef struct art_t
{
	kstring_t *c_std;
	kstring_t *cxx_std;
	kstring_t *output;
	kstring_t *cflags;
	kstring_t *lflags;

	uint8_t binary;
	uint8_t mode;

	kstring_array_t *inc_paths;
	kstring_array_t *lib_paths;
	kstring_array_t *sources;
	kstring_array_t *defines;
	kstring_array_t *libs;

	bool cpp_mode;
	bool rebuild;
} art_t;

typedef struct cmake_art_t
{
	kstring_t *source;
	kstring_t *generator;
	kstring_t *output;
} cmake_art_t;

typedef struct make_art_t
{
	kstring_t *flags;
} make_art_t;

typedef struct artefact_t
{
	kstring_t *name;
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

	kstring_t *make_prefix;
	uint8_t compiler;
	bool verbose;
} build_table_t;

build_table_t *new_build_table(uint8_t *path);
void delete_build_table(build_table_t *table);
void append_new_artefact(build_table_t *table, artefact_t *art);
void parse_config_into_table(build_table_t *table, uint8_t *path);
void validate_table(build_table_t *table);

uint8_t resolve_artefact_type(uint8_t *new, uint8_t *name);
uint8_t resolve_artefact_binary(uint8_t *binary);
uint8_t resolve_artefact_mode(uint8_t *mode);
uint8_t resolve_compiler(uint8_t *compiler);

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