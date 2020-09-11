#include "DatabaseView.h"
#include "FeatureView.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>

#include "Database.h"
#include <qcoreapplication.h>

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

	QLineEdit* databaseCountField = createField("0");

	QSpacerItem* spacer = new QSpacerItem(0, 1000);

	viewLayout->addWidget(databaseCountLabel, 0, 0);
	viewLayout->addWidget(databaseCountField, 0, 1);
	viewLayout->addItem(spacer, 1, 0);

	QGroupBox* settingsBox = new QGroupBox("Basic features");
	settingsBox->setLayout(viewLayout);

	mainlayout->addWidget(settingsBox);
}
