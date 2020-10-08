#include "FeatureExtraction.h"

constexpr int HISTOGRAM_ITERATIONS = 100000;
constexpr double M_PI = 3.14159265358979323846;  /* pi */

namespace
{
	float ExtractTriangleArea(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
	{
		glm::vec3 e1 = v1 - v0;
		glm::vec3 e2 = v2 - v0;

		float triangleArea = 0.5f * glm::length(glm::cross(e1, e2));
		return triangleArea;
	}

	float ExtractFaceArea(const Mesh& mesh, const Face& face)
	{
		const glm::vec3& v0 = mesh.positions[face.indices[0]];
		const glm::vec3& v1 = mesh.positions[face.indices[1]];
		const glm::vec3& v2 = mesh.positions[face.indices[2]];

		return ExtractTriangleArea(v0, v1, v2);
	}


}

float ExtractSurfaceArea(Model& model)
{
	float totalSurfaceArea = 0;

	for (Mesh& mesh : model.m_meshes)
		for (Face& face : mesh.faces)
			totalSurfaceArea += ExtractFaceArea(mesh, face);

	return totalSurfaceArea;
}

std::vector<double> ExtractFaceAreas(Model& model)
{
	std::vector<double> facesAreas;
	for (Mesh& mesh : model.m_meshes)
	{
		for (Face& face : mesh.faces)
		{
			double triangleArea = ExtractFaceArea(mesh, face);
			facesAreas.push_back(triangleArea);
		}
	}
	return facesAreas;
}

float ExtractAABBArea(Model& _model)
{
	glm::vec3 AABB = _model.m_bounds.max - _model.m_bounds.min;

	return 2 * (AABB.x * AABB.y + AABB.x * AABB.z + AABB.y * AABB.z);
}


float ExtractAABBVolume(Model& _model)
{
	glm::vec3 AABB = _model.m_bounds.max - _model.m_bounds.min;

	return AABB.x * AABB.y * AABB.z;
}

float SignedVolumeOfTriangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
	float v321 = p3.x * p2.y * p1.z;
	float v231 = p2.x * p3.y * p1.z;
	float v312 = p3.x * p1.y * p2.z;
	float v132 = p1.x * p3.y * p2.z;
	float v213 = p2.x * p1.y * p3.z;
	float v123 = p1.x * p2.y * p3.z;
	return (1.0f / 6.0f) * (-v321 + v231 + v312 - v132 - v213 + v123);
}

float ExtractVolumeOfTetrahedron(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4)
{
	return glm::length(glm::dot(p1 - p4, glm::cross(p2 - p4, p3 - p4))) / 6.0;
}

float ExtractVolumeOfMesh(const Mesh& _mesh) {
	float vols = 0;
	for (int i = 0; i < _mesh.faces.size(); i++)
	{
		const unsigned* indices = _mesh.faces[i].indices;
		vols += SignedVolumeOfTriangle(_mesh.positions[indices[0]], _mesh.positions[indices[1]], _mesh.positions[indices[2]]);
	}

	return std::abs(vols);
}

float ExtractVolume(const Model& _model)
{
	float vol = 0;
	for (const Mesh& mesh : _model.m_meshes)
	{
		vol += ExtractVolumeOfMesh(mesh);
	}
	return vol;
}

void GetRandomVertices(const Model& _model, int _count, int* _meshPositions, glm::vec3* o_randomVertices)
{
	for (int j = 0; j < _count; j++)
	{
		int randomIndex = rand() % _model.m_vertexCount;
		int subIndex = randomIndex;
		for (int k = 0; k < _model.m_meshes.size(); k++)
		{
			if (subIndex - (_meshPositions[k]) < _model.m_meshes[k].positions.size())
			{
				o_randomVertices[j] = _model.m_meshes[k].positions[subIndex];
				break;
			}
			subIndex -= _meshPositions[k];
		}
	}
}

void GetMeshPositions(const Model& _model, int* meshPositions)
{
	size_t vertexCount = 0;
	meshPositions[0] = 0;
	for (size_t i = 0; i < _model.m_meshes.size(); i++)
	{
		if (i != _model.m_meshes.size() - 1)
		{
			meshPositions[i + 1] = vertexCount + _model.m_meshes[i].positions.size();
		}
		vertexCount += _model.m_meshes[i].positions.size();
	}
}

HistogramFeature ExtractA1(const Model& _model)
{
	//The size of each of the bins.
	const float binSize = M_PI / HISTOGRAM_BIN_SIZE;

	//Get the start positions for each mesh so that we can pick a random vertex out of all meshes.
	int* meshPositions = new int[_model.m_meshes.size()];;
	GetMeshPositions(_model, meshPositions);

	//Create all bins for the histogram.
	HistogramFeature a1Feature{};
	a1Feature.min = 0;
	a1Feature.max = M_PI;
	memset(a1Feature.binCount, 0, sizeof a1Feature.binCount);
	
	//Iterate for a while, each time picking three random vertices and extracting the angle between them.
	glm::vec3* randomVertices = new glm::vec3[3];
	for (int i = 0; i < HISTOGRAM_ITERATIONS; i++)
	{
		//Get three random vertices.
		GetRandomVertices(_model, 3, meshPositions, randomVertices);

		//calculate the difference of two of the vertices with the third vertex.
		glm::vec3 u = randomVertices[0] - randomVertices[1];
		glm::vec3 v = randomVertices[2] - randomVertices[1];

		//Calculate the angle.
		const float length = (glm::length(u) * glm::length(v));
		const float dt = glm::dot(u, v);
		const float angle = std::acos(dt / length);

		if (std::isnan(angle))
		{
			i--;
			continue;
		}

		//Find the bin it fits in by first dividing by the bin size and then flooring the result by casting it to an int.
		const int bin = static_cast<int>(angle / binSize);
		a1Feature.binCount[bin]++;
	}
	
	return a1Feature;
}

HistogramFeature ExtractD1(const Model& _model)
{
	//assume barycenter is at 0
	//TODO(Resul): Do we want to set this to the actual barycenter?
	const glm::vec3 barycenter{ 0 };

	//Get the start positions for each mesh so that we can pick a random vertex out of all meshes.
	int* meshPositions = new int[_model.m_meshes.size()];
	GetMeshPositions(_model, meshPositions);

	//Get the maximum distance two points can be from the barycenter.
	float maxDistance = 0;
	for (size_t i = 0; i < _model.m_meshes.size(); i++)
	{
		for (int j = 0; j < _model.m_meshes[i].positions.size(); j++)
		{
			maxDistance = std::max(glm::length(_model.m_meshes[i].positions[j] - barycenter), maxDistance);
		}
	}

	//Add small epsilon so that we include all vertices.
	maxDistance += 0.001f;

	//Create all bins for the histogram.
	HistogramFeature d1Feature{};
	d1Feature.max = maxDistance;
	d1Feature.min = 0;
	memset(d1Feature.binCount, 0, sizeof d1Feature.binCount);
	
	//Get the bin size
	const float binSize = maxDistance / HISTOGRAM_BIN_SIZE;

	//Iterate for a while, each time picking a random vertex and extracting the distance between it and the barycenter.
	glm::vec3 randomVertex;
	for (int i = 0; i < HISTOGRAM_ITERATIONS; i++)
	{
		GetRandomVertices(_model, 1, meshPositions, &randomVertex);

		for (int j = 0; j < _model.m_meshes.size(); j++)
		{
			glm::vec3 diff = randomVertex - barycenter;
			const float distance = glm::length(diff);
			const int bin = static_cast<int>(distance / binSize);
			d1Feature.binCount[bin]++;
		}

	}	
	return d1Feature;
}

HistogramFeature ExtractD2(const Model& _model)
{
	//Get the start positions for each mesh so that we can pick a random vertex out of all meshes.
	int* meshPositions = new int[_model.m_meshes.size()];
	GetMeshPositions(_model, meshPositions);

	//Add small epsilon so that we include all vertices.
	const float maxDistance = glm::length(_model.m_bounds.max - _model.m_bounds.min) + 0.001;
	//Calculate the bin size.
	const float binSize = maxDistance / HISTOGRAM_BIN_SIZE;

	//Create all bins for the histogram.
	HistogramFeature d2Feature{};
	d2Feature.min = 0;
	d2Feature.max = maxDistance;
	memset(d2Feature.binCount, 0, sizeof d2Feature.binCount);
	
	//Iterate for a while, each time picking two random vertices and extracting the distance between them.
	glm::vec3 randomVertices[2];
	for (int i = 0; i < HISTOGRAM_ITERATIONS; i++)
	{
		GetRandomVertices(_model, 2, meshPositions, randomVertices);

		//Calculate distance between the two vertices
		glm::vec3 diff = randomVertices[0] - randomVertices[1];
		const float distance = glm::length(diff);

		//Add one to the correct bin
		int bin = static_cast<int>(distance / binSize);
		assert(bin < HISTOGRAM_BIN_SIZE);
		d2Feature.binCount[bin]++;
	}
	
	return d2Feature;
}

HistogramFeature ExtractD3(const Model& _model)
{
	//Get the start positions for each mesh so that we can pick a random vertex out of all meshes.
	int* meshPositions = new int[_model.m_meshes.size()];
	GetMeshPositions(_model, meshPositions);

	//Add small epsilon so that we include all vertices.
	float minArea = std::numeric_limits<float>::max();
	float maxArea = std::numeric_limits<float>::lowest();

	//Get all random triangle areas and the max and min areas
	float* triangleAreas = new float[HISTOGRAM_ITERATIONS];
	glm::vec3 randomVertices[3];
	for (int i = 0; i < HISTOGRAM_ITERATIONS; i++)
	{
		GetRandomVertices(_model, 3, meshPositions, randomVertices);


		float area = std::sqrt(ExtractTriangleArea(randomVertices[0], randomVertices[1], randomVertices[2]));
		triangleAreas[i] = area;

		maxArea = std::max(maxArea, area);
		minArea = std::min(minArea, area);
	}

	//Calculate the bin sizes
	const float distance = maxArea - minArea + 0.001;
	const float binSize = (distance) / HISTOGRAM_BIN_SIZE;

	//Create all bins for the histogram.
	HistogramFeature d3Feature{};
	d3Feature.min = minArea;
	d3Feature.max = maxArea;
	memset(d3Feature.binCount, 0, sizeof d3Feature.binCount);

	//Add to the correct bins
	for (int i = 0; i < HISTOGRAM_ITERATIONS; i++)
	{
		const int bin = static_cast<int>(triangleAreas[i] / binSize);
		d3Feature.binCount[bin]++;
	}

	delete[] triangleAreas;
	return d3Feature;
}

HistogramFeature ExtractD4(const Model& _model)
{
	//Get the start positions for each mesh so that we can pick a random vertex out of all meshes.
	int* meshPositions = new int[_model.m_meshes.size()];
	GetMeshPositions(_model, meshPositions);

	//Add small epsilon so that we include all vertices.
	float maxVolume = std::numeric_limits<float>::lowest();
	float minVolume = std::numeric_limits<float>::max();

	//Get all random tet volumes so that we know what the min and max are.
	float* tetVolumes = new float[HISTOGRAM_ITERATIONS];
	glm::vec3 randomVertices[4];
	for (int i = 0; i < HISTOGRAM_ITERATIONS; i++)
	{
		GetRandomVertices(_model, 4, meshPositions, randomVertices);

		float volume = std::cbrt(ExtractVolumeOfTetrahedron(randomVertices[0], randomVertices[1], randomVertices[2], randomVertices[3]));
		tetVolumes[i] = volume;

		maxVolume = std::max(maxVolume, volume);
		minVolume = std::min(minVolume, volume);
	}

	//Calculate the bin size.
	const float distance = maxVolume - minVolume + 0.001;
	const float binSize = (distance) / HISTOGRAM_BIN_SIZE;

	//Create all bins for the histogram.
	HistogramFeature d4Feature{};
	d4Feature.min = minVolume;
	d4Feature.max = maxVolume;
	memset(d4Feature.binCount, 0, sizeof d4Feature.binCount);

	//Go over all volumes and add them to their correct bins.
	for (int i = 0; i < HISTOGRAM_ITERATIONS; i++)
	{
		const int bin = static_cast<int>(tetVolumes[i] / binSize);
		d4Feature.binCount[bin]++;
	}

	delete[] tetVolumes;
	return d4Feature;
}
