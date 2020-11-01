#pragma once

class Database;

namespace eval
{
	void ComputeMeanAveragePrecision(Database& database, bool preciseKNN = false);
	void WriteNNResults(Database& database, bool preciseKNN = false);
}
