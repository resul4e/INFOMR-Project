#include "FeatureView.h"

#include "Model.h"
#include "FeatureExtraction.h"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
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

	m_faceAreaHistogram = CreateFaceAreaHistogram();
	m_faceAreaHistogram->setVisible(false);
	QChartView* chartView = new QChartView(m_faceAreaHistogram);
	chartView->setRenderHint(QPainter::Antialiasing);

	QSpacerItem* spacer = new QSpacerItem(0, 1000);

	attributeLayout->addWidget(modelNameLabel, 0, 0);
	attributeLayout->addWidget(m_modelNameField, 0, 1);
	attributeLayout->addWidget(verticesLabel, 1, 0);
	attributeLayout->addWidget(m_verticesField, 1, 1);
	attributeLayout->addWidget(facesLabel, 2, 0);
	attributeLayout->addWidget(m_facesField, 2, 1);

	chartLayout->addWidget(chartView, 0, 0);

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
	featureLayout->addItem(spacer, 7, 0);

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

QChart* FeatureView::CreateFaceAreaHistogram()
{
	//chart
	QBarSet* vertexCountSet = new QBarSet("Database");

	//Create a dummy chart with all zeros
	*vertexCountSet << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0;

	//Create a new series with the dummy values.
	QBarSeries* series = new QBarSeries();
	series->append(vertexCountSet);

	//Create the chart and set the desired properties
	QChart* vertexCountHistogram = new QChart();
	vertexCountHistogram->addSeries(series);
	vertexCountHistogram->legend()->setVisible(false);
	vertexCountHistogram->setTitle("Face area distribution");
	vertexCountHistogram->setAnimationOptions(QChart::SeriesAnimations);

	//Create the x axis categories, these are also dummy values and will be updated.
	QStringList categories;
	categories << "0" << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10";
	
	//Create the x axis and attach it to the chart.
	QBarCategoryAxis* vertexCountHistogramXAxis = new QBarCategoryAxis();
	vertexCountHistogramXAxis->append(categories);
	vertexCountHistogram->addAxis(vertexCountHistogramXAxis, Qt::AlignBottom);
	series->attachAxis(vertexCountHistogramXAxis);

	//Create the 10^-5 used in the x axis title.
	std::ostringstream streamObj;
	streamObj << std::scientific;
	streamObj << std::setprecision(2);
	streamObj << (1.0 / static_cast<double>(FACE_AREA_HISTOGRAM_PRECISION));
	//Update the title for the x axis.
	vertexCountHistogramXAxis->setTitleText(("Face area (*" + streamObj.str() + ")").c_str());
	vertexCountHistogramXAxis->setTitleVisible(true);
	
	//Create the y axis and attach it to the chart.
	QValueAxis* vertexCountHistogramYAxis = new QValueAxis();
	vertexCountHistogramYAxis->setRange(0, 10);
	vertexCountHistogram->addAxis(vertexCountHistogramYAxis, Qt::AlignLeft);
	series->attachAxis(vertexCountHistogramYAxis);

	//y-axis will always have a range from 0-1
	vertexCountHistogramYAxis->setRange(0, 1);
	vertexCountHistogramYAxis->setTitleText("% of faces");
	vertexCountHistogramYAxis->setTitleVisible(true);

	vertexCountHistogram->setMinimumHeight(450);
	vertexCountHistogram->setPreferredHeight(450);
	vertexCountHistogram->setMaximumHeight(450);

	return vertexCountHistogram;
}


void FeatureView::SetModel(std::shared_ptr<Model> _model)
{
	m_model = _model;

	m_modelNameField->setText(m_model->m_name.c_str());
	m_verticesField->setText(QString::number(m_model->m_vertexCount));
	m_facesField->setText(QString::number(m_model->m_faceCount));

	m_surfaceAreaField->setText(QString::number(_model->m_3DFeatures.surfaceArea));
	m_AABBAreaField->setText(QString::number(_model->m_3DFeatures.boundsArea));
	m_AABBVolumeField->setText(QString::number(_model->m_3DFeatures.boundsVolume));
	m_shapeVolumeField->setText(QString::number(_model->m_3DFeatures.volume));
	m_vsaRatioField->setText(QString::number(_model->m_3DFeatures.compactness));
	m_eccentricityRatioField->setText(QString::number(_model->m_3DFeatures.eccentricity));
	
	UpdateFaceAreaHistogram(_model);
}

void FeatureView::UpdateFaceAreaHistogram(std::shared_ptr<Model> _model)
{
	//get the areas of each of the triangles in the model
	std::vector<double> areas = ExtractFaceAreas(*_model);
	std::sort(areas.begin(), areas.end(), [](double lhs, double rhs) { return lhs < rhs; });

	//How many histogram bars there are.
	const int DIVISION_COUNT = 10;
	int faceAreasCount[DIVISION_COUNT];
	const double largestArea = areas.back();

	//Go through each division, and count how many models fall in that range.
	//Then update the chart to reflect this.
	QStringList categories;
	int largestSet = 0;
	int index = 0;
	for (int i = 0; i < DIVISION_COUNT; i++)
	{
		//Get the min and max values for this range.
		const double minRange = i == 0 ? 0 : (largestArea * (1.0 / DIVISION_COUNT) * (i));
		const double maxRange = largestArea * (1.0 / DIVISION_COUNT) * (1 + i);

		//Create the label
		std::ostringstream streamObj;
		streamObj << std::setprecision(2);
		streamObj << (minRange * static_cast<double>(FACE_AREA_HISTOGRAM_PRECISION)) << "-" << (maxRange * static_cast<double>(FACE_AREA_HISTOGRAM_PRECISION));
		categories << streamObj.str().c_str();

		//Get how many models lie in this range.
		faceAreasCount[i] = 0;
		while (index < areas.size() && areas[index] <= maxRange)
		{
			faceAreasCount[i]++;
			index++;
		}

		//Update the values in the histogram
		static_cast<QBarSeries*>(m_faceAreaHistogram->series()[0])->barSets()[0]->replace(i, static_cast<float>(faceAreasCount[i]) / static_cast<float>(areas.size()));
		static_cast<QBarSeries*>(m_faceAreaHistogram->series()[0])->barSets()[0]->setLabel(std::to_string(static_cast<float>(faceAreasCount[i]) / static_cast<float>(areas.size())).c_str());
		static_cast<QBarSeries*>(m_faceAreaHistogram->series()[0])->barSets()[0]->setLabelColor(QColor(0, 0, 0));
		static_cast<QBarSeries*>(m_faceAreaHistogram->series()[0])->setLabelsVisible(true);
		static_cast<QBarSeries*>(m_faceAreaHistogram->series()[0])->setLabelsPosition(QAbstractBarSeries::LabelsPosition::LabelsOutsideEnd);

		largestSet = std::max(largestSet, faceAreasCount[i]);
	}

	//Set the new range labels on the x axis and update the title.
	QBarCategoryAxis* xAxis = static_cast<QBarCategoryAxis*>(m_faceAreaHistogram->axes(Qt::Orientation::Horizontal)[0]);
	xAxis->clear();
	xAxis->append(categories);
	xAxis->setLabelsAngle(-45);

	//Set the height and make it visible
	m_faceAreaHistogram->setVisible(true);
}
