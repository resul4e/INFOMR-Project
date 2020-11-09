#include "Database.h"

#include <string>
#include <iostream>
#include <fstream>

#include "ModelLoader.h"
#include "Model.h"
#include "ModelSaver.h"
#include "ModelAnalytics.h"
#include "ModelProcessing.h"

#include "Evaluation/Evaluation.h"

#include <flann/algorithms/kdtree_index.h>

#include <QDebug>

namespace fs = std::filesystem;

namespace
{
	template <typename T>
	std::vector<size_t> sortIndices(const std::vector<T>& v)
	{
		std::vector<size_t> idx(v.size());
		std::iota(idx.begin(), idx.end(), 0);

		std::stable_sort(idx.begin(), idx.end(),
			[&v](size_t i1, size_t i2) {return v[i1] < v[i2]; });

		return idx;
	}
}

Database::Database() :
	m_index(flann::KDTreeIndexParams(4))
{
	connect(this, &Database::featuresLoaded, this, &Database::OnFeaturesLoaded);
}

void Database::AddModel(ModelDescriptor _model)
{
	m_modelDatabase.push_back(_model);
}

std::vector<ModelDescriptor>& Database::GetModelDatabase()
{
	return m_modelDatabase;
}

std::unordered_map<std::string, int>& Database::GetClassCounts()
{
	return m_classCounts;
}

ModelDescriptor Database::FindModelByName(const std::string& _name)
{
	for(auto md : m_modelDatabase)
	{
		if(md.m_name == _name)
		{
			return md;
		}
	}
	std::cerr << "Could not find Model with name " << _name << "!";
	return {};
}

void Database::ProcessAllModels()
{
	const fs::path modifiedMeshesPath = fs::path("..\\ModifiedMeshes");
	const fs::path featureDatabasePath = fs::path("..\\FeatureDatabase");
	const fs::path descriptorDatabasePath = fs::path("..\\DescriptorDatabase");
	const fs::path savedMeshesPath = fs::path("..\\SavedMeshes");
	fs::create_directory(savedMeshesPath);
	fs::create_directory(modifiedMeshesPath);

	Features3D::globalBoundsA3.s = 0;
	Features3D::globalBoundsA3.t = 3.14159f;
	Features3D::globalBoundsD1.s = 0;
	Features3D::globalBoundsD1.t = std::sqrt(2) / 2.0f + 0.001f;
	Features3D::globalBoundsD2.s = 0;
	Features3D::globalBoundsD2.t = std::sqrt(3) + 0.001f;
	Features3D::globalBoundsD3.s = 0;
	Features3D::globalBoundsD3.t = std::sqrt(2*std::sqrt(3) / 4.0f);
	Features3D::globalBoundsD4.s = 0;
	Features3D::globalBoundsD4.t = std::cbrt(1.0f/3.0f);

	for (ModelDescriptor& modelDescriptor : m_modelDatabase)
	{
		if (fs::exists(featureDatabasePath / modelDescriptor.m_path.filename().replace_extension(".csv")) && fs::exists(descriptorDatabasePath / modelDescriptor.m_path.filename().replace_extension(".csv")))
		{
			continue;
		}
		
		if(!fs::exists(savedMeshesPath / modelDescriptor.m_path.filename().replace_extension(".ply")))
		{
			modelDescriptor.m_model = ModelLoader::LoadModel(std::filesystem::path(modelDescriptor.m_path));
			//proc::SubdivideModel(modelDescriptor);
			proc::CrunchModel(modelDescriptor);
			proc::Remesh(modelDescriptor);
			proc::Normalize(modelDescriptor);
		}
		else
		{
			modelDescriptor.m_model = LoadSavedModel(modelDescriptor.m_path);
			if(modelDescriptor.m_model == nullptr)
			{
				continue;
			}
			proc::Normalize(modelDescriptor);
		}

		//modelDescriptor.UpdateBounds();
		//modelDescriptor.UpdateFeatures();
		ModelSaver::SavePly(modelDescriptor, savedMeshesPath / modelDescriptor.m_path.filename().replace_extension(".ply"));
		modelDescriptor.m_model = nullptr;
	}

	LoadFeatureDatabase();
	//CompoundHistogramPerClass();
}

void Database::RemeshAllModels()
{
	for (ModelDescriptor& modelDescriptor : m_modelDatabase)
	{
		proc::Remesh(modelDescriptor);
	}
}

void Database::SaveAllModels()
{
	const fs::path savedMeshesPath = fs::path("..\\SavedMeshes");
	fs::create_directory(savedMeshesPath);
	for (ModelDescriptor& modelDescriptor : m_modelDatabase)
	{
		ModelSaver::SavePly(modelDescriptor, savedMeshesPath / modelDescriptor.m_path.filename().replace_extension(".ply"));
	}
}

void Database::NormalizeAllModels()
{
	analytics::DataRecorder barycenterRecorder;
	analytics::DataRecorder alignmentRecorder;
	analytics::DataRecorder scaleRecorder;

	for (ModelDescriptor& modelDescriptor : m_modelDatabase)
	{
		barycenterRecorder.preRecord(analytics::ComputeBarycenterDistance(*modelDescriptor.m_model));
		alignmentRecorder.preRecord(analytics::ComputeAbsCosineMajorEigenToXAxis(*modelDescriptor.m_model));
		scaleRecorder.preRecord(analytics::ComputeLongestAABBAxis(*modelDescriptor.m_model));

		proc::Normalize(modelDescriptor);

		barycenterRecorder.postRecord(analytics::ComputeBarycenterDistance(*modelDescriptor.m_model));
		alignmentRecorder.postRecord(analytics::ComputeAbsCosineMajorEigenToXAxis(*modelDescriptor.m_model));
		scaleRecorder.postRecord(analytics::ComputeLongestAABBAxis(*modelDescriptor.m_model));
	}

	barycenterRecorder.saveData("barycenters.csv");
	alignmentRecorder.saveData("alignment.csv");
	scaleRecorder.saveData("scale.csv");
}

void Database::SortDatabase(SortingOptions _option)
{
	switch (_option)
	{
	case SortingOptions::VERTEX_COUNT:
		std::sort(m_modelDatabase.begin(), m_modelDatabase.end(), [](const ModelDescriptor& _left, const ModelDescriptor& _right)
	{
				return _left.m_vertexCount < _right.m_vertexCount;
	});
		break;
	case SortingOptions::FACE_COUNT:
		std::sort(m_modelDatabase.begin(), m_modelDatabase.end(), [](const ModelDescriptor& _left, const ModelDescriptor& _right)
			{
				return _left.m_faceCount < _right.m_faceCount;
			});
		break;
	case SortingOptions::BOUNDS:
		std::sort(m_modelDatabase.begin(), m_modelDatabase.end(), [](const ModelDescriptor& _left, const ModelDescriptor& _right)
			{
				return glm::length(_left.m_bounds.max - _left.m_bounds.min) < glm::length(_right.m_bounds.max - _right.m_bounds.min);
			});
		break;
	default:
		break;
	}
}

void Database::OnFeaturesLoaded()
{
	BuildANNIndex();
	ComputeQualityMetrics();
}

void Database::ComputeFeatureVectors()
{
	for (ModelDescriptor& md : GetModelDatabase())
	{
		md.m_featureVector = std::make_shared<FeatureVector>(ComputeFeatureVector(md));
	}
}

FeatureVector Database::ComputeFeatureVector(const ModelDescriptor& md)
{
	FeatureVector featureVector;
	
	Feature singleFeatures(6);
	singleFeatures[0] = (md.m_3DFeatures[VOLUME_3D] - m_singleFeatureAverage[VOLUME_3D]) / m_singleFeatureStddev[VOLUME_3D];
	singleFeatures[1] = (md.m_3DFeatures[SURFACE_AREA_3D] - m_singleFeatureAverage[SURFACE_AREA_3D]) / m_singleFeatureStddev[SURFACE_AREA_3D];
	singleFeatures[2] = (md.m_3DFeatures[COMPACTNESS_3D] - m_singleFeatureAverage[COMPACTNESS_3D]) / m_singleFeatureStddev[COMPACTNESS_3D];
	singleFeatures[3] = (md.m_3DFeatures[BOUNDS_AREA_3D] - m_singleFeatureAverage[BOUNDS_AREA_3D]) / m_singleFeatureStddev[BOUNDS_AREA_3D];
	singleFeatures[4] = (md.m_3DFeatures[BOUNDS_VOLUME_3D] - m_singleFeatureAverage[BOUNDS_VOLUME_3D]) / m_singleFeatureStddev[BOUNDS_VOLUME_3D];
	singleFeatures[5] = (md.m_3DFeatures[ECCENTRICITY_3D] - m_singleFeatureAverage[ECCENTRICITY_3D]) / m_singleFeatureStddev[ECCENTRICITY_3D];
	Feature boundsFeature(3);
	boundsFeature[0] = md.m_3DFeatures.bounds.max.x - md.m_3DFeatures.bounds.min.x;
	boundsFeature[1] = md.m_3DFeatures.bounds.max.y - md.m_3DFeatures.bounds.min.y;
	boundsFeature[2] = md.m_3DFeatures.bounds.max.z - md.m_3DFeatures.bounds.min.z;
	featureVector.AddFeature(singleFeatures);
	//featureVector.AddFeature(boundsFeature);
	featureVector.AddFeature(md.m_3DFeatures.a3);
	featureVector.AddFeature(md.m_3DFeatures.d1);
	featureVector.AddFeature(md.m_3DFeatures.d2);
	featureVector.AddFeature(md.m_3DFeatures.d3);
	featureVector.AddFeature(md.m_3DFeatures.d4);

	return featureVector;
}

void Database::BuildANNIndex()
{
	auto& modelDatabase = GetModelDatabase();

	int numShapes = modelDatabase.size();
	int numDims = modelDatabase[0].m_featureVector->AsFloatVector().size();
	flann::Matrix<float> dataset = flann::Matrix<float>(new float[numShapes * numDims], numShapes, numDims);

	for (int i = 0; i < modelDatabase.size(); i++)
	{
		ModelDescriptor& md = modelDatabase[i];

		std::vector<float> floatVector = md.m_featureVector->AsFloatVector();

		for (int d = 0; d < numDims; d++)
		{
			dataset[i][d] = floatVector[d];
		}
	}

	// Construct an randomized kd-tree index using 4 kd-trees
	m_index = flann::Index<flann::L2<float>>(dataset, flann::KDTreeIndexParams(4));
	m_index.buildIndex();
}

std::vector<int> Database::FindClosestKNNShapes(ModelDescriptor& md, int k)
{
	auto& modelDatabase = GetModelDatabase();
	FeatureVector fv1 = ComputeFeatureVector(md);

	std::vector<float> distances(modelDatabase.size());
	for (int i = 0; i < modelDatabase.size(); i++)
	{
		ModelDescriptor& md = modelDatabase[i];
		FeatureVector fv2 = ComputeFeatureVector(md);

		distances[i] = FeatureVectorDistance(fv1, fv2, m_hist_weights);
	}

	std::vector<size_t> indices = sortIndices(distances);

	std::vector<int> closestKIndices;
	for (int i = 0; i < k; i++)
		closestKIndices.push_back(indices[i + 1]);

	return closestKIndices;
}

std::vector<int> Database::FindClosestANNShapes(ModelDescriptor& md, int k)
{
	FeatureVector fv = ComputeFeatureVector(md);
	std::vector<float> floatVector = fv.AsFloatVector();

	int numDims = floatVector.size();
	flann::Matrix<float> query(floatVector.data(), 1, numDims);

	std::vector<std::vector<int>> indices;
	std::vector<std::vector<float>> dists;
	// Do a knn search, using 128 checks
	m_index.knnSearch(query, indices, dists, k+1, flann::SearchParams(128));

	std::vector<int> closestKIndices;
	for (int i = 1; i <= k; i++)
		closestKIndices.push_back(indices[0][i]);

	return closestKIndices;
}

void Database::ComputeQualityMetrics()
{
	eval::WritePerformance(*this, false);
	eval::WritePerformance(*this, true);
	//eval::WriteNNResults(*this, false);
	//eval::WriteNNResults(*this, true);
}

void Database::ComputeFeatureStandardization(DescriptorName _descriptorName)
{
	float averageValue = 0;
	float stddevValue = 0;

	for (ModelDescriptor& modelDescriptor : m_modelDatabase)
		averageValue += modelDescriptor.m_3DFeatures[_descriptorName];

	averageValue /= m_modelDatabase.size();

	for (ModelDescriptor& modelDescriptor : m_modelDatabase)
		stddevValue += pow(modelDescriptor.m_3DFeatures[_descriptorName] - averageValue, 2);

	stddevValue = sqrt(stddevValue / (m_modelDatabase.size() - 1));

	m_singleFeatureAverage[_descriptorName] = averageValue;
	m_singleFeatureStddev[_descriptorName] = stddevValue;
	qDebug() << "Standardization computed";
}

void Database::ComputeHistogramFeatureWeights()
{
	std::vector<float> distancesA3;
	std::vector<float> distancesD1;
	std::vector<float> distancesD2;
	std::vector<float> distancesD3;
	std::vector<float> distancesD4;

	WassersteinDistance wasserstein;
	for (int i = 0; i < m_modelDatabase.size(); i++)
	{
		for (int j = i+1; j < m_modelDatabase.size(); j++)
		{
			HistogramFeature& a = m_modelDatabase[i].m_3DFeatures.a3;
			HistogramFeature& b = m_modelDatabase[j].m_3DFeatures.a3;
			float distanceA3 = wasserstein.distance(a, b);

			a = m_modelDatabase[i].m_3DFeatures.d1;
			b = m_modelDatabase[j].m_3DFeatures.d1;
			float distanceD1 = wasserstein.distance(a, b);

			a = m_modelDatabase[i].m_3DFeatures.d2;
			b = m_modelDatabase[j].m_3DFeatures.d2;
			float distanceD2 = wasserstein.distance(a, b);

			a = m_modelDatabase[i].m_3DFeatures.d3;
			b = m_modelDatabase[j].m_3DFeatures.d3;
			float distanceD3 = wasserstein.distance(a, b);

			a = m_modelDatabase[i].m_3DFeatures.d4;
			b = m_modelDatabase[j].m_3DFeatures.d4;
			float distanceD4 = wasserstein.distance(a, b);

			distancesA3.push_back(distanceA3);
			distancesD1.push_back(distanceD1);
			distancesD2.push_back(distanceD2);
			distancesD3.push_back(distanceD3);
			distancesD4.push_back(distanceD4);
		}
	}

	float averageA3 = 0, averageD1 = 0, averageD2 = 0, averageD3 = 0, averageD4 = 0;
	float stddevA3 = 0, stddevD1 = 0, stddevD2 = 0, stddevD3 = 0, stddevD4 = 0;

	for (int i = 0; i < distancesA3.size(); i++)
	{
		averageA3 += distancesA3[i];
		averageD1 += distancesD1[i];
		averageD2 += distancesD2[i];
		averageD3 += distancesD3[i];
		averageD4 += distancesD4[i];
	}
	averageA3 /= distancesA3.size();
	averageD1 /= distancesA3.size();
	averageD2 /= distancesA3.size();
	averageD3 /= distancesA3.size();
	averageD4 /= distancesA3.size();

	for (int i = 0; i < distancesA3.size(); i++)
	{
		stddevA3 += pow(distancesA3[i] - averageA3, 2);
		stddevD1 += pow(distancesD1[i] - averageD1, 2);
		stddevD2 += pow(distancesD2[i] - averageD2, 2);
		stddevD3 += pow(distancesD3[i] - averageD3, 2);
		stddevD4 += pow(distancesD4[i] - averageD4, 2);
	}

	stddevA3 = sqrt(stddevA3 / (distancesA3.size() - 1));
	stddevD1 = sqrt(stddevD1 / (distancesD1.size() - 1));
	stddevD2 = sqrt(stddevD2 / (distancesD2.size() - 1));
	stddevD3 = sqrt(stddevD3 / (distancesD3.size() - 1));
	stddevD4 = sqrt(stddevD4 / (distancesD4.size() - 1));

	m_hist_weights.push_back(1 / stddevA3);
	m_hist_weights.push_back(1 / stddevD1);
	m_hist_weights.push_back(1 / stddevD2);
	m_hist_weights.push_back(1 / stddevD3);
	m_hist_weights.push_back(1 / stddevD4);
}

void Database::ComputeClassCounts()
{
	for (ModelDescriptor& md : m_modelDatabase)
	{
		if (m_classCounts.find(md.m_class) != m_classCounts.end())
			m_classCounts[md.m_class]++;
		else
			m_classCounts[md.m_class] = 0;
	}
}

void Database::LoadFeatureDatabase()
{
	const fs::path featureDatabasePath = fs::path("..\\FeatureDatabase");

	for (ModelDescriptor& modelDescriptor : m_modelDatabase)
	{
		fs::path featurePath = featureDatabasePath / modelDescriptor.m_path.filename().replace_extension(".csv");
		if (fs::exists(featurePath))
		{
			modelDescriptor.m_3DFeatures = ModelLoader::LoadFeatures(featurePath);
		}
	}

	// Standardize single features
	ComputeFeatureStandardization(VOLUME_3D);
	ComputeFeatureStandardization(SURFACE_AREA_3D);
	ComputeFeatureStandardization(COMPACTNESS_3D);
	ComputeFeatureStandardization(BOUNDS_3D);
	ComputeFeatureStandardization(BOUNDS_AREA_3D);
	ComputeFeatureStandardization(BOUNDS_VOLUME_3D);
	ComputeFeatureStandardization(ECCENTRICITY_3D);

	// Compute histogram distance weights
	//ComputeHistogramFeatureWeights();
	m_hist_weights = { 2.03818, 1.14862, 2.13344, 1.71947, 2.04633 };

	ComputeFeatureVectors();
	ComputeClassCounts();
	emit featuresLoaded();
}

void Database::CompoundHistogramPerClass()
{
	const fs::path featureDatabasePath = fs::path("..\\FeatureDatabase");
	fs::create_directory(featureDatabasePath / "Combined");
	std::map<std::string, std::vector<ModelDescriptor>> classMap;
	for (ModelDescriptor& modelDescriptor : m_modelDatabase)
	{
		if(!fs::exists(featureDatabasePath / modelDescriptor.m_path.filename().replace_extension(".csv")))
		{
			continue;
		}
		
		modelDescriptor.m_3DFeatures = ModelLoader::LoadFeatures(featureDatabasePath / modelDescriptor.m_path.filename().replace_extension(".csv"));
		
		if(classMap.find(modelDescriptor.m_class) == classMap.end())
		{
			classMap.insert({ modelDescriptor.m_class, std::vector<ModelDescriptor>() });
		}
		classMap[modelDescriptor.m_class].push_back(modelDescriptor);
	}

	for(auto classDescPair : classMap)
	{
		std::ofstream outFile(featureDatabasePath / "Combined" / ("a3" + classDescPair.first + ".csv"));

		for(const ModelDescriptor& desc : classDescPair.second)
		{
			const HistogramFeature& a3 = desc.m_3DFeatures.a3;
			for (int i = 0; i < a3.m_numBins; i++)
			{
				outFile << a3[i] << ", ";
			}
			outFile << "\n";
		}
		outFile << Features3D::globalBoundsA3.s << ", " << Features3D::globalBoundsA3.t;
		
		outFile.close();
	}

	for (auto classDescPair : classMap)
	{
		std::ofstream outFile(featureDatabasePath / "Combined" / ("d1" + classDescPair.first + ".csv"));

		for (const ModelDescriptor& desc : classDescPair.second)
		{
			const HistogramFeature& d1 = desc.m_3DFeatures.d1;
			for (int i = 0; i < d1.m_numBins; i++)
			{
				outFile << d1[i] << ", ";
			}
			outFile << "\n";
		}
		outFile << Features3D::globalBoundsD1.s << ", " << Features3D::globalBoundsD1.t;

		outFile.close();
	}

	for (auto classDescPair : classMap)
	{
		std::ofstream outFile(featureDatabasePath / "Combined" / ("d2" + classDescPair.first + ".csv"));

		for (const ModelDescriptor& desc : classDescPair.second)
		{
			const HistogramFeature& d2 = desc.m_3DFeatures.d2;
			for (int i = 0; i < d2.m_numBins; i++)
			{
				outFile << d2[i] << ", ";
			}
			outFile << "\n";
		}
		outFile << Features3D::globalBoundsD2.s << ", " << Features3D::globalBoundsD2.t;

		outFile.close();
	}

	for (auto classDescPair : classMap)
	{
		std::ofstream outFile(featureDatabasePath / "Combined" / ("d3" + classDescPair.first + ".csv"));

		for (const ModelDescriptor& desc : classDescPair.second)
		{
			const HistogramFeature& d3 = desc.m_3DFeatures.d3;
			for (int i = 0; i < d3.m_numBins; i++)
			{
				outFile << d3[i] << ", ";
			}
			outFile << "\n";
		}
		outFile << Features3D::globalBoundsD3.s << ", " << Features3D::globalBoundsD3.t;

		outFile.close();
	}

	for (auto classDescPair : classMap)
	{
		std::ofstream outFile(featureDatabasePath / "Combined" / ("d4" + classDescPair.first + ".csv"));

		for (const ModelDescriptor& desc : classDescPair.second)
		{
			const HistogramFeature& d4 = desc.m_3DFeatures.d4;
			for (int i = 0; i < d4.m_numBins; i++)
			{
				outFile << d4[i] << ", ";
			}
			outFile << "\n";
		}
		outFile << Features3D::globalBoundsD4.s << ", " << Features3D::globalBoundsD4.t;

		outFile.close();
	}
}

std::shared_ptr<Model> Database::LoadSavedModel(std::filesystem::path _modelFileName)
{
	//TODO(Resul): Remove this return
	//For now just always load the default model

	fs::path modifiedMeshesPath = fs::path("..\\SavedMeshes");

	fs::path offPath = _modelFileName;
	fs::path plyPath = _modelFileName.replace_extension(".ply");
	if (fs::exists(modifiedMeshesPath / plyPath))
	{
		return ModelLoader::LoadModel(modifiedMeshesPath / plyPath);
	}

	if(fs::exists(modifiedMeshesPath / offPath))
	{
		return ModelLoader::LoadModel(modifiedMeshesPath / offPath);
	}

	return nullptr;
}
