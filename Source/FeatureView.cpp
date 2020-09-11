#include "FeatureView.h"

#include "Model.h"
#include "FeatureExtraction.h"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>

#include <string>

QLineEdit* createField(std::string text, int maxWidth)
{
	QLineEdit* field = new QLineEdit();
	field->setText(text.c_str());
	field->setMaximumWidth(maxWidth);
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

	QGridLayout* attributeLayout = new QGridLayout();
	QGridLayout* featureLayout = new QGridLayout();

	QLabel* modelNameLabel = new QLabel("Name");
	QLabel* verticesLabel = new QLabel("# Vertices");
	QLabel* facesLabel = new QLabel("# Faces");
	QLabel* volumeLabel = new QLabel("Volume");
	QLabel* surfaceAreaLabel = new QLabel("Surface Area");
	QLabel* vsaRatioLabel = new QLabel("Volume-to-Surface Area Ratio");
	QLabel* bbaRatioLabel = new QLabel("Bounding-Box Aspect Ratio");

	m_verticesField = createField("0");
	m_facesField = createField("0");
	m_modelNameField = createField("NULL", 150);
	QLineEdit* volumeField = createField("0");
	m_surfaceAreaField = createField("0");
	QLineEdit* vsaRatioField = createField("0");
	QLineEdit* bbaRatioField = createField("0");

	QSpacerItem* spacer = new QSpacerItem(0, 1000);

	attributeLayout->addWidget(modelNameLabel, 0, 0);
	attributeLayout->addWidget(m_modelNameField, 0, 1);
	attributeLayout->addWidget(verticesLabel, 1, 0);
	attributeLayout->addWidget(m_verticesField, 1, 1);
	attributeLayout->addWidget(facesLabel, 2, 0);
	attributeLayout->addWidget(m_facesField, 2, 1);

	featureLayout->addWidget(volumeLabel, 0, 0);
	featureLayout->addWidget(volumeField, 0, 1);
	featureLayout->addWidget(surfaceAreaLabel, 1, 0);
	featureLayout->addWidget(m_surfaceAreaField, 1, 1);
	featureLayout->addWidget(vsaRatioLabel, 2, 0);
	featureLayout->addWidget(vsaRatioField, 2, 1);
	featureLayout->addWidget(bbaRatioLabel, 3, 0);
	featureLayout->addWidget(bbaRatioField, 3, 1);
	featureLayout->addItem(spacer, 4, 0);

	QGroupBox* attributeBox = new QGroupBox("Model attributes");
	attributeBox->setLayout(attributeLayout);

	QGroupBox* featureBox = new QGroupBox("Model features");
	featureBox->setLayout(featureLayout);

	mainlayout->addWidget(attributeBox);
	mainlayout->addWidget(featureBox);
}

void FeatureView::SetModel(std::shared_ptr<Model> _model)
{
	m_model = _model;

	m_modelNameField->setText(m_model->m_name.c_str());
	m_verticesField->setText(QString::number(m_model->m_vertexCount));
	m_facesField->setText(QString::number(m_model->m_faceCount));

	m_surfaceAreaField->setText(QString::number(ExtractSurfaceArea(*_model)));
}
