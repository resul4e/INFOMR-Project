#pragma once

#include <QDockWidget>

class Model;
class QLineEdit;

class FeatureView : public QDockWidget
{
	Q_OBJECT
public:
	FeatureView();

	void SetModel(std::shared_ptr<Model> _model);
	
private:
	std::shared_ptr<Model> m_model;

	QLineEdit* m_verticesField;
	QLineEdit* m_facesField;
	QLineEdit* m_modelNameField;
};
