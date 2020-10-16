#include "MainWindow.h"

#include "QueryManager.h"
#include "Widgets/FeatureView.h"
#include "Widgets/DatabaseView.h"
#include "Normalizer.h"

#include <QApplication> // Used by centerAndResize
#include <QDesktopWidget> // Used by centerAndResize
#include <QStyle> // Used by centerAndResize
#include <QDockWidget>
#include <QFileDialog>
#include <QDebug>
#include <QKeyEvent>

#include <filesystem>
#include <iostream>
#include "ModelSaver.h"


MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	m_selectedModelIndex(0)
{
	setupUi(this);
	

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
		m_context.GetDatabase()->SubdivideModel(m_context.GetActiveModel());
		m_context.GetDatabase()->CrunchModel(m_context.GetActiveModel());
		m_context.GetActiveModel().m_model->markForReupload();
		_databaseWidget->Update();
	};

	QAction* importModelAction = new QAction("3D Model");
	connect(importModelAction, &QAction::triggered, this, &MainWindow::importModelFromFile);
	importDataFileMenu->addAction(importModelAction);

	QAction* exportModelAction = new QAction("3D Model");
	connect(exportModelAction, &QAction::triggered, this, &MainWindow::exportModelToFile);
	exportDataFileMenu->addAction(exportModelAction);

	QAction* processModelAction = new QAction("Process Model");
	connect(processModelAction, &QAction::triggered, this, processModelFunc);
	menuFile->insertAction(exitAction, processModelAction);
	
	QAction* normalizeModelAction = new QAction("Normalize Model");
	connect(normalizeModelAction, &QAction::triggered, this, &MainWindow::normalizeCurrentModel);
	menuFile->insertAction(exitAction, normalizeModelAction);

	QAction* remeshModelAction = new QAction("Remesh Model");
	connect(remeshModelAction, &QAction::triggered, this, &MainWindow::remeshCurrentModel);
	menuFile->insertAction(exitAction, remeshModelAction);

	addDatabaseMenuActions();
}

void MainWindow::addDatabaseMenuActions()
{
	auto processModelsFunc = [=]()
	{
		m_context.GetDatabase()->ProcessAllModels();
		_databaseWidget->Update();
	};

	auto remeshModelsFunc = [=]()
	{
		m_context.GetDatabase()->RemeshAllModels();
		_databaseWidget->Update();
	};

	auto saveModifiedModelsFunc = [=]()
	{
		m_context.GetDatabase()->SaveAllModels();
	};
	
	auto normalizeModelsFunc = [=]()
	{
		m_context.GetDatabase()->NormalizeAllModels();
		_databaseWidget->Update();
	};
	
	auto sortByVertexCountFunc = [=]()
	{
		m_context.GetDatabase()->SortDatabase(Database::SortingOptions::VERTEX_COUNT);
		m_menuModelSelect->clear();
	};

	auto sortByFaceCountFunc = [=]()
	{
		m_context.GetDatabase()->SortDatabase(Database::SortingOptions::FACE_COUNT);
		m_menuModelSelect->clear();
	};

	auto sortByBoundsFunc = [=]()
	{
		m_context.GetDatabase()->SortDatabase(Database::SortingOptions::BOUNDS);
		m_menuModelSelect->clear();
	};
	
	//Importing databases
	QAction* loadLabelledPSBAction = new QAction("Load Labelled PSB");
	connect(loadLabelledPSBAction, &QAction::triggered, this, &MainWindow::loadLabelledPSB);
	menuDatabase->addAction(loadLabelledPSBAction);

	QAction* loadPSBAction = new QAction("Load PSB");
	connect(loadPSBAction, &QAction::triggered, this, &MainWindow::loadPSB);
	menuDatabase->addAction(loadPSBAction);

	//Model selector
	m_menuModelSelect = new QMenu("Select Model");
	connect(m_menuModelSelect, &QMenu::aboutToShow, this, &MainWindow::populateDatabaseModelSelector);
	menuDatabase->addMenu(m_menuModelSelect);

	//Sorting menu
	QMenu* menuSortDatabase = new QMenu("Sort database by...");
	menuDatabase->addMenu(menuSortDatabase);

	QAction* sortByVertexCount = new QAction("Vertex Count");
	connect(sortByVertexCount, &QAction::triggered, this, sortByVertexCountFunc);
	menuSortDatabase->addAction(sortByVertexCount);

	QAction* sortByFaceCount = new QAction("Face Count");
	connect(sortByFaceCount, &QAction::triggered, this, sortByFaceCountFunc);
	menuSortDatabase->addAction(sortByFaceCount);

	QAction* sortByBounds = new QAction("Bounds");
	connect(sortByBounds, &QAction::triggered, this, sortByBoundsFunc);
	menuSortDatabase->addAction(sortByBounds);
	
	//Processing menu
	QAction* menuProcessDatabase = new QAction("Process database");
	connect(menuProcessDatabase, &QAction::triggered, this, processModelsFunc);
	menuDatabase->addAction(menuProcessDatabase);

	QAction* menuRemeshDatabase = new QAction("Remesh database");
	connect(menuRemeshDatabase, &QAction::triggered, this, remeshModelsFunc);
	menuDatabase->addAction(menuRemeshDatabase);
	
	QAction* menuNormalizeDatabase = new QAction("Normalize database");
	connect(menuNormalizeDatabase, &QAction::triggered, this, normalizeModelsFunc);
	menuDatabase->addAction(menuNormalizeDatabase);

	QAction* menuSaveDatabase = new QAction("Save modified database");
	connect(menuSaveDatabase, &QAction::triggered, this, saveModifiedModelsFunc);
	menuDatabase->addAction(menuSaveDatabase);
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

void MainWindow::loadLabelledPSB()
{
	QString fileName = QFileDialog::getExistingDirectory(Q_NULLPTR, "Select labelled PSB directory", "");

	// Don't try to load a file if the dialog was cancelled or the file name is empty
	if (fileName.isNull() || fileName.isEmpty())
		return;

	m_context.GetDatabase()->LoadLabelledPSB(std::filesystem::path(fileName.toStdString()));
	m_menuModelSelect->clear();
	_databaseWidget->Update();
}

void MainWindow::loadPSB()
{
	QString fileName = QFileDialog::getExistingDirectory(Q_NULLPTR, "Select PSB directory", "");

	// Don't try to load a file if the dialog was cancelled or the file name is empty
	if (fileName.isNull() || fileName.isEmpty())
		return;

	m_context.GetDatabase()->LoadPSB(std::filesystem::path(fileName.toStdString()));
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

void MainWindow::normalizeCurrentModel()
{
	ModelDescriptor& modelDescriptor = m_context.GetActiveModel();
	if(modelDescriptor.m_model != nullptr)
	{
		//Normalizer::Remesh(*model);
		Normalizer::Normalize(modelDescriptor);

		m_context.SetModel(modelDescriptor);
	}
	else
	{
		std::cerr << "No Model to perform normalization on!" << std::endl;
	}
}

void MainWindow::remeshCurrentModel()
{
	ModelDescriptor& modelDescriptor = m_context.GetActiveModel();
	if (modelDescriptor.m_model != nullptr)
	{
		Normalizer::Remesh(modelDescriptor);
		m_context.SetModel(modelDescriptor);
	}
	else
	{
		std::cerr << "No Model to perform remeshing on!" << std::endl;
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
	
	std::cout << _event->key() << std::endl;
	if(_event->key() == Qt::Key::Key_M)
	{
		m_selectedModelIndex++;
	}
	else if (_event->key() == Qt::Key::Key_N)
	{
		m_selectedModelIndex--;
	}

	std::vector<ModelDescriptor> db = m_context.GetDatabase()->GetModelDatabase();
	if(m_selectedModelIndex < db.size())
	{
		m_context.SetModel(db[m_selectedModelIndex]);
	}
}
