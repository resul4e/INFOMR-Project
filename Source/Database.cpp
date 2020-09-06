#include "Database.h"

void Database::AddModel(std::shared_ptr<Model> _model)
{
	m_modelDatabase.push_back(_model);
}
