#include "Database.h"

#include <string>
#include <iostream>
#include <fstream>

#include "ModelLoader.h"
#include "Model.h"
#include "ModelSaver.h"
#include "ModelAnalytics.h"
#include "ModelProcessing.h"

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

}

void Database::AddModel(ModelDescriptor _model)
{
	m_modelDatabase.push_back(_model);
}

std::vector<ModelDescriptor>& Database::GetModelDatabase()
{
	return m_modelDatabase;
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
		if (fs::exists(featureDatabasePath / modelDescriptor.m_path.filename().replace_extension(".csv")))
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

	CompoundHistogramPerClass();
	ComputeFeatureVectors();
	BuildANNIndex();
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
	
	featureVector.AddFeature((md.m_3DFeatures.volume - m_averageFeatures.volume) / m_stddevFeatures.volume);
	featureVector.AddFeature((md.m_3DFeatures.surfaceArea - m_averageFeatures.surfaceArea) / m_stddevFeatures.surfaceArea);
	featureVector.AddFeature((md.m_3DFeatures.compactness - m_averageFeatures.compactness) / m_stddevFeatures.compactness);
	featureVector.AddFeature((md.m_3DFeatures.boundsArea - m_averageFeatures.boundsArea) / m_stddevFeatures.boundsArea);
	featureVector.AddFeature((md.m_3DFeatures.boundsVolume - m_averageFeatures.boundsVolume) / m_stddevFeatures.boundsVolume);
	featureVector.AddFeature((md.m_3DFeatures.eccentricity - m_averageFeatures.eccentricity) / m_stddevFeatures.eccentricity);
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
	int numDims = 46;
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

		distances[i] = FeatureVectorDistance(fv1, fv2);
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
	m_index.knnSearch(query, indices, dists, k, flann::SearchParams(128));

	std::vector<int> closestKIndices;
	for (int i = 0; i < k; i++)
		closestKIndices.push_back(indices[0][i]);

	return closestKIndices;
}

void Database::ComputeFeatureStandardization()
{
	float averageVolume = 0;
	float averageSurfaceArea = 0;
	float averageCompactness = 0;
	float averageBoundsArea = 0;
	float averageBoundsVolume = 0;
	float averageEccentricity = 0;

	for (ModelDescriptor& modelDescriptor : m_modelDatabase)
	{
		averageVolume += modelDescriptor.m_3DFeatures.volume;
		averageSurfaceArea += modelDescriptor.m_3DFeatures.surfaceArea;
		averageCompactness += modelDescriptor.m_3DFeatures.compactness;
		averageBoundsArea += modelDescriptor.m_3DFeatures.boundsArea;
		averageBoundsVolume += modelDescriptor.m_3DFeatures.boundsVolume;
		averageEccentricity += modelDescriptor.m_3DFeatures.eccentricity;
	}

	averageVolume /= m_modelDatabase.size();
	averageSurfaceArea /= m_modelDatabase.size();
	averageCompactness /= m_modelDatabase.size();
	averageBoundsArea /= m_modelDatabase.size();
	averageBoundsVolume /= m_modelDatabase.size();
	averageEccentricity /= m_modelDatabase.size();

	float stddevVolume = 0;
	float stddevSurfaceArea = 0;
	float stddevCompactness = 0;
	float stddevBoundsArea = 0;
	float stddevBoundsVolume = 0;
	float stddevEccentricity = 0;

	for (ModelDescriptor& modelDescriptor : m_modelDatabase)
	{
		stddevVolume += pow(modelDescriptor.m_3DFeatures.volume - averageVolume, 2);
		stddevSurfaceArea += pow(modelDescriptor.m_3DFeatures.surfaceArea - averageSurfaceArea, 2);
		stddevCompactness += pow(modelDescriptor.m_3DFeatures.compactness - averageCompactness, 2);
		stddevBoundsArea += pow(modelDescriptor.m_3DFeatures.boundsArea - averageBoundsArea, 2);
		stddevBoundsVolume += pow(modelDescriptor.m_3DFeatures.boundsVolume - averageBoundsVolume, 2);
		stddevEccentricity += pow(modelDescriptor.m_3DFeatures.eccentricity - averageEccentricity, 2);
	}

	stddevVolume = sqrt(stddevVolume / (m_modelDatabase.size() - 1));
	stddevSurfaceArea = sqrt(stddevSurfaceArea / (m_modelDatabase.size() - 1));
	stddevCompactness = sqrt(stddevCompactness / (m_modelDatabase.size() - 1));
	stddevBoundsArea = sqrt(stddevBoundsArea / (m_modelDatabase.size() - 1));
	stddevBoundsVolume = sqrt(stddevBoundsVolume / (m_modelDatabase.size() - 1));
	stddevEccentricity = sqrt(stddevEccentricity / (m_modelDatabase.size() - 1));

	m_averageFeatures.volume = averageVolume;
	m_averageFeatures.surfaceArea = averageSurfaceArea;
	m_averageFeatures.compactness = averageCompactness;
	m_averageFeatures.boundsArea = averageBoundsArea;
	m_averageFeatures.boundsVolume = averageBoundsVolume;
	m_averageFeatures.eccentricity = averageEccentricity;

	m_stddevFeatures.volume = stddevVolume;
	m_stddevFeatures.surfaceArea = stddevSurfaceArea;
	m_stddevFeatures.compactness = stddevCompactness;
	m_stddevFeatures.boundsArea = stddevBoundsArea;
	m_stddevFeatures.boundsVolume = stddevBoundsVolume;
	m_stddevFeatures.eccentricity = stddevEccentricity;
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

	ComputeFeatureStandardization();
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
