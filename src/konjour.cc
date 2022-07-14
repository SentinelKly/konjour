#include "konjour.hh"

static const std::string LOG_LEVELS[] = {"ERROR: ", "WARN: ", "INFO: "};

static const std::string FIELDS[] = 
{
	"c_std", "cxx_std", "output", "cflags", "lflags", "binary", "mode",
	"inc_paths", "lib_paths", "sources", "defines", "libs"
};

void logMessage(LogType type, const std::string& message, ...)
{
	char buffer[999] = {};
	std::va_list parseArgs;
		
	va_start(parseArgs, message);
	std::vsnprintf(buffer, 999, message.c_str(), parseArgs);
	va_end(parseArgs);

	std::cout << LOG_LEVELS[(uint8) type] << buffer << std::endl;
}

void Artefact::print()
{
	std::cout 
	<< "name:         " << m_Name                             << "\n"
	<< "c standard:   " << m_StringFields[FieldType::C_STD]   << "\n"
	<< "cxx standard: " << m_StringFields[FieldType::CXX_STD] << "\n"
	<< "release mode: " << m_StringFields[FieldType::MODE]    << "\n"
	<< std::endl;
}

BuildTable::~BuildTable()
{
	for (auto const& art : this->m_SortedArtefacts)
		delete art.second;

	m_SortedArtefacts.clear();
}

void BuildTable::sortArtefactsIntoMap()
{
	for (auto const& art : this->m_Artefacts)
		this->m_SortedArtefacts[art.second->m_Priority] = art.second;

	m_Artefacts.clear();
}

void BuildTable::addArtefact(Artefact *art)
{
	this->m_Artefacts[art->m_Name] = art;
}

void BuildTable::parseConfiguration(std::string& path)
{
	toml::color::enable();
	
	try 
	{
		auto config = toml::parse(path);
		std::string globalCompiler = toml::find_or<std::string>(config, "KONJOUR_COMPILER", "gcc");
		std::vector<std::string> artefactList = toml::find<std::vector<std::string>>(config, "artefacts");

		if (artefactList.size() < 1) 
		{
			std::cerr << toml::format_error("[error] no artefacts are defined!",
			config.at("artefacts"), "at least one artefact required.") << std::endl;

			this->m_ErrorFlag = true;
		}

		for (auto i : artefactList)
		{
			const auto& arteTable = toml::find(config, i);

			auto art = new Artefact(i);
			art->m_StringFields[FieldType::C_STD]   = toml::find_or<std::string>(arteTable, FIELDS[(uint8) FieldType::C_STD]   , "11");
			art->m_StringFields[FieldType::CXX_STD] = toml::find_or<std::string>(arteTable, FIELDS[(uint8) FieldType::CXX_STD] , "11");
			art->m_StringFields[FieldType::OUTPUT]  = toml::find_or<std::string>(arteTable, FIELDS[(uint8) FieldType::OUTPUT]  , "bin");
			art->m_StringFields[FieldType::CFLAGS]  = toml::find_or<std::string>(arteTable, FIELDS[(uint8) FieldType::CFLAGS]  , "");
			art->m_StringFields[FieldType::LFLAGS]  = toml::find_or<std::string>(arteTable, FIELDS[(uint8) FieldType::LFLAGS]  , "");
			art->m_StringFields[FieldType::BINARY]  = toml::find_or<std::string>(arteTable, FIELDS[(uint8) FieldType::BINARY]  , "executable");
			art->m_StringFields[FieldType::MODE]    = toml::find_or<std::string>(arteTable, FIELDS[(uint8) FieldType::MODE]    , "debug");

			if (arteTable.contains(FIELDS[(uint8) FieldType::INC_PATHS]))
				art->m_VectorFields[FieldType::INC_PATHS] = toml::find<std::vector<std::string>>(arteTable, FIELDS[(uint8) FieldType::INC_PATHS]);
			
			if (arteTable.contains(FIELDS[(uint8) FieldType::LIB_PATHS]))
				art->m_VectorFields[FieldType::LIB_PATHS] = toml::find<std::vector<std::string>>(arteTable, FIELDS[(uint8) FieldType::LIB_PATHS]);
			
			art->m_VectorFields[FieldType::SOURCES] = toml::find<std::vector<std::string>>(arteTable, FIELDS[(uint8) FieldType::SOURCES]);

			if (arteTable.contains(FIELDS[(uint8) FieldType::DEFINES]))
				art->m_VectorFields[FieldType::DEFINES]   = toml::find<std::vector<std::string>>(arteTable, FIELDS[(uint8) FieldType::DEFINES]);

			if (arteTable.contains(FIELDS[(uint8) FieldType::LIBS]))
				art->m_VectorFields[FieldType::LIBS]      = toml::find<std::vector<std::string>>(arteTable, FIELDS[(uint8) FieldType::LIBS]);

			this->addArtefact(art);
		}
	}
	
	catch (std::exception& e) 
	{
		LOG_ERROR(e.what());
		this->m_ErrorFlag = true;
	}
}

void BuildTable::printContents()
{
	for (auto arte : this->m_Artefacts)
	{
		arte.second->print();
	}
}

int32 main(int32 argc, char8 **argv)
{
	std::string configPath = (argc > 1) ? argv[1] : "./konjour.toml";

	auto table = new BuildTable();
	table->parseConfiguration(configPath);
	table->printContents();
	delete table;
	return 0;
}