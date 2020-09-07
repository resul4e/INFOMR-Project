#include "QueryManager.h"

QueryManager::QueryManager()
{
	m_database = std::make_shared<Database>();
}

QueryManager::~QueryManager()
{
}

std::shared_ptr<Database> QueryManager::GetDatabase()
{
	return m_database;
}
