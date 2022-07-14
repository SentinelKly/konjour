#include "konjour.hh"

BuildTable::BuildTable()
{

}

int main()
{
	const auto art = new Artefact("test");
	std::cout << art->m_Name << std::endl;
	return 0;
}