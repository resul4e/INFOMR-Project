#include "Context.h"

Context::Context()
{
	m_database = std::make_shared<Database>();
}

void Context::SetModel(ModelDescriptor _modelDescriptor)
{
	m_modelDescriptor = _modelDescriptor;

	emit modelChanged();
}

ModelDescriptor& Context::GetActiveModel()
{
	return m_modelDescriptor;
}

std::shared_ptr<Database> Context::GetDatabase()
{
	return m_database;
}
