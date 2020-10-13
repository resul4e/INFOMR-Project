#pragma once

#include <QAbstractItemModel>

#include <QVector>

class Database;

class DatabaseHierarchyItem
{
public:
	DatabaseHierarchyItem(QString shapeName, DatabaseHierarchyItem* parent = nullptr);

	~DatabaseHierarchyItem();

	void addChild(DatabaseHierarchyItem* item);

	DatabaseHierarchyItem* getParent();
	DatabaseHierarchyItem* getChild(int row);

	QString getDataAtColumn(int column) const;
	int row() const;

	int childCount() const;
	int columnCount() const;

private:
	DatabaseHierarchyItem* m_parentItem;

	QVector<DatabaseHierarchyItem*> m_childItems;

	QString m_shapeName;
};

class DatabaseHierarchyModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	explicit DatabaseHierarchyModel(Database& database, QObject* parent = nullptr);

	~DatabaseHierarchyModel();

	/**
	 * Mandatory override for QAbstractItemModel.
	 */
	QVariant data(const QModelIndex& index, int role) const override;

	/**
	 * Mandatory override for QAbstractItemModel. Provides an index associated
	 * to a particular data item at location (row, column).
	 */
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

	/**
	 * Mandatory override for QAbstractItemModel. Returns the index of the parent
	 * of this item. If this item is not a child, an invalid index is returned.
	 */
	QModelIndex parent(const QModelIndex& index) const override;

	/**
	 * Mandatory override for QAbstractItemModel.
	 * Returns the number of children of this parent item.
	 */
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;

	/**
	* Mandatory override for QAbstractItemModel.
	* Returns number of columns associated with this parent item.
	*/
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	DatabaseHierarchyItem* getItem(const QModelIndex& index, int role) const;

	Qt::ItemFlags flags(const QModelIndex& index) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	void setupModelData(Database& database, DatabaseHierarchyItem* parent);

	/** Root node of the data hierarchy */
	DatabaseHierarchyItem* m_rootItem;

	/** Reference to database to be able to query for data */
	Database& m_database;
};
