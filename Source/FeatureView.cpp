#include "FeatureView.h"

#include "Model.h"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>

#include <string>

QLineEdit* createField(std::string text)
{
	QLineEdit* field = new QLineEdit();
	field->setText(text.c_str());
	field->setMaximumWidth(50);
	field->setReadOnly(true);
	return field;
}

FeatureView::FeatureView() :
	m_model(nullptr)
{
	setWindowTitle("Feature View");
	setMinimumWidth(250);

	QWidget* mainWidget = new QWidget(this);
	QLayout* mainlayout = new QGridLayout(mainWidget);
	mainlayout->setMargin(0);
	setWidget(mainWidget);

	QGridLayout* viewLayout = new QGridLayout();
	QLabel* modelNameLabel = new QLabel("Name");
	QLabel* verticesLabel = new QLabel("# Vertices");
	QLabel* facesLabel = new QLabel("# Faces");
	QLabel* volumeLabel = new QLabel("Volume");
	QLabel* surfaceAreaLabel = new QLabel("Surface Area");
	QLabel* vsaRatioLabel = new QLabel("Volume-to-Surface Area Ratio");
	QLabel* bbaRatioLabel = new QLabel("Bounding-Box Aspect Ratio");

	m_verticesField = createField("0");
	m_facesField = createField("0");
	m_modelNameField = createField("NULL");
	QLineEdit* volumeField = createField("0");
	QLineEdit* surfaceAreaField = createField("0");
	QLineEdit* vsaRatioField = createField("0");
	QLineEdit* bbaRatioField = createField("0");

	QSpacerItem* spacer = new QSpacerItem(0, 1000);

	viewLayout->addWidget(volumeLabel, 0, 0);
	viewLayout->addWidget(volumeField, 0, 1);
	viewLayout->addWidget(surfaceAreaLabel, 1, 0);
	viewLayout->addWidget(surfaceAreaField, 1, 1);
	viewLayout->addWidget(vsaRatioLabel, 2, 0);
	viewLayout->addWidget(vsaRatioField, 2, 1);
	viewLayout->addWidget(bbaRatioLabel, 3, 0);
	viewLayout->addWidget(bbaRatioField, 3, 1);
	viewLayout->addWidget(verticesLabel, 4, 0);
	viewLayout->addWidget(m_verticesField, 4, 1);
	viewLayout->addWidget(facesLabel, 5, 0);
	viewLayout->addWidget(m_facesField, 5, 1);
	viewLayout->addWidget(modelNameLabel, 6, 0);
	viewLayout->addWidget(m_modelNameField, 6, 1);
	//viewLayout->setColumnStretch(0, 1);
	viewLayout->addItem(spacer, 7, 0);

	QGroupBox* settingsBox = new QGroupBox("Basic features");
	settingsBox->setLayout(viewLayout);

	mainlayout->addWidget(settingsBox);
}

void FeatureView::SetModel(std::shared_ptr<Model> _model)
{
	m_model = _model;
	m_verticesField->setText(std::to_string(m_model->m_vertexCount).c_str());
	m_facesField->setText(std::to_string(m_model->m_faceCount).c_str());
	m_modelNameField->setText(m_model->m_name.c_str());
}
