#pragma once

#include "ModelViewer.h"

#include "../Resources/ui_MainWindow.h"

#include <QMainWindow>
#include <QAction>
#include <QByteArray>

class QueryManager;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow, private Ui::MainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
	~MainWindow() override;

    /**
    * Adds a new item to the import menu.
    */
    QAction* addImportOption(QString menuName);

    /**
    * Adds a new item to the export menu.
    */
    QAction* addExportOption(QString menuName);

    void importModelFromFile();

	/**
	 * @brief Gets path to database directory from user and Calls QueryManager::LoadLabelledPSB
	 * @note Also clears the database model selector
	*/
	void loadLabelledPSB();
    /**
     * @brief Gets path to database directory from user and Calls QueryManager::LoadPSB
     * @note Also clears the database model selector
    */
    void loadPSB();

    /**
     * @brief GO through all of the models in the database and add them to the menu (only if the list is empty)
    */
    void populateDatabaseModelSelector();
    /**
     * @brief Set the model to the selected model in the Model database selector.
     * @param _model The model to display on the screen.
    */
    void selectModel(std::shared_ptr<Model> _model);

    void normalizeCurrentModel();

    /**
     * Moves the window to the center of the screen and resizes it to a fraction
     * of the total screen size defined by 'coverage'.
     */
    void centerAndResize(float coverage);

public slots:
    /**
    * Store the current window layout so we can restore it later
    */
    void storeLayout();

    /**
    * Restore the configuration of dockable widgets
    */
    void restoreLayout();

private:
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void showEvent(QShowEvent *event) override;

    void addDatabaseMenuActions();
	
private:
    QMenu* m_menuModelSelect;
    QByteArray _windowConfiguration;

    ModelViewer* _modelViewer;
    std::shared_ptr<QueryManager> m_queryManager;
};
