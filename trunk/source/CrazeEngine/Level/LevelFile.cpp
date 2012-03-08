#include "CrazeEngine.h"
#include "LevelFile.h"

using namespace Craze;

void LevelFile::CreateNew(std::string name)
{
	std::vector<std::string> queries;

	queries.push_back("CREATE TABLE ProjSettings (id INTEGER PRIMARY KEY, Name TEXT);");

}