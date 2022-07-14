#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

#include "../vendor/toml.hpp"

using int8   = signed char;
using int16  = signed short;
using int32  = signed int;
using int64  = signed long long;

using uint8  = unsigned char;
using uint16 = unsigned short;
using uint32 = unsigned int;
using uint64 = unsigned long long;

enum class FieldType: uint8
{
	//std::string fields
	C_STD, CXX_STD, OUTPUT, CFLAGS, LFLAGS, BINARY, MODE,

	//vector<std::string> fields
	INC_PATHS, LIB_PATHS, SOURCES, DEFINES, LIBS
};

struct Artefact
{
	public:
		std::string m_Name;
		std::map<FieldType, std::string> m_StringFields;
		std::map<FieldType, std::vector<std::string>> m_VectorFields;

	public:
		Artefact(std::string&& name) : m_Name{name}{};
};

class BuildTable
{
	private:
	public:
		BuildTable();
};