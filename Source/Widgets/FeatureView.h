#pragma once

#include "Context.h"

#include <QDockWidget>

class Model;
class QLineEdit;
namespace QtCharts
{
	class QChart;
}

class FeatureView : public QDockWidget
{
	Q_OBJECT
public:
	FeatureView(Context& _context);
	QtCharts::QChart* CreateFaceAreaHistogram();

public slots:
	void OnModelChanged();
	
private:
	const int FACE_AREA_HISTOGRAM_PRECISION = 10000;
	
	void UpdateFaceAreaHistogram(const ModelDescriptor& _modelDescriptor);
	
	Context& m_context;

	bool updated;

	QLineEdit* m_modelNameField;
	QLineEdit* m_verticesField;
	QLineEdit* m_facesField;
	QtCharts::QChart* m_faceAreaHistogram;

	QLineEdit* m_surfaceAreaField;
	QLineEdit* m_AABBAreaField;
	QLineEdit* m_AABBVolumeField;
	QLineEdit* m_shapeVolumeField;
	QLineEdit* m_vsaRatioField;
	QLineEdit* m_eccentricityRatioField;
};

QLineEdit* createField(std::string text, int maxWidth = 75);
