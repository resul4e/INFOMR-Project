#pragma once

#include "ModelDescriptor.h"
#include "Database.h"

#include <QObject>

#include <memory>

typedef std::shared_ptr<ModelDescriptor> ModelDescPtr;

class Context : public QObject
{
	Q_OBJECT
public:
	Context();

	void SetModel(ModelDescriptor _modelDescriptor);
	ModelDescriptor& GetActiveModel();

	/**
	 * @brief Returns a pointer to the database.
	 * @return The database containing all loaded models.
	*/
	std::shared_ptr<Database> GetDatabase();

signals:
	void modelChanged();

private:
	ModelDescriptor m_modelDescriptor;

	std::shared_ptr<Database> m_database;
};
