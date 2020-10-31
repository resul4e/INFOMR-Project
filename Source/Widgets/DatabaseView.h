#pragma once

#include "Context.h"

#include <QDockWidget>
#include <QLineEdit>
#include <QTreeView>
#include <QListWidget>

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
class QPushButton;

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

	void FindClosestShapes();
	void Update();
	void SimilarModelSelectionChanged();

private:
	QtCharts::QChart* CreateVertexCountChart();
	
	Context& m_context;

	QLineEdit* m_databaseCountField;
	DatabaseHierarchy* m_databaseHierarchy;
	QPushButton* m_computeSimilar;
	QListWidget* m_matchList;

	QtCharts::QChart* m_vertexCountHistogram;
	QSlider* m_vertexCountSlider;
	
	int m_maxVertexCount;
};
