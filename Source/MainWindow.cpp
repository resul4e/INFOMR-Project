#include "MainWindow.h"

#include "QueryManager.h"
#include "FeatureView.h"
#include "ModelLoader.h"
#include "Normalizer.h"

#include <QApplication> // Used by centerAndResize
#include <QDesktopWidget> // Used by centerAndResize
#include <QStyle> // Used by centerAndResize
#include <QDockWidget>
#include <QFileDialog>
#include <QDebug>

#include <filesystem>
#include <iostream>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);
	

	//setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    QObject::connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    _modelViewer = new ModelViewer();
    setCentralWidget(_modelViewer);

	_featureWidget = new FeatureView();
	_featureWidget->setAllowedAreas(Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, _featureWidget);

    m_queryManager = std::make_shared<QueryManager>();

    QAction* importModelAction = new QAction("3D Model");
    connect(importModelAction, &QAction::triggered, this, &MainWindow::importModelFromFile);
    importDataFileMenu->addAction(importModelAction);

    QAction* normalizeModelAction = new QAction("Normalize Model");
    connect(normalizeModelAction, &QAction::triggered, this, &MainWindow::normalizeCurrentModel);
    menuFile->insertAction(exitAction, normalizeModelAction);

    addDatabaseMenuActions();
}

void MainWindow::addDatabaseMenuActions()
{
    auto processModelsFunc = [=]()
    {
        m_queryManager->GetDatabase()->ProcessAllModels();
    };

	auto sortByVertexCountFunc = [=]()
	{
		m_queryManager->GetDatabase()->SortDatabase(Database::SortingOptions::VERTEX_COUNT);
		m_menuModelSelect->clear();
	};

	auto sortByFaceCountFunc = [=]()
	{
		m_queryManager->GetDatabase()->SortDatabase(Database::SortingOptions::FACE_COUNT);
		m_menuModelSelect->clear();
	};

	auto sortByBoundsFunc = [=]()
	{
		m_queryManager->GetDatabase()->SortDatabase(Database::SortingOptions::BOUNDS);
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
    QString fileName = QFileDialog::getOpenFileName(Q_NULLPTR, "Load File", "", "Model Files (*.off *.ply *)");

    // Don't try to load a file if the dialog was cancelled or the file name is empty
    if (fileName.isNull() || fileName.isEmpty())
        return;

    qDebug() << "Loading Model file: " << fileName;

    std::shared_ptr<Model> model = ModelLoader::LoadModel(std::filesystem::path(fileName.toStdString()));

	selectModel(model);
}

void MainWindow::loadLabelledPSB()
{
    QString fileName = QFileDialog::getExistingDirectory(Q_NULLPTR, "Select labelled PSB directory", "");

    // Don't try to load a file if the dialog was cancelled or the file name is empty
    if (fileName.isNull() || fileName.isEmpty())
        return;

    m_queryManager->GetDatabase()->LoadLabelledPSB(std::filesystem::path(fileName.toStdString()));
    m_menuModelSelect->clear();
}

void MainWindow::loadPSB()
{
    QString fileName = QFileDialog::getExistingDirectory(Q_NULLPTR, "Select PSB directory", "");

    // Don't try to load a file if the dialog was cancelled or the file name is empty
    if (fileName.isNull() || fileName.isEmpty())
        return;

    m_queryManager->GetDatabase()->LoadPSB(std::filesystem::path(fileName.toStdString()));
    m_menuModelSelect->clear();
}

void MainWindow::populateDatabaseModelSelector()
{
	if(!m_menuModelSelect->actions().empty())
	{
        return;
	}
	
	for(std::shared_ptr<Model> m : m_queryManager->GetDatabase()->GetModelDatabase())
	{
        auto selectModelLamda = [=]()
        {
            selectModel(m);
        };
		
        QAction* modelAction = new QAction(m->m_name.c_str());
        connect(modelAction, &QAction::triggered, this, selectModelLamda);
        m_menuModelSelect->addAction(modelAction);
        m_menuModelSelect->setStyleSheet("QMenu { menu-scrollable: 1; }");

	}
}

void MainWindow::selectModel(std::shared_ptr<Model> _model)
{
    _modelViewer->setModel(_model);
	_featureWidget->SetModel(_model);
}

void MainWindow::normalizeCurrentModel()
{
	std::shared_ptr<Model> model = _modelViewer->getModel();
	if(model != nullptr)
    {
        Normalizer::Normalize(*model);
    }
    else
    {
        std::cerr << "No Model to perform normalization on!" << std::endl;
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
