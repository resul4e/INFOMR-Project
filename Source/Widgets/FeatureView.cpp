#include "FeatureView.h"

#include "Model.h"
#include "FeatureExtraction.h"
#include "Widgets/ScatterplotView.h"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QBitmap>
#include <QLineEdit>
#include <QChartView>
#include <QBarSet>
#include <QBarSeries>
#include <qbarcategoryaxis.h>
#include <qvalueaxis.h>

#include <sstream>
#include <string>
#include <fstream>

using namespace QtCharts;

QLineEdit* createField(std::string text, int maxWidth)
{
	QLineEdit* field = new QLineEdit();
	field->setText(text.c_str());
	field->setMaximumWidth(maxWidth);
	field->setReadOnly(true);
	return field;
}

FeatureView::FeatureView(Context& _context) :
	m_context(_context)
{
	connect(&m_context, &Context::modelChanged, this, &FeatureView::OnModelChanged);

	setWindowTitle("Feature View");
	setMinimumWidth(512);

	QWidget* mainWidget = new QWidget(this);
	QLayout* mainlayout = new QGridLayout(mainWidget);
	mainlayout->setMargin(0);
	setWidget(mainWidget);

	QGridLayout* attributeLayout = new QGridLayout();
	QGridLayout* chartLayout = new QGridLayout();
	QGridLayout* featureLayout = new QGridLayout();

	QLabel* modelNameLabel = new QLabel("Name");
	QLabel* verticesLabel = new QLabel("# Vertices");
	QLabel* facesLabel = new QLabel("# Faces");
	QLabel* volumeLabel = new QLabel("Volume");
	QLabel* AABBAreaLabel = new QLabel("AABB Area");
	QLabel* AABBVolumeLabel = new QLabel("AABB Volume");
	QLabel* surfaceAreaLabel = new QLabel("Surface Area");
	QLabel* vsaRatioLabel = new QLabel("Compactness");
	QLabel* eccentricityRatioLabel = new QLabel("Eccentricity");
	QLabel* bbaRatioLabel = new QLabel("Bounding-Box Aspect Ratio");

	m_verticesField = createField("0");
	m_facesField = createField("0");
	m_modelNameField = createField("NULL", 150);
	m_shapeVolumeField = createField("0");
	m_surfaceAreaField = createField("0");
	m_AABBAreaField = createField("0");
	m_AABBVolumeField = createField("0");
	m_vsaRatioField = createField("0");
	m_eccentricityRatioField = createField("0");
	QLineEdit* bbaRatioField = createField("0");

	attributeLayout->addWidget(modelNameLabel, 0, 0);
	attributeLayout->addWidget(m_modelNameField, 0, 1);
	attributeLayout->addWidget(verticesLabel, 1, 0);
	attributeLayout->addWidget(m_verticesField, 1, 1);
	attributeLayout->addWidget(facesLabel, 2, 0);
	attributeLayout->addWidget(m_facesField, 2, 1);

	ScatterplotView* scatterplot = new ScatterplotView(m_context);
	QLabel* legend = new QLabel();
	QPixmap pixmap("../Resources/Legend.png");
	legend->setPixmap(pixmap);
	legend->setMask(pixmap.mask());
	legend->show();

	chartLayout->addWidget(scatterplot, 0, 0);
	chartLayout->addWidget(legend, 1, 0);

	featureLayout->addWidget(volumeLabel, 0, 0);
	featureLayout->addWidget(m_shapeVolumeField, 0, 1);
	featureLayout->addWidget(surfaceAreaLabel, 1, 0);
	featureLayout->addWidget(m_surfaceAreaField, 1, 1);
	featureLayout->addWidget(vsaRatioLabel, 2, 0);
	featureLayout->addWidget(m_vsaRatioField, 2, 1);
	featureLayout->addWidget(bbaRatioLabel, 3, 0);
	featureLayout->addWidget(bbaRatioField, 3, 1);
	featureLayout->addWidget(AABBAreaLabel, 4, 0);
	featureLayout->addWidget(m_AABBAreaField, 4, 1);
	featureLayout->addWidget(AABBVolumeLabel, 5, 0);
	featureLayout->addWidget(m_AABBVolumeField, 5, 1);
	featureLayout->addWidget(eccentricityRatioLabel, 6, 0);
	featureLayout->addWidget(m_eccentricityRatioField, 6, 1);

	QGroupBox* attributeBox = new QGroupBox("Model attributes");
	attributeBox->setLayout(attributeLayout);

	QGroupBox* chartsBox = new QGroupBox("Model charts");
	chartsBox->setLayout(chartLayout);

	QGroupBox* featureBox = new QGroupBox("Model features");
	featureBox->setLayout(featureLayout);

	mainlayout->addWidget(attributeBox);
	mainlayout->addWidget(chartsBox);
	mainlayout->addWidget(featureBox);
}


void FeatureView::OnModelChanged()
{
	const ModelDescriptor& modelDescriptor = m_context.GetActiveModel();

	m_modelNameField->setText(modelDescriptor.m_name.c_str());
	m_verticesField->setText(QString::number(modelDescriptor.m_vertexCount));
	m_facesField->setText(QString::number(modelDescriptor.m_faceCount));

	m_shapeVolumeField->setText(QString::number(modelDescriptor.m_3DFeatures[VOLUME_3D]));
	m_surfaceAreaField->setText(QString::number(modelDescriptor.m_3DFeatures[SURFACE_AREA_3D]));
	m_vsaRatioField->setText(QString::number(modelDescriptor.m_3DFeatures[COMPACTNESS_3D]));
	m_AABBAreaField->setText(QString::number(modelDescriptor.m_3DFeatures[BOUNDS_AREA_3D]));
	m_AABBVolumeField->setText(QString::number(modelDescriptor.m_3DFeatures[BOUNDS_VOLUME_3D]));
	m_eccentricityRatioField->setText(QString::number(modelDescriptor.m_3DFeatures[ECCENTRICITY_3D]));
}
