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

#include "Database.h"
#include "Model.h"

using namespace QtCharts;

DatabaseView::DatabaseView(std::shared_ptr<Database> _database) :
	m_database(_database)
{
	setWindowTitle("Database View");
	setMinimumWidth(250);

	QWidget* mainWidget = new QWidget(this);
	QLayout* mainlayout = new QGridLayout(mainWidget);
	mainlayout->setMargin(0);
	setWidget(mainWidget);

	QGridLayout* viewLayout = new QGridLayout();
	QLabel* databaseCountLabel = new QLabel("Database entries");

	m_databaseCountField = createField("0");
	
	//chart
	QBarSet* vertexCountSet = new QBarSet("Database");

	*vertexCountSet << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0;

	QBarSeries* series = new QBarSeries();
	series->append(vertexCountSet);

	m_vertexCountHistogram = new QChart();
	m_vertexCountHistogram->addSeries(series);
	m_vertexCountHistogram->legend()->setVisible(false);
	m_vertexCountHistogram->setTitle("Vertex count distribution");
	m_vertexCountHistogram->setAnimationOptions(QChart::SeriesAnimations);
	m_vertexCountHistogram->setMaximumHeight(450);

	QStringList categories;
	categories << "0" << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10";
	QBarCategoryAxis* vertexCountHistogramXAxis = new QBarCategoryAxis();
	vertexCountHistogramXAxis->append(categories);
	m_vertexCountHistogram->addAxis(vertexCountHistogramXAxis, Qt::AlignBottom);
	series->attachAxis(vertexCountHistogramXAxis);

	QValueAxis* vertexCountHistogramYAxis = new QValueAxis();
	vertexCountHistogramYAxis->setRange(0, 10);
	m_vertexCountHistogram->addAxis(vertexCountHistogramYAxis, Qt::AlignLeft);
	series->attachAxis(vertexCountHistogramYAxis);

	QChartView* chartView = new QChartView(m_vertexCountHistogram);
	chartView->setRenderHint(QPainter::Antialiasing);
	
	QSpacerItem* spacer = new QSpacerItem(0, 1000);

	viewLayout->addWidget(databaseCountLabel, 0, 0);
	viewLayout->addWidget(m_databaseCountField, 0, 1);
	viewLayout->addWidget(chartView, 1, 0);
	viewLayout->addItem(spacer, 1, 0);

	QGroupBox* settingsBox = new QGroupBox("Basic features");
	settingsBox->setLayout(viewLayout);

	mainlayout->addWidget(settingsBox);
}

void DatabaseView::Update()
{
	m_databaseCountField->setText(std::to_string(m_database->GetModelDatabase().size()).c_str());

	QStringList categories;
	
	const int DIVISION_COUNT = 10;

	int modelCount[DIVISION_COUNT];
	std::vector<std::shared_ptr<Model>> modelDatabase = m_database->GetModelDatabase();
	std::sort(modelDatabase.begin(), modelDatabase.end(), [](std::shared_ptr<Model> lhs, std::shared_ptr<Model> rhs) { return lhs->m_vertexCount < rhs->m_vertexCount; });
	int largestModelVertexCount = modelDatabase.back()->m_vertexCount;
	int largestSet = 0;
	
	int index = 0;
	for (int i = 0; i < DIVISION_COUNT; i++)
	{
		int prev = i == 0 ? 0 : (largestModelVertexCount * 0.1 * (i)) + 1;
		int desc = largestModelVertexCount * 0.1 * (i + 1);
		categories << (std::to_string(prev) + "-" + std::to_string(desc)).c_str();
		modelCount[i] = 0;

		while(index < modelDatabase.size() && modelDatabase[index]->m_vertexCount <= desc)
		{
			modelCount[i]++;
			index++;
		}
		static_cast<QBarSeries*>(m_vertexCountHistogram->series()[0])->barSets()[0]->replace(i, modelCount[i]);
		static_cast<QBarSeries*>(m_vertexCountHistogram->series()[0])->barSets()[0]->setLabel(std::to_string(modelCount[i]).c_str());
		static_cast<QBarSeries*>(m_vertexCountHistogram->series()[0])->barSets()[0]->setLabelColor(QColor(0,0,0));
		static_cast<QBarSeries*>(m_vertexCountHistogram->series()[0])->setLabelsVisible(true);
		static_cast<QBarSeries*>(m_vertexCountHistogram->series()[0])->setLabelsPosition(QtCharts::QAbstractBarSeries::LabelsPosition::LabelsOutsideEnd);
		largestSet = std::max(largestSet, modelCount[i]);
	}
	
	
	
	m_vertexCountHistogram->axisY()->setRange(0, largestSet + 50);

	static_cast<QBarCategoryAxis*>(m_vertexCountHistogram->axisX())->clear();
	static_cast<QBarCategoryAxis*>(m_vertexCountHistogram->axisX())->append(categories);
}
