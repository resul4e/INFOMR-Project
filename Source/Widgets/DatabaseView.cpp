#include "DatabaseView.h"
#include "FeatureView.h"

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

#include "Database.h"
#include "Model.h"

#include <QDebug>
using namespace QtCharts;

DatabaseHierarchy::DatabaseHierarchy(Context& _context) :
	m_context(_context)
{
	setFixedWidth(200);

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

DatabaseView::DatabaseView(Context& _context) :
	m_context(_context),
	m_maxVertexCount(100)
{
	setObjectName("DatabaseView");
	setWindowTitle("Database View");
	setMinimumWidth(250);

	QWidget* mainWidget = new QWidget(this);
	QLayout* mainlayout = new QGridLayout(mainWidget);
	mainlayout->setMargin(0);
	setWidget(mainWidget);

	QLabel* databaseCountLabel = new QLabel("Database entries");

	m_databaseCountField = createField("0");

	m_databaseHierarchy = new DatabaseHierarchy(_context);

	m_vertexCountHistogram = CreateVertexCountChart();
	m_vertexCountSlider = new QSlider();
	m_vertexCountSlider->setOrientation(Qt::Orientation::Horizontal);
	m_vertexCountSlider->setTickPosition(QSlider::TickPosition::TicksBelow);
	m_vertexCountSlider->setMaximum(m_maxVertexCount);
	m_vertexCountSlider->setMinimum(10);
	m_vertexCountSlider->setEnabled(false);

	QLineEdit* vertexCountSliderField = createField("0");
	auto changeChartRange = [=]()
	{
		m_maxVertexCount = m_vertexCountSlider->value();
		vertexCountSliderField->setText(std::to_string(m_vertexCountSlider->value()).c_str());
		Update();
	};
	changeChartRange();
	connect(m_vertexCountSlider, &QSlider::sliderReleased, this, changeChartRange);


	QChartView* chartView = new QChartView(m_vertexCountHistogram);
	chartView->setRenderHint(QPainter::Antialiasing);

	QGridLayout* informationLayout = new QGridLayout();
	QGridLayout* chartsLayout = new QGridLayout();
	informationLayout->addWidget(databaseCountLabel, 0, 0);
	informationLayout->addWidget(m_databaseCountField, 0, 1);
	informationLayout->addWidget(m_databaseHierarchy, 1, 0);
	chartsLayout->addWidget(m_vertexCountSlider, 0, 0);
	chartsLayout->addWidget(vertexCountSliderField, 0, 1);
	chartsLayout->addWidget(chartView, 1, 0);

	
	QGroupBox* informationBox = new QGroupBox("Database information");
	informationBox->setLayout(informationLayout);

	QGroupBox* chartsBox = new QGroupBox("Charts");
	chartsBox->setLayout(chartsLayout);

	mainlayout->addWidget(informationBox);
	mainlayout->addWidget(chartsBox);
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
	vertexCountHistogram->setMaximumHeight(450);
	vertexCountHistogram->setMinimumHeight(450);

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
	const int largestModelVertexCount = std::min(static_cast<int>(modelDatabase.back().m_vertexCount), m_maxVertexCount);

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
