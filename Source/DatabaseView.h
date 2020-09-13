#pragma once
#include <QDockWidget>
#include <QLineEdit>

class Database;
namespace QtCharts
{
	class QBarSet;
	class QChart;
	class QBarCategoryAxis;
	class QValueAxis;
}


class DatabaseView : public QDockWidget
{
	Q_OBJECT
public:
	DatabaseView(std::shared_ptr<Database> _database);

	void Update();
	
private:
	QLineEdit* m_databaseCountField;
	QtCharts::QChart* m_vertexCountHistogram;
	
	std::shared_ptr<Database> m_database;
};
