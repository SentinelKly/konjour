#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdarg>
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

#define LOG_ERROR(x, ...) logMessage(LogType::ERROR, x, ##__VA_ARGS__)
#define LOG_WARN(x, ...)  logMessage(LogType::WARN , x, ##__VA_ARGS__)
#define LOG_INFO(x, ...)  logMessage(LogType::INFO , x, ##__VA_ARGS__)

enum class FieldType: uint8
{
	//std::string fields
	C_STD, CXX_STD, OUTPUT, CFLAGS, LFLAGS, BINARY, MODE,

	//vector<std::string> fields
	INC_PATHS, LIB_PATHS, SOURCES, DEFINES, LIBS
};

enum class LogType : int8 
{
	ERROR, WARN, INFO
};

void logMessage(LogType type, const std::string& message, ...);

struct Artefact
{
	public:
		std::unordered_map<FieldType, std::string> m_StringFields;
		std::unordered_map<FieldType, std::vector<std::string>> m_VectorFields;

		std::string m_Name;
		uint64 m_Priority = 0;

	public:
		Artefact(std::string& name) : m_Name{name}{};
		~Artefact() = default;

		void print();
};

class BuildTable
{
	private:
		std::map<uint64, Artefact *> m_SortedArtefacts;
		std::unordered_map<std::string, Artefact *> m_Artefacts;
		std::string m_Compiler;

		bool m_ErrorFlag;

	public:
		BuildTable() = default;
		~BuildTable();

		void addArtefact(Artefact *art);
		void sortArtefactsIntoMap();

		void parseConfiguration(std::string& path);
		void printContents();
};