#include "MainWindow.h"

#include "QueryManager.h"
#include "Widgets/FeatureView.h"
#include "Widgets/DatabaseView.h"
#include "ModelAnalytics.h"
#include "ModelUtil.h"
#include "ModelSaver.h"
#include "PSBLoader.h"
#include "ModelProcessing.h"

#include <QApplication> // Used by centerAndResize
#include <QDesktopWidget> // Used by centerAndResize
#include <QStyle> // Used by centerAndResize
#include <QDockWidget>
#include <QFileDialog>
#include <QDebug>
#include <QKeyEvent>

#include <filesystem>
#include <iostream>


MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent)
{
	setupUi(this);
	//Remove unhelpfull help menu bar item
	static_cast<Ui::MainWindow*>(this)->menuBar->removeAction(menuHelp->menuAction());
	

	//setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

	QObject::connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

	_modelView = new ModelView(m_context);
	setCentralWidget(_modelView);

	_featureView = new FeatureView(m_context);
	_featureView->setAllowedAreas(Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, _featureView);

	_databaseWidget = new DatabaseView(m_context);
	_databaseWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, _databaseWidget);

	auto processModelFunc = [=]()
	{
		proc::SubdivideModel(m_context.GetActiveModel());
		proc::CrunchModel(m_context.GetActiveModel());
		m_context.GetActiveModel().m_model->markForReupload();
		_databaseWidget->Update();
	};

	QAction* importModelAction = new QAction("3D Model");
	connect(importModelAction, &QAction::triggered, this, &MainWindow::importModelFromFile);
	importDataFileMenu->addAction(importModelAction);

	QAction* exportModelAction = new QAction("3D Model");
	connect(exportModelAction, &QAction::triggered, this, &MainWindow::exportModelToFile);
	exportDataFileMenu->addAction(exportModelAction);

	addDatabaseMenuActions();
}

void MainWindow::addDatabaseMenuActions()
{
	auto processModelsFunc = [=]()
	{
		m_context.GetDatabase()->ProcessAllModels();
		_databaseWidget->Update();
	};
	
	//Importing databases
	//QAction* loadLabelledPSBAction = new QAction("Load Labelled PSB");
	//connect(loadLabelledPSBAction, &QAction::triggered, this, &MainWindow::loadLabelledPSB);
	//menuDatabase->addAction(loadLabelledPSBAction);

	QAction* loadPSBAction = new QAction("Load PSB");
	connect(loadPSBAction, &QAction::triggered, this, &MainWindow::loadPSB);
	menuDatabase->addAction(loadPSBAction);
	
	//Processing menu
	QAction* menuProcessDatabase = new QAction("Process database");
	connect(menuProcessDatabase, &QAction::triggered, this, processModelsFunc);
	menuDatabase->addAction(menuProcessDatabase);
}

MainWindow::~MainWindow()
{
}

QAction* MainWindow::addImportOption(QString menuName)
{
	return importDataFileMenu->addAction(menuName);
}

QAction* MainWindow::addExportOption(QString menuName)
{
	return exportDataFileMenu->addAction(menuName);
}

void MainWindow::importModelFromFile()
{
	QString filePath = QFileDialog::getOpenFileName(Q_NULLPTR, "Load File", "", "Model Files (*.off *.ply *)");

	// Don't try to load a file if the dialog was cancelled or the file name is empty
	if (filePath.isNull() || filePath.isEmpty())
		return;

	QFileInfo fi(filePath);
	QString fileName = fi.fileName();

	ModelDescriptor modelDescriptor;
	modelDescriptor.m_path = filePath.toStdString();
	modelDescriptor.m_name = fileName.toStdString();

	m_context.SetModel(modelDescriptor);
	m_context.GetActiveModel().UpdateFeatures();
}

void MainWindow::exportModelToFile()
{
	QString fileName = QFileDialog::getSaveFileName(Q_NULLPTR, "Save File", "", "Model Files (*.off *.ply *)");

	// Don't try to load a file if the dialog was cancelled or the file name is empty
	if (fileName.isNull() || fileName.isEmpty())
		return;

	qDebug() << "Saving Model file: " << fileName;

	ModelDescriptor& modelDescriptor = m_context.GetActiveModel();
	if(modelDescriptor.m_model != nullptr)
	{
		ModelSaver::SavePly(modelDescriptor, std::filesystem::path(fileName.toStdString()));
	}
	m_context.SetModel(modelDescriptor);
}

//void MainWindow::loadLabelledPSB()
//{
//	QString fileName = QFileDialog::getExistingDirectory(Q_NULLPTR, "Select labelled PSB directory", "");
//
//	// Don't try to load a file if the dialog was cancelled or the file name is empty
//	if (fileName.isNull() || fileName.isEmpty())
//		return;
//
//	io::LoadLabelledPSB(std::filesystem::path(fileName.toStdString()), *m_context.GetDatabase());
//	m_menuModelSelect->clear();
//	_databaseWidget->Update();
//}

void MainWindow::loadPSB()
{
	QString fileName = QFileDialog::getExistingDirectory(Q_NULLPTR, "Select PSB directory", "");

	// Don't try to load a file if the dialog was cancelled or the file name is empty
	if (fileName.isNull() || fileName.isEmpty())
		return;

	io::LoadPSB(std::filesystem::path(fileName.toStdString()), *m_context.GetDatabase());
	m_menuModelSelect->clear();
	_databaseWidget->Update();
}

void MainWindow::populateDatabaseModelSelector()
{
	if(!m_menuModelSelect->actions().empty())
	{
		return;
	}
	
	for(ModelDescriptor m : m_context.GetDatabase()->GetModelDatabase())
	{
		auto selectModelLamda = [=]()
		{
			m_context.SetModel(m);
		};
		
		QAction* modelAction = new QAction(m.m_name.c_str());
		connect(modelAction, &QAction::triggered, this, selectModelLamda);
		m_menuModelSelect->addAction(modelAction);
		m_menuModelSelect->setStyleSheet("QMenu { menu-scrollable: 1; }");

	}
}

void MainWindow::centerAndResize(float coverage) {
	// Retrieve the dimensions available on the current screen
	QSize availableSize = qApp->desktop()->availableGeometry().size();
	QSize newSize(availableSize.width() * coverage, availableSize.height() * coverage);

	qDebug() << "Available screen size " << availableSize.width() << "x" << availableSize.height();
	qDebug() << "Initial application size " << newSize.width() << "x" << newSize.height();

	setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, newSize, qApp->desktop()->availableGeometry()));
}

void MainWindow::storeLayout()
{
	_windowConfiguration = saveState();
}

void MainWindow::restoreLayout()
{
	restoreState(_windowConfiguration);
}

void MainWindow::changeEvent(QEvent *event)
{
	storeLayout();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	storeLayout();
}

void MainWindow::hideEvent(QHideEvent *event)
{
	storeLayout();
}

void MainWindow::showEvent(QShowEvent *event)
{
	restoreLayout();

	QList<QDockWidget *> dockWidgets = findChildren<QDockWidget *>();
	for (auto child : dockWidgets)
		child->setVisible(true);
}


void MainWindow::keyPressEvent(QKeyEvent* _event)
{
	
}
