#pragma once

#include "Context.h"

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
	DatabaseHierarchy(Context& _context);

	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void UpdateDataModel();

private:
	Context& m_context;

	DatabaseHierarchyModel* m_model;
};

class DatabaseView : public QDockWidget
{
	Q_OBJECT
public:
	DatabaseView(Context& _context);

	void Update();
	
private:
	QtCharts::QChart* CreateVertexCountChart();
	
	Context& m_context;

	QLineEdit* m_databaseCountField;
	DatabaseHierarchy* m_databaseHierarchy;

	QtCharts::QChart* m_vertexCountHistogram;
	QSlider* m_vertexCountSlider;
	
	int m_maxVertexCount;
};
