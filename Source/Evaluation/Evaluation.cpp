#include "Evaluation.h"

#include "Database.h"

#include <fstream>
#include <random>
#include <chrono>

namespace eval
{
	void ComputeMeanAveragePrecision(Database& database, bool preciseKNN)
	{
		auto& modelDatabase = database.GetModelDatabase();

		std::vector<float> precisions;
		std::vector<float> recalls;

		for (int k = 1; k <= 30; k++)
		{
			std::cout << "K=" << k << std::endl;
			float averagePrecision = 0;
			float averageRecall = 0;

			for (int i = 0; i < modelDatabase.size(); i++)
			{
				ModelDescriptor& md = modelDatabase[i];

				int correct = 0;
				std::vector<int> closestShapes;

				if (preciseKNN)
					closestShapes = database.FindClosestKNNShapes(md, k);
				else
					closestShapes = database.FindClosestANNShapes(md, k);

				for (int index : closestShapes)
				{
					ModelDescriptor& shape = modelDatabase[index];

					if (md.m_class == shape.m_class)
						correct++;
				}

				int TP = correct;
				int FP = k - correct;
				int FN = database.GetClassCounts()[md.m_class] - correct;
				int TN = modelDatabase.size() - FP;
				float precision = (float)TP / (TP + FP);
				float recall = (float)TP / (TP + FN);

				averagePrecision += precision;
				averageRecall += recall;
			}

			averagePrecision /= modelDatabase.size();
			averageRecall /= modelDatabase.size();

			precisions.push_back(averagePrecision);
			recalls.push_back(averageRecall);
			std::cout << "Precision: " << averagePrecision << std::endl;
			std::cout << "Recall: " << averageRecall << std::endl;
		}

		std::string precisionFileName = "";
		std::string recallFileName = "";

		if (preciseKNN)
		{
			precisionFileName = "../Evaluation/precision_knn.csv";
			recallFileName = "../Evaluation/recall_knn.csv";
		}
		else
		{
			precisionFileName = "../Evaluation/precision_ann.csv";
			recallFileName = "../Evaluation/recall_ann.csv";
		}

		std::ofstream precisionFile;
		precisionFile.open(precisionFileName);
		for (int k = 0; k < precisions.size(); k++)
		{
			precisionFile << precisions[k] << '\n';
		}
		precisionFile.close();

		std::ofstream recallFile;
		recallFile.open(recallFileName);
		for (int k = 0; k < recalls.size(); k++)
		{
			recallFile << recalls[k] << '\n';
		}
		recallFile.close();
	}

	void WritePerformance(Database& database, bool preciseKNN)
	{
		auto& modelDatabase = database.GetModelDatabase();

		//int numQueries = 10000;

		//std::random_device rd;
		//std::mt19937 gen(rd());
		//std::uniform_int_distribution<> distrib(0, modelDatabase.size());

		//std::vector<int> modelIndices(numQueries);
		//for (int i = 0; i < numQueries; i++)
		//{
		//	modelIndices[i] = distrib(gen);
		//}

		std::vector<float> timings;
		for (int k = 1; k < 31; k++)
		{
			std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
			for (ModelDescriptor& md : modelDatabase)
			{
				if (preciseKNN)
					database.FindClosestKNNShapes(md, k);
				else
					database.FindClosestANNShapes(md, k);
			}
			std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
			float timing = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / modelDatabase.size();
			timings.push_back(timing);
			std::cout << "Average NN query time = " << timing << "[microseconds] for k=" << k << std::endl;
		}
	}

	void WriteNNResults(Database& database, bool preciseKNN)
	{
		auto& modelDatabase = database.GetModelDatabase();

		std::vector<float> precisions;
		std::vector<float> recalls;

		int k = 1813;

		std::cout << "K=" << k << std::endl;

		std::string confusionFileName = "";

		if (preciseKNN)
			confusionFileName = "../Evaluation/statistics_knn_" + std::to_string(k) + ".csv";
		else
			confusionFileName = "../Evaluation/statistics_ann_" + std::to_string(k) + ".csv";

		std::ofstream confusionFile;
		confusionFile.open(confusionFileName);

		for (int i = 0; i < modelDatabase.size(); i++)
		{
			ModelDescriptor& md = modelDatabase[i];

			int correct = 0;
			std::vector<int> closestShapes;

			if (preciseKNN)
				closestShapes = database.FindClosestKNNShapes(md, k);
			else
				closestShapes = database.FindClosestANNShapes(md, k);

			confusionFile << md.m_class;
			std::vector<std::string> closestClasses(closestShapes.size());
			for (int i = 0; i < closestShapes.size(); i++)
			{
				ModelDescriptor& shape = modelDatabase[closestShapes[i]];

				closestClasses[i] = shape.m_class;
				confusionFile << ',' << shape.m_class;
			}
			confusionFile << '\n';

			//int TP = correct;
			//int FP = k - correct;
			//int FN = database.GetClassCounts()[md.m_class] - correct;
			//int TN = modelDatabase.size() - FP;

			//confusionFile << TP << ' ' << FP << ' ' << FN << ' ' << TN << '\n';
		}

		confusionFile.close();
	}
}
