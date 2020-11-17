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
class QLineEdit;

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

	QtCharts::QChart* CreateFaceAreaHistogram();
	void FindClosestShapes();
	void FindClosestShapesRadius();
	void Update();
	void SimilarModelSelectionChanged();

public slots:
	void OnModelChanged();

private:
	const int FACE_AREA_HISTOGRAM_PRECISION = 10000;

	QtCharts::QChart* CreateVertexCountChart();

	void UpdateFaceAreaHistogram(const ModelDescriptor& _modelDescriptor);
	
	Context& m_context;

	QLineEdit* m_databaseCountField;
	DatabaseHierarchy* m_databaseHierarchy;
	QPushButton* m_computeSimilarK;
	QPushButton* m_computeSimilarR;
	QLineEdit* m_querySizeInput;
	QLineEdit* m_queryRadiusInput;
	QListWidget* m_matchList;

	QtCharts::QChart* m_vertexCountHistogram;
	QSlider* m_vertexCountSlider;
	QtCharts::QChart* m_faceAreaHistogram;
	
	int m_maxVertexCount;
};
