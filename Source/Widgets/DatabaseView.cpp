#include "DatabaseView.h"
#include "FeatureView.h"
#include "FeatureExtraction.h"

#include <QChartView>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QBarSet>
#include <QBarSeries>
#include <qbarcategoryaxis.h>
#include <qvalueaxis.h>
#include <QSlider>
#include <QPushButton>

#include "Database.h"
#include "Model.h"

#include <chrono>
#include <fstream>
#include <sstream>
#include <QDebug>

using namespace QtCharts;

DatabaseHierarchy::DatabaseHierarchy(Context& _context) :
	m_context(_context)
{
	//setFixedWidth(200);

	m_model = new DatabaseHierarchyModel(*_context.GetDatabase());

	// Create the tree view that shows all shapes
	setSelectionMode(QAbstractItemView::SingleSelection);
	setModel(m_model);
}

void DatabaseHierarchy::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QModelIndexList indices = selected.indexes();
	if (!indices.isEmpty())
	{
		QModelIndex selectedIndex = indices[0];
		DatabaseHierarchyItem* item = m_model->getItem(selectedIndex, Qt::DisplayRole);
		ModelDescriptor modelDescriptor = item->getDataAtColumn(0);
		m_context.SetModel(modelDescriptor);
		qDebug() << QString::fromStdString(modelDescriptor.m_path.string());
	}
}

void DatabaseHierarchy::UpdateDataModel()
{
	m_model = new DatabaseHierarchyModel(*m_context.GetDatabase());
	setModel(m_model);
}

void DatabaseView::FindClosestShapes()
{
	int k = m_querySizeInput->text().toInt();

	auto& modelDatabase = m_context.GetDatabase()->GetModelDatabase();

	std::vector<int> closestIndices;

	//std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	//closestIndices = m_context.GetDatabase()->FindClosestKNNShapes(m_context.GetActiveModel(), k);
	//std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	std::chrono::steady_clock::time_point begin2 = std::chrono::steady_clock::now();
	closestIndices = m_context.GetDatabase()->FindClosestANNShapes(m_context.GetActiveModel(), k);
	std::chrono::steady_clock::time_point end2 = std::chrono::steady_clock::now();

	//std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
	std::cout << "ANN Query Time = " << std::chrono::duration_cast<std::chrono::microseconds> (end2 - begin2).count() << "[µs]" << std::endl;

	m_matchList->clear();

	for (int i = 0; i < k; i++)
	{
		ModelDescriptor& closest = modelDatabase[closestIndices[i]];
		QString s = QString::fromStdString(closest.m_name);
		m_matchList->addItem(s);
	}
}

void DatabaseView::FindClosestShapesRadius()
{
	float r = m_queryRadiusInput->text().toFloat();

	auto& modelDatabase = m_context.GetDatabase()->GetModelDatabase();

	std::vector<int> closestIndices;

	closestIndices = m_context.GetDatabase()->FindClosestANNShapesRadius(m_context.GetActiveModel(), r);

	m_matchList->clear();

	for (int i = 0; i < closestIndices.size(); i++)
	{
		ModelDescriptor& closest = modelDatabase[closestIndices[i]];
		QString s = QString::fromStdString(closest.m_name);
		m_matchList->addItem(s);
	}
}

DatabaseView::DatabaseView(Context& _context) :
	m_context(_context),
	m_maxVertexCount(100)
{
	setObjectName("DatabaseView");
	setWindowTitle("Database View");
	setMinimumWidth(425);
	setMaximumWidth(425);

	QWidget* mainWidget = new QWidget(this);
	QLayout* mainlayout = new QGridLayout(mainWidget);
	mainlayout->setMargin(0);
	setWidget(mainWidget);

	connect(&m_context, &Context::modelChanged, this, &DatabaseView::OnModelChanged);

	QLabel* databaseCountLabel = new QLabel("Database entries");

	m_databaseCountField = createField("0");

	m_databaseHierarchy = new DatabaseHierarchy(_context);
	m_databaseHierarchy->setMinimumWidth(200);
	m_databaseHierarchy->setMaximumWidth(200);

	m_computeSimilarK = new QPushButton("Search similar");
	m_computeSimilarK->setMinimumHeight(40);
	connect(m_computeSimilarK, &QPushButton::pressed, this, &DatabaseView::FindClosestShapes);

	m_computeSimilarR = new QPushButton("Search similar (Radius)");
	m_computeSimilarR->setMinimumHeight(40);
	connect(m_computeSimilarR, &QPushButton::pressed, this, &DatabaseView::FindClosestShapesRadius);

	QLabel* kLabel = new QLabel("Query Size k=");
	m_querySizeInput = new QLineEdit();
	m_querySizeInput->setText("10");

	QLabel* rLabel = new QLabel("Query Radius r=");
	m_queryRadiusInput = new QLineEdit();

	m_matchList = new QListWidget();
	m_matchList->setMinimumWidth(200);
	m_matchList->setMaximumWidth(200);
	m_matchList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	connect(m_matchList, &QListWidget::itemSelectionChanged, this, &DatabaseView::SimilarModelSelectionChanged);

	m_vertexCountHistogram = CreateVertexCountChart();
	m_vertexCountSlider = new QSlider();
	m_vertexCountSlider->setOrientation(Qt::Orientation::Horizontal);
	m_vertexCountSlider->setTickPosition(QSlider::TickPosition::TicksBelow);
	m_vertexCountSlider->setMaximum(m_maxVertexCount);
	m_vertexCountSlider->setMinimum(10);
	m_vertexCountSlider->setEnabled(false);
	m_vertexCountSlider->setVisible(false);

	m_faceAreaHistogram = CreateFaceAreaHistogram();
	//m_faceAreaHistogram->setVisible(false);

	QChartView* faceChartView = new QChartView(m_faceAreaHistogram);
	faceChartView->setRenderHint(QPainter::Antialiasing);
	faceChartView->setMinimumWidth(400);
	faceChartView->setMaximumHeight(300);

	QLineEdit* vertexCountSliderField = createField("0");
	auto changeChartRange = [=]()
	{
		m_maxVertexCount = m_vertexCountSlider->value();
		vertexCountSliderField->setText(std::to_string(m_vertexCountSlider->value()).c_str());
		Update();
	};
	vertexCountSliderField->setVisible(false);
	changeChartRange();
	connect(m_vertexCountSlider, &QSlider::sliderReleased, this, changeChartRange);

	QChartView* chartView = new QChartView(m_vertexCountHistogram);
	chartView->setRenderHint(QPainter::Antialiasing);
	chartView->setMinimumWidth(400);
	chartView->setMaximumHeight(300);

	QGridLayout* informationLayout = new QGridLayout();
	QGridLayout* chartsLayout = new QGridLayout();
	informationLayout->addWidget(databaseCountLabel, 0, 0);

	QHBoxLayout* kSelectionLayout = new QHBoxLayout();
	kSelectionLayout->addWidget(kLabel);
	kSelectionLayout->addWidget(m_querySizeInput);

	QHBoxLayout* rSelectionLayout = new QHBoxLayout();
	rSelectionLayout->addWidget(rLabel);
	rSelectionLayout->addWidget(m_queryRadiusInput);

	informationLayout->addWidget(m_databaseCountField, 0, 2);
	informationLayout->addLayout(kSelectionLayout, 1, 0);
	//informationLayout->addLayout(rSelectionLayout, 2, 0);
	informationLayout->addWidget(m_computeSimilarK, 2, 0);
	//informationLayout->addWidget(m_computeSimilarR, 3, 3);
	informationLayout->addWidget(m_databaseHierarchy, 3, 0, 1, 3, Qt::AlignHCenter);
	informationLayout->addWidget(m_matchList, 3, 3, 1, 3, Qt::AlignHCenter);
	chartsLayout->addWidget(m_vertexCountSlider, 0, 0);
	chartsLayout->addWidget(vertexCountSliderField, 0, 1);
	chartsLayout->addWidget(chartView, 1, 0, 1, 4, Qt::AlignHCenter);
	chartsLayout->addWidget(faceChartView, 2, 0, 1, 4, Qt::AlignHCenter);
	
	QGroupBox* informationBox = new QGroupBox("Database information");
	informationBox->setLayout(informationLayout);

	QGroupBox* chartsBox = new QGroupBox("Charts");
	chartsBox->setLayout(chartsLayout);

	mainlayout->addWidget(informationBox);
	mainlayout->addWidget(chartsBox);
}

void DatabaseView::OnModelChanged()
{
	const ModelDescriptor& modelDescriptor = m_context.GetActiveModel();

	if (modelDescriptor.m_model != nullptr)
		UpdateFaceAreaHistogram(modelDescriptor);
}


QChart* DatabaseView::CreateVertexCountChart()
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
	vertexCountHistogram->setTitle("Vertex count distribution");
	vertexCountHistogram->setAnimationOptions(QChart::SeriesAnimations);
	vertexCountHistogram->setMinimumWidth(400);
	vertexCountHistogram->setMaximumHeight(300);
	vertexCountHistogram->setMinimumHeight(300);

	//Create the x axis categories, these are also dummy values and will be updated.
	QStringList categories;
	categories << "0" << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10";

	//Create the x axis and attach it to the chart.
	QBarCategoryAxis* vertexCountHistogramXAxis = new QBarCategoryAxis();
	vertexCountHistogramXAxis->append(categories);
	vertexCountHistogram->addAxis(vertexCountHistogramXAxis, Qt::AlignBottom);
	series->attachAxis(vertexCountHistogramXAxis);

	//Create the y axis and attach it to the chart.
	QValueAxis* vertexCountHistogramYAxis = new QValueAxis();
	vertexCountHistogramYAxis->setRange(0, 10);
	vertexCountHistogram->addAxis(vertexCountHistogramYAxis, Qt::AlignLeft);
	series->attachAxis(vertexCountHistogramYAxis);

	return vertexCountHistogram;
}

QChart* DatabaseView::CreateFaceAreaHistogram()
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

	vertexCountHistogram->setMinimumWidth(400);
	vertexCountHistogram->setMinimumHeight(300);
	vertexCountHistogram->setPreferredHeight(300);
	vertexCountHistogram->setMaximumHeight(300);

	return vertexCountHistogram;
}

void DatabaseView::UpdateFaceAreaHistogram(const ModelDescriptor& _modelDescriptor)
{
	//get the areas of each of the triangles in the model
	std::vector<double> areas = ExtractFaceAreas(_modelDescriptor);
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

void DatabaseView::Update()
{
	std::vector<ModelDescriptor> modelDatabase = m_context.GetDatabase()->GetModelDatabase();

	//If we call update without any items in the database, we can't do much.
	if(modelDatabase.empty())
	{
		return;
	}

	//Update the number of entries in the database.
	m_databaseCountField->setText(std::to_string(modelDatabase.size()).c_str());

	//Update the hierarchy
	m_databaseHierarchy->UpdateDataModel();

	//The amount of bars in the chart
	const int DIVISION_COUNT = 10;
	//How many models fall within each of the ranges in the chart.
	int modelCount[DIVISION_COUNT];

	//Sort the model from largest to smallest and get the largest vertex count (or the range set by the slider)
	std::sort(modelDatabase.begin(), modelDatabase.end(), [](ModelDescriptor& lhs, ModelDescriptor& rhs) { return lhs.m_vertexCount < rhs.m_vertexCount; });
	const int largestModelVertexCount = modelDatabase.back().m_vertexCount; //std::min(static_cast<int>(modelDatabase.back().m_vertexCount), m_maxVertexCount);

	//Go through each division, and count how many models fall in that range.
	//Then update the chart to reflect this.
	QStringList categories;
	int largestSet = 0;
	int index = 0;
	for (int i = 0; i < DIVISION_COUNT; i++)
	{
		//Get the min and max values for this range.
		const int minRange = i == 0 ? 0 : (largestModelVertexCount * 0.1 * (i)) + 1;
		const int maxRange = largestModelVertexCount * 0.1 * (i + 1);

		//Create the label
		categories << (std::to_string(minRange) + "-" + std::to_string(maxRange)).c_str();

		//Get how many models lie in this range.
		modelCount[i] = 0;
		while(index < modelDatabase.size() && modelDatabase[index].m_vertexCount <= maxRange)
		{
			modelCount[i]++;
			index++;
		}

		//Update chart.
		static_cast<QBarSeries*>(m_vertexCountHistogram->series()[0])->barSets()[0]->replace(i, modelCount[i]);
		static_cast<QBarSeries*>(m_vertexCountHistogram->series()[0])->barSets()[0]->setLabel(std::to_string(modelCount[i]).c_str());
		static_cast<QBarSeries*>(m_vertexCountHistogram->series()[0])->barSets()[0]->setLabelColor(QColor(0,0,0));
		static_cast<QBarSeries*>(m_vertexCountHistogram->series()[0])->setLabelsVisible(true);
		static_cast<QBarSeries*>(m_vertexCountHistogram->series()[0])->setLabelsPosition(QtCharts::QAbstractBarSeries::LabelsPosition::LabelsOutsideEnd);

		//Update the largest set var.
		largestSet = std::max(largestSet, modelCount[i]);
	}

	//Update the vertex count slider.
	m_vertexCountSlider->setMaximum(modelDatabase.back().m_vertexCount);
	m_vertexCountSlider->setTickInterval(modelDatabase.back().m_vertexCount * 0.1);
	m_vertexCountSlider->setEnabled(true);

	//Set the correct range for the y axis
	QAbstractAxis* yAxis = m_vertexCountHistogram->axes(Qt::Orientation::Vertical)[0];
	yAxis->setRange(0, largestSet + static_cast<int>(largestSet * 0.1f));

	//Set the new range labels on the x axis.
	QBarCategoryAxis* xAxis = static_cast<QBarCategoryAxis*>(m_vertexCountHistogram->axes(Qt::Orientation::Horizontal)[0]);
	xAxis->clear();
	xAxis->append(categories);
}

void DatabaseView::SimilarModelSelectionChanged()
{
	if(m_matchList->selectedItems().size() <= 0)
	{
		return;
	}
	QListWidgetItem* item = m_matchList->selectedItems()[0];
	
	ModelDescriptor desc = m_context.GetDatabase()->FindModelByName(item->text().toStdString());
	m_context.SetModel(desc);
}
