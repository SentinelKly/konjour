#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "konjour.h"

static const int8_t BINARY_LIST[3][11] = {"executable", "shared", "static"};
static const int8_t FIELD_NAMES[13][8] = 
{
	"name", "binary", "out_dir", "inc_dir", "lib_dir", "libraries",
	"defines", "sources", "cflags", "lflags", "c_std", "cxx_std", "build"
};