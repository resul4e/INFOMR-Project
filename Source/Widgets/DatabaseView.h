#pragma once
#include <QDockWidget>
#include <QLineEdit>
#include <QTreeView>

#include "DatabaseHierarchyModel.h"

class Database;
namespace QtCharts
{
	class QBarSet;
	class QChart;
	class QBarCategoryAxis;
	class QValueAxis;
}
class QSlider;

class DatabaseHierarchy : public QTreeView
{
	Q_OBJECT
public:
	DatabaseHierarchy(Database& database);

	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void UpdateDataModel();

private:
	Database& m_database;

	DatabaseHierarchyModel* m_model;
};

class DatabaseView : public QDockWidget
{
	Q_OBJECT
public:
	DatabaseView(std::shared_ptr<Database> _database);

	void Update();
	
private:
	QtCharts::QChart* CreateVertexCountChart();
	
	QLineEdit* m_databaseCountField;
	DatabaseHierarchy* m_databaseHierarchy;

	QtCharts::QChart* m_vertexCountHistogram;
	QSlider* m_vertexCountSlider;
	
	std::shared_ptr<Database> m_database;
	int m_maxVertexCount;
};
