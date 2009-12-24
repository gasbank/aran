#include "TreePch.h"
#include "SceneGraphModel.h"

SceneGraphModel::SceneGraphModel (QObject *parent)
    : QAbstractItemModel (parent)
    , m_rootItem (0)
{
    m_header << "Name" << "Type";
}

SceneGraphModel::~SceneGraphModel ()
{

}

int SceneGraphModel::columnCount (const QModelIndex &parent) const
{
    return 2;
}

QVariant SceneGraphModel::data (const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    ArnNode *item = static_cast<ArnNode*>(index.internalPointer());

    if (index.column() == 0)
    {
        if (strlen(item->getName()))
            return QVariant(item->getName());
        else
            return QVariant("<Unnamed>");
    }
    else if (index.column() == 1)
    {
        char name[64];
        ArnGetNameFromRtNdt(name, item->getType());
        return QVariant(name);
    }
    else
    {
        return QVariant();
    }
}

Qt::ItemFlags SceneGraphModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant SceneGraphModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_header.value(section);

    return QVariant();
}

QModelIndex SceneGraphModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    const ArnNode *parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<ArnNode*>(parent.internalPointer());

    ArnNode *childItem = parentItem->getNodeAt(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex SceneGraphModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    ArnNode *childItem = static_cast<ArnNode*>(index.internalPointer());
    ArnNode *parentItem = childItem->getParent();

    if (parentItem == m_rootItem)
        return QModelIndex();

    int row = 0;
    if (parentItem->getParent())
        row = parentItem->getParent()->getIndexOfChild(parentItem);
    return createIndex(row, 0, parentItem);
}

int SceneGraphModel::rowCount(const QModelIndex &parent) const
{
    const ArnNode *parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<ArnNode*>(parent.internalPointer());

    if (parentItem)
        return parentItem->getChildren().size();
    else
        return 0;
}

void SceneGraphModel::setRootItem (const ArnNode *rootItem)
{
    m_rootItem = rootItem;
    emit reset();
}
