#include "konjour.hh"

static const std::string COMPILERS[]  = {"gcc", "g++", "clang", "clang++"};

static const std::string FIELDS[] = 
{
	"c_std", "cxx_std", "output", "cflags", "lflags", "binary", "mode",
	"inc_paths", "lib_paths", "sources", "defines", "libs"
};

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

void BuildTable::buildArtefacts()
{

}

void BuildTable::compileObject(Artefact *arte)
{

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
		table->buildArtefacts();
	}

	delete table;
	return 0;
}