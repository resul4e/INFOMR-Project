#pragma once

#include <hdi/utils/glad/glad.h>
#include "ModelDescriptor.h"
#include "Database.h"

#include <QObject>

#include <memory>
#include <vector>

typedef std::shared_ptr<ModelDescriptor> ModelDescPtr;

class Context : public QObject
{
	Q_OBJECT
public:
	Context();

	void SetModel(ModelDescriptor _modelDescriptor);
	ModelDescriptor& GetActiveModel();

	void SetEmbedding(std::vector<glm::vec2>& _embedding);
	const std::vector<glm::vec2>& GetEmbedding();

	/**
	 * @brief Returns a pointer to the database.
	 * @return The database containing all loaded models.
	*/
	std::shared_ptr<Database> GetDatabase();

signals:
	void modelChanged();
	void embeddingChanged();

private slots:
	void onDatabaseLoaded();

private:
	void ComputeEmbedding();

	ModelDescriptor m_modelDescriptor;

	std::shared_ptr<Database> m_database;

	std::vector<glm::vec2> m_embedding;
};
