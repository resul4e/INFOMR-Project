#include "MainWindow.h"

#include "QueryManager.h"
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
	

    QObject::connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    _modelViewer = new ModelViewer();
    setCentralWidget(_modelViewer);

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
    QAction* loadLabelledPSBAction = new QAction("Load Labelled PSB");
    connect(loadLabelledPSBAction, &QAction::triggered, this, &MainWindow::loadLabelledPSB);
    menuDatabase->addAction(loadLabelledPSBAction);

    QAction* loadPSBAction = new QAction("Load PSB");
    connect(loadPSBAction, &QAction::triggered, this, &MainWindow::loadPSB);
    menuDatabase->addAction(loadPSBAction);
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

    _modelViewer->setModel(model);
}

void MainWindow::loadLabelledPSB()
{
    QString fileName = QFileDialog::getExistingDirectory(Q_NULLPTR, "Select labelled PSB directory", "");

    // Don't try to load a file if the dialog was cancelled or the file name is empty
    if (fileName.isNull() || fileName.isEmpty())
        return;

    m_queryManager->LoadLabelledPSB(std::filesystem::path(fileName.toStdString()));
}

void MainWindow::loadPSB()
{
    QString fileName = QFileDialog::getExistingDirectory(Q_NULLPTR, "Select labelled PSB directory", "");

    // Don't try to load a file if the dialog was cancelled or the file name is empty
    if (fileName.isNull() || fileName.isEmpty())
        return;

    m_queryManager->LoadPSB(std::filesystem::path(fileName.toStdString()));
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
