#pragma once

#include "Context.h"

#include <QDockWidget>

class Model;
class QLineEdit;

class FeatureView : public QDockWidget
{
	Q_OBJECT
public:
	FeatureView(Context& _context);

public slots:
	void OnModelChanged();
	
private:
	Context& m_context;

	bool updated;

	QLineEdit* m_modelNameField;
	QLineEdit* m_verticesField;
	QLineEdit* m_facesField;

	QLineEdit* m_surfaceAreaField;
	QLineEdit* m_AABBAreaField;
	QLineEdit* m_AABBVolumeField;
	QLineEdit* m_shapeVolumeField;
	QLineEdit* m_vsaRatioField;
	QLineEdit* m_eccentricityRatioField;
};

QLineEdit* createField(std::string text, int maxWidth = 75);
