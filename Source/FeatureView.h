#pragma once

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

	void SetModel(std::shared_ptr<Model> _model);
	
private:
	const int FACE_AREA_HISTOGRAM_PRECISION = 10000;
	
	void UpdateFaceAreaHistogram(std::shared_ptr<Model> _model);
	
	bool updated;
	std::shared_ptr<Model> m_model;

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