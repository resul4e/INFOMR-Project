#pragma once
#include <QDockWidget>
#include <QLineEdit>

class Database;
namespace QtCharts
{
	class QBarSet;
	class QChart;
	class QBarCategoryAxis;
}


class DatabaseView : public QDockWidget
{
	Q_OBJECT
public:
	DatabaseView(std::shared_ptr<Database> _database);

	void Update();
	
private:
	QLineEdit* m_databaseCountField;
	QtCharts::QBarSet* m_set0;
	QtCharts::QChart* m_chart;
	QtCharts::QBarCategoryAxis* m_XAxis;
	
	std::shared_ptr<Database> m_database;
};
