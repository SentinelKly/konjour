#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <thread>
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

using char8  = char;

#if defined(_WIN64)
	#define SO_EXT   "dll"
	#define EX_EXT   "exe"
	#define RM_EXEC  "rmdir /Q /s"
	#define CLR_EXEC "cls"
	#define DIR_SEP  "\\"
	#define OUT_NULL "> nul"
#elif defined(_APPLE_)
	#define SO_EXT   "dylib"
	#define EX_EXT   ""
	#define RM_EXEC  "rm -rf ./"
	#define CLR_EXEC "clear"
	#define DIR_SEP  "/"
	#define OUT_NULL "> /dev/null"
#else
	#define SO_EXT   "so"
	#define EX_EXT   ""
	#define RM_EXEC  "rm -rf ./"
	#define CLR_EXEC "clear"
	#define DIR_SEP  "/"
	#define OUT_NULL "> /dev/null"
#endif

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
		std::unordered_map<FieldType, std::string> m_StringFields;
		std::unordered_map<FieldType, std::vector<std::string>> m_VectorFields;
		std::string m_Name;

		bool m_CppMode = false;

	public:
		Artefact(std::string& name) : m_Name{name}{};
		~Artefact() = default;

		void print();
};

class BuildTable
{
	public:
		bool m_ErrorFlag = false;

	private:
		std::vector<Artefact *> m_Artefacts;
		std::string m_Compiler;

	public:
		BuildTable() = default;
		~BuildTable();

		void addArtefact(Artefact *arte);

		void parseConfiguration(std::string& path);
		void printContents();

		void buildArtefacts();
		void compileObject(Artefact *arte);
};