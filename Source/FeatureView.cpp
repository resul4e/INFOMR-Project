#include "FeatureView.h"

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

FeatureView::FeatureView()
{
	setWindowTitle("Feature View");
	setMinimumWidth(250);

	QWidget* mainWidget = new QWidget(this);
	QLayout* mainlayout = new QGridLayout(mainWidget);
	mainlayout->setMargin(0);
	setWidget(mainWidget);

	QGridLayout* viewLayout = new QGridLayout();
	QLabel* volumeLabel = new QLabel("Volume");
	QLabel* surfaceAreaLabel = new QLabel("Surface Area");
	QLabel* vsaRatioLabel = new QLabel("Volume-to-Surface Area Ratio");
	QLabel* bbaRatioLabel = new QLabel("Bounding-Box Aspect Ratio");

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
	//viewLayout->setColumnStretch(0, 1);
	viewLayout->addItem(spacer, 4, 0);

	QGroupBox* settingsBox = new QGroupBox("Basic features");
	settingsBox->setLayout(viewLayout);

	mainlayout->addWidget(settingsBox);
}
