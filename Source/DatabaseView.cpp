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
	m_set0 = new QBarSet("Jane");

	*m_set0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0;

	QBarSeries* series = new QBarSeries();
	series->append(m_set0);

	m_chart = new QChart();
	m_chart->addSeries(series);
	m_chart->setTitle("Vertex count distribution");
	m_chart->setAnimationOptions(QChart::SeriesAnimations);

	QStringList categories;
	categories << "0" << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10";
	m_XAxis = new QBarCategoryAxis();
	m_XAxis->append(categories);
	m_chart->addAxis(m_XAxis, Qt::AlignBottom);
	series->attachAxis(m_XAxis);

	QValueAxis* axisY = new QValueAxis();
	axisY->setRange(0, 900);
	m_chart->addAxis(axisY, Qt::AlignLeft);
	series->attachAxis(axisY);

	QChartView* chartView = new QChartView(m_chart);
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

	int index = 0;
	for (int i = 0; i < DIVISION_COUNT; i++)
	{
		int prev = i == 0 ? 0 : (largestModelVertexCount * 0.1 * (i)) + 1;
		int desc = largestModelVertexCount * 0.1 * (i + 1);
		categories << (std::to_string(prev) + "-" + std::to_string(desc)).c_str();
		modelCount[i] = 0;

		while(modelDatabase[index]->m_vertexCount < desc && index < modelDatabase.size())
		{
			modelCount[i]++;
			index++;
		}
		m_set0->replace(i, modelCount[i]);
	}

	m_XAxis->clear();
	m_XAxis->append(categories);
}
