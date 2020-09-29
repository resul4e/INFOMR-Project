#pragma once

#include "ModelDescriptor.h"

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
	FeatureView();
	QtCharts::QChart* CreateFaceAreaHistogram();

	void SetModel(ModelDescriptor _modelDescriptor);
	
private:
	const int FACE_AREA_HISTOGRAM_PRECISION = 10000;
	
	void UpdateFaceAreaHistogram(ModelDescriptor& _modelDescriptor);
	
	bool updated;
	ModelDescriptor m_modelDescriptor;

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
