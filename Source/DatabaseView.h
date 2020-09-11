#pragma once
#include <QDockWidget>

class Database;

class DatabaseView : public QDockWidget
{
	Q_OBJECT
public:
	DatabaseView(std::shared_ptr<Database> _database);

private:
	std::shared_ptr<Database> m_database;
};
