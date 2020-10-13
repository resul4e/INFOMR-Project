#include "DatabaseHierarchyModel.h"

#include "Database.h"

#include <QString>

DatabaseHierarchyItem::DatabaseHierarchyItem(ModelDescriptor modelDescriptor, DatabaseHierarchyItem* parent) :
	m_modelDescriptor(modelDescriptor),
	m_parentItem(parent)
{

}

DatabaseHierarchyItem::~DatabaseHierarchyItem()
{
	qDeleteAll(m_childItems);
}

void DatabaseHierarchyItem::addChild(DatabaseHierarchyItem* item)
{
	m_childItems.append(item);
}

DatabaseHierarchyItem* DatabaseHierarchyItem::getParent()
{
	return m_parentItem;
}

DatabaseHierarchyItem* DatabaseHierarchyItem::getChild(int row)
{
	if (row < 0 || row >= m_childItems.size())
		return nullptr;
	return m_childItems[row];
}

// String that gets displayed in the data hierarchy at the given column
ModelDescriptor DatabaseHierarchyItem::getDataAtColumn(int column) const
{
	return m_modelDescriptor;
}

int DatabaseHierarchyItem::row() const
{
	if (m_parentItem)
		return m_parentItem->m_childItems.indexOf(const_cast<DatabaseHierarchyItem*>(this));
	return 0;
}

int DatabaseHierarchyItem::childCount() const
{
	return m_childItems.count();
}

int DatabaseHierarchyItem::columnCount() const
{
	return 1;
}




DatabaseHierarchyModel::DatabaseHierarchyModel(Database& database, QObject* parent) :
	QAbstractItemModel(parent),
	m_database(database)
{
	ModelDescriptor rootDescriptor;
	rootDescriptor.m_name = std::string("Database Shape Hierarchy");
	m_rootItem = new DatabaseHierarchyItem(rootDescriptor);
	setupModelData(database, m_rootItem);
}

DatabaseHierarchyModel::~DatabaseHierarchyModel()
{
	delete m_rootItem;
}

QVariant DatabaseHierarchyModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	DatabaseHierarchyItem* item = static_cast<DatabaseHierarchyItem*>(index.internalPointer());
	//if (role == Qt::DecorationRole) {
	//	return item->getIcon();
	//}
	if (role != Qt::DisplayRole)
		return QVariant();

	return QString::fromStdString(item->getDataAtColumn(index.column()).m_name);
}

QModelIndex DatabaseHierarchyModel::index(int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	DatabaseHierarchyItem* parentItem;

	if (!parent.isValid())
		parentItem = m_rootItem;
	else
		parentItem = static_cast<DatabaseHierarchyItem*>(parent.internalPointer());

	DatabaseHierarchyItem* childItem = parentItem->getChild(row);
	if (childItem)
		return createIndex(row, column, childItem);
	return QModelIndex();
}

QModelIndex DatabaseHierarchyModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	DatabaseHierarchyItem* childItem = static_cast<DatabaseHierarchyItem*>(index.internalPointer());
	DatabaseHierarchyItem* parentItem = childItem->getParent();

	if (parentItem == m_rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int DatabaseHierarchyModel::rowCount(const QModelIndex& parent) const
{
	DatabaseHierarchyItem* parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = m_rootItem;
	else
		parentItem = static_cast<DatabaseHierarchyItem*>(parent.internalPointer());

	return parentItem->childCount();
}

int DatabaseHierarchyModel::columnCount(const QModelIndex& parent) const
{
	if (parent.isValid())
		return static_cast<DatabaseHierarchyItem*>(parent.internalPointer())->columnCount();
	return m_rootItem->columnCount();
}

DatabaseHierarchyItem* DatabaseHierarchyModel::getItem(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return nullptr;

	if (role != Qt::DisplayRole)
		return nullptr;

	DatabaseHierarchyItem* item = static_cast<DatabaseHierarchyItem*>(index.internalPointer());

	return item;
}

Qt::ItemFlags DatabaseHierarchyModel::flags(const QModelIndex& index) const
{
	// If the index is negative or doesn't belong to this model, return no item flags
	if (!index.isValid())
	{
		return Qt::NoItemFlags;
	}
	return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | QAbstractItemModel::flags(index);
}


QVariant DatabaseHierarchyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return QString::fromStdString(m_rootItem->getDataAtColumn(section).m_name);

	return QVariant();
}

void DatabaseHierarchyModel::setupModelData(Database& database, DatabaseHierarchyItem* parent)
{
	for (const ModelDescriptor& modelDescriptor : database.GetModelDatabase())
	{
		DatabaseHierarchyItem* shapeItem = new DatabaseHierarchyItem(modelDescriptor, m_rootItem);

		m_rootItem->addChild(shapeItem);
	}
}
