#include "konjour.hh"

static const std::string COMPILERS[]  = {"gcc", "g++", "clang", "clang++"};
static const std::string CPP_EXTS[]   = {"cpp", "cc", "cxx", "c++", "C"};
static const std::string C_EXTS[]     = {"c", "s", "S", "asm"};

static const std::string FIELDS[] = 
{
	"c_std", "cxx_std", "output", "cflags", "lflags", "binary", "mode",
	"inc_paths", "lib_paths", "sources", "defines", "libs"
};

static const std::string GCC_CLANG_FLAGS[] =
{
	" -O3 -DNDEBUG -s ", " -g ", "-std=", "c", "c++", " -c ", " -Wall -Werror -fPIC ", " -o ",
	" ar rcs ", " -shared ", " -D", " -I", " -L", " -l"
};

uint8 Artefact::getCompilerFromExt(const std::string& str)
{
	std::string ext = str.substr(str.find_last_of(".") + 1);
	uint8 result = INVALID_ENUM;

	for (auto token : CPP_EXTS) if (!ext.compare(token)) result = 1;
	for (auto token : C_EXTS)   if (!ext.compare(token)) result = 0;
	
	if (result < INVALID_ENUM && !m_CppMode) this->m_CppMode = result;
	return result;
}

void Artefact::print()
{
	std::cout 
	<< "artefact:     " << m_Name                             << "\n"
	<< "c standard:   " << m_StringFields[FieldType::C_STD]   << "\n"
	<< "cxx standard: " << m_StringFields[FieldType::CXX_STD] << "\n"
	<< "release mode: " << m_StringFields[FieldType::MODE]    << "\n"
	<< std::endl;
}

BuildTable::~BuildTable()
{
	for (auto i : this->m_Artefacts) delete i;
}

void BuildTable::addArtefact(Artefact *arte)
{
	this->m_Artefacts.push_back(arte);
}

void BuildTable::parseConfiguration(std::string& path)
{
	toml::color::enable();
	
	try 
	{
		auto config = toml::parse(path);
		this->m_Compiler = toml::find_or<std::string>(config, "KONJOUR_COMPILER", "gcc");

		if (this->m_Compiler.compare("gcc") && this->m_Compiler.compare("clang"))
		{
			std::cerr << toml::format_error("[error] unsupported compiler",
			config.at("KONJOUR_COMPILER"), "currently supports 'gcc' and 'clang'") << std::endl;

			this->m_ErrorFlag = true;
		}

		std::vector<std::string> artefactList = toml::find<std::vector<std::string>>(config, "artefacts");

		if (artefactList.size() < 1) 
		{
			std::cerr << toml::format_error("[error] no artefacts are defined",
			config.at("artefacts"), "at least one artefact required") << std::endl;

			this->m_ErrorFlag = true;
		}

		for (auto i : artefactList)
		{
			const auto& arteTable = toml::find(config, i);
			auto arte = new Artefact(i);

			arte->m_StringFields[FieldType::C_STD]   = toml::find_or<std::string>(arteTable, FIELDS[(uint8) FieldType::C_STD]   , "11");
			arte->m_StringFields[FieldType::CXX_STD] = toml::find_or<std::string>(arteTable, FIELDS[(uint8) FieldType::CXX_STD] , "11");
			arte->m_StringFields[FieldType::OUTPUT]  = toml::find_or<std::string>(arteTable, FIELDS[(uint8) FieldType::OUTPUT]  , "bin");
			arte->m_StringFields[FieldType::CFLAGS]  = toml::find_or<std::string>(arteTable, FIELDS[(uint8) FieldType::CFLAGS]  , "");
			arte->m_StringFields[FieldType::LFLAGS]  = toml::find_or<std::string>(arteTable, FIELDS[(uint8) FieldType::LFLAGS]  , "");
			arte->m_StringFields[FieldType::BINARY]  = toml::find_or<std::string>(arteTable, FIELDS[(uint8) FieldType::BINARY]  , "executable");
			arte->m_StringFields[FieldType::MODE]    = toml::find_or<std::string>(arteTable, FIELDS[(uint8) FieldType::MODE]    , "debug");

			if (arte->m_StringFields[FieldType::BINARY].compare("executable") && 
				arte->m_StringFields[FieldType::BINARY].compare("shared")     &&
				arte->m_StringFields[FieldType::BINARY].compare("static"))
			{
				std::cerr << toml::format_error("[error] invalid binary type",
				arteTable.at(FIELDS[(uint8) FieldType::BINARY]), "must be 'executable', 'shared', or 'static'") << std::endl;

				this->m_ErrorFlag = true;
			}

			if (arte->m_StringFields[FieldType::MODE].compare("debug") &&
				arte->m_StringFields[FieldType::MODE].compare("release"))
			{
				std::cerr << toml::format_error("[error] invalid binary type",
				arteTable.at(FIELDS[(uint8) FieldType::BINARY]), "must be 'executable', 'shared', or 'static'") << std::endl;

				this->m_ErrorFlag = true;
			}

			arte->m_VectorFields[FieldType::SOURCES] = toml::find<std::vector<std::string>>(arteTable, FIELDS[(uint8) FieldType::SOURCES]);

			if (arteTable.contains(FIELDS[(uint8) FieldType::INC_PATHS]))
				arte->m_VectorFields[FieldType::INC_PATHS] = toml::find<std::vector<std::string>>(arteTable, FIELDS[(uint8) FieldType::INC_PATHS]);
			
			if (arteTable.contains(FIELDS[(uint8) FieldType::LIB_PATHS]))
				arte->m_VectorFields[FieldType::LIB_PATHS] = toml::find<std::vector<std::string>>(arteTable, FIELDS[(uint8) FieldType::LIB_PATHS]);

			if (arteTable.contains(FIELDS[(uint8) FieldType::DEFINES]))
				arte->m_VectorFields[FieldType::DEFINES]   = toml::find<std::vector<std::string>>(arteTable, FIELDS[(uint8) FieldType::DEFINES]);

			if (arteTable.contains(FIELDS[(uint8) FieldType::LIBS]))
				arte->m_VectorFields[FieldType::LIBS]      = toml::find<std::vector<std::string>>(arteTable, FIELDS[(uint8) FieldType::LIBS]);

			this->addArtefact(arte);
		}
	}
	
	catch (std::exception& e) 
	{
		std::cerr << e.what() << std::endl;
		this->m_ErrorFlag = true;
	}
}

void BuildTable::printContents()
{
	std::cout << "--Konjour configuration--\ncompiler: " << this->m_Compiler
	<< "\n\n--Preparing to summon the following artefacts--\n" << std::endl;

	for (const auto& arte : this->m_Artefacts) arte->print();
}

uint8 BuildTable::compilerToOffset()
{
	return (!m_Compiler.compare("clang") ? 2 : 0);
}

void BuildTable::executeConfig()
{
	clock_t start, end;

	for (const auto& arte : this->m_Artefacts)
	{
		start = clock();
		this->buildArtefact(arte);
		end = clock();

		double cpuTime = ((double) (end - start)) / CLOCKS_PER_SEC;

		std::cout.precision(2);
		std::cout << "Build completed in [" << std::fixed << cpuTime << "] secs.\n" << std::endl;
	}
}

static void compileUnit(ThreadArg *arg)
{
	std::string mkdirExec = "mkdir " + arg->m_Arte->m_StringFields[FieldType::OUTPUT] + DIR_SEP + arg->m_Arte->m_Name + OUT_NULL;
	system(mkdirExec.c_str());

	std::stringstream compileExec;
	uint8 compilerMode = arg->m_Arte->getCompilerFromExt(arg->m_String);

	std::cout << "compiling '" << arg->m_String << "' from artefact " << arg->m_Arte->m_Name << std::endl;

	compileExec << COMPILERS[arg->m_CompilerIndex + compilerMode] + " "
				<< arg->m_Arte->m_StringFields[FieldType::CFLAGS];
	
	for (auto i : arg->m_Arte->m_VectorFields[FieldType::INC_PATHS]) compileExec << GCC_CLANG_FLAGS[INC_PREFIX] << i << " ";
	for (auto i : arg->m_Arte->m_VectorFields[FieldType::DEFINES]) compileExec << GCC_CLANG_FLAGS[DEFINE_PREFIX] << i << " ";

	compileExec	<< ((!arg->m_Arte->m_StringFields[FieldType::MODE].compare("release")) ? GCC_CLANG_FLAGS[RELEASE] : GCC_CLANG_FLAGS[DEBUG])
				<< GCC_CLANG_FLAGS[STD_PREFIX]
				<< ((compilerMode) ? GCC_CLANG_FLAGS[STD_CXX] + arg->m_Arte->m_StringFields[FieldType::CXX_STD] : GCC_CLANG_FLAGS[STD_C] + arg->m_Arte->m_StringFields[FieldType::C_STD])
				<< GCC_CLANG_FLAGS[COMPILE] + ((!arg->m_Arte->m_StringFields[FieldType::BINARY].compare("shared")) ? GCC_CLANG_FLAGS[SHARED] : "")
				<< arg->m_String
				<< GCC_CLANG_FLAGS[OUTPUT] 
				<< arg->m_Arte->m_StringFields[FieldType::OUTPUT]
				<< "/" + arg->m_Arte->m_Name + "/out" << arg->m_SourceIndex << ".o";

	std::cout << compileExec.str() << std::endl;
	std::string compileExecStr = compileExec.str();
	system(compileExecStr.c_str());
	delete arg;
}

void BuildTable::buildArtefact(Artefact *arte)
{
	std::stringstream compileExec, libExpr, objExpr;
	std::vector<std::thread> threadPool;

	uint64 index = 0;

	for (const auto& i : arte->m_VectorFields[FieldType::SOURCES])
	{
		auto args = new ThreadArg(arte, i, compilerToOffset(), index++);
		threadPool.push_back(std::thread(compileUnit, args));
	}

	for (auto& thread : threadPool) thread.join();
	threadPool.clear();

	for (const auto& i : arte->m_VectorFields[FieldType::LIB_PATHS]) libExpr << GCC_CLANG_FLAGS[LIB_PATH_PREFIX] << i << " ";
	for (const auto& i : arte->m_VectorFields[FieldType::LIBS]) libExpr << GCC_CLANG_FLAGS[LIB_PREFIX] << i << " ";
	for (uint64 i = 0; i < index; i++) objExpr << arte->m_StringFields[FieldType::OUTPUT] << "/" << arte->m_Name << "/out" << i << ".o" << " ";

	if (!arte->m_StringFields[FieldType::BINARY].compare("static"))
	{
		compileExec << GCC_CLANG_FLAGS[STATIC] + " "
					<< arte->m_StringFields[FieldType::OUTPUT] + "/lib" + arte->m_Name + ".a "
					<< libExpr.str()
					<< objExpr.str();
	}

	else if (!arte->m_StringFields[FieldType::BINARY].compare("shared"))
	{
		compileExec << COMPILERS[arte->m_CppMode + this->compilerToOffset()] + " "
					<< arte->m_StringFields[FieldType::LFLAGS]
					<< GCC_CLANG_FLAGS[SHARED_LINK]
					<< libExpr.str()
					<< objExpr.str()
					<< GCC_CLANG_FLAGS[OUTPUT] + arte->m_StringFields[FieldType::OUTPUT] + "/lib" + arte->m_Name + "." + SO_EXT;
	}

	else if (!arte->m_StringFields[FieldType::BINARY].compare("executable"))
	{
		compileExec << COMPILERS[arte->m_CppMode + this->compilerToOffset()] + " "
					<< arte->m_StringFields[FieldType::LFLAGS]
					<< libExpr.str()
					<< objExpr.str()
					<< GCC_CLANG_FLAGS[OUTPUT] + arte->m_StringFields[FieldType::OUTPUT] + "/" + arte->m_Name + "." + EX_EXT;
	}
	
	std::cout << compileExec.str() << std::endl;
	std::string compileExecStr = compileExec.str();
	system(compileExecStr.c_str());

	compileExecStr = RM_EXEC + arte->m_StringFields[FieldType::OUTPUT] + DIR_SEP + arte->m_Name;
	system(compileExecStr.c_str());
}

int32 main(int32 argc, char8 **argv)
{
	std::string configPath = (argc > 1) ? argv[1] : "./konjour.toml";
	std::cout << std::endl;

	auto table = new BuildTable();
	table->parseConfiguration(configPath);

	if (!table->m_ErrorFlag)
	{
		table->printContents();
		table->executeConfig();
	}

	delete table;
	return 0;
}