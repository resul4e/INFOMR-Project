#include "Context.h"

#include "ModelLoader.h"
#include "TsneAnalysis.h"

Context::Context()
{
	m_database = std::make_shared<Database>();

	connect(m_database.get(), &Database::featuresLoaded, this, &Context::onDatabaseLoaded);
}

/**
 * @brief Set the context model according to the given model descriptor.
 *		  Loads the model associated with the descriptor if not loaded before.
 * @param _modelDescriptor The descriptor describing model identity and features
*/
void Context::SetModel(ModelDescriptor _modelDescriptor)
{
	m_modelDescriptor = _modelDescriptor;

	if (m_modelDescriptor.m_model == nullptr)
		m_modelDescriptor.m_model = ModelLoader::LoadModel(std::filesystem::path(_modelDescriptor.m_path));
	
	m_modelDescriptor.UpdateFeatures();

	emit modelChanged();
}

ModelDescriptor& Context::GetActiveModel()
{
	return m_modelDescriptor;
}

void Context::SetEmbedding(std::vector<glm::vec2>& _embedding)
{
	m_embedding = _embedding;

	emit embeddingChanged();
}

const std::vector<glm::vec2>& Context::GetEmbedding()
{
	return m_embedding;
}

std::shared_ptr<Database> Context::GetDatabase()
{
	return m_database;
}

void Context::onDatabaseLoaded()
{
	ComputeEmbedding();
}

void Context::ComputeEmbedding()
{
	TsneAnalysis tsne(*this);

	std::vector<float> features;

	auto& modelDatabase = GetDatabase()->GetModelDatabase();
	int numDimensions = 1;
	for (ModelDescriptor& md : modelDatabase)
	{
		FeatureVector& fv = GetDatabase()->ComputeFeatureVector(md);
		std::vector<float> floatVector = fv.AsFloatVector();
		numDimensions = floatVector.size();
		features.insert(features.end(), floatVector.begin(), floatVector.end());
	}
	tsne.initTSNE(features, numDimensions);

	tsne.run();
}
