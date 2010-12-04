#include "TreePch.h"
#include "IkSolverTreeModel.h"


IkSolverTreeModel::IkSolverTreeModel (QObject *parent)
        : QAbstractItemModel (parent)
        , m_node(0)
{
    m_header << "Name" << "Type";
}

IkSolverTreeModel::~IkSolverTreeModel ()
{

}

void IkSolverTreeModel::setNode(const Node *node)
{
    m_node = node;
    emit reset();
}

QVariant IkSolverTreeModel::data (const QModelIndex &index, int role) const
{
    if (!index.isValid ())
        return QVariant ();

    if (role != Qt::DisplayRole)
        return QVariant ();

    Node *item = static_cast <Node *> (index.internalPointer ());
    if (index.column () == 0)
    {
        if (strlen(item->getName ()))
            return QVariant(item->getName ());
        else
            return QVariant ("<Unnamed>");
    }
    else if (index.column () == 1)
    {
        return QVariant ();
    }
    else
    {
        return QVariant ();
    }
}

Qt::ItemFlags IkSolverTreeModel::flags (const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant IkSolverTreeModel::headerData (int section, Qt::Orientation orientation,
                                        int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_header.value(section);

    return QVariant();
}

QModelIndex IkSolverTreeModel::index (int row, int column,
                                      const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    const Node *parentItem;

    if (!parent.isValid())
        parentItem = m_node;
    else
        parentItem = static_cast<Node *>(parent.internalPointer());

    Node *childItem = parentItem->getChildNodeAt(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex IkSolverTreeModel::parent (const QModelIndex &index) const
{
    if (!index.isValid ())
        return QModelIndex ();

    Node *childItem = static_cast <Node *> (index.internalPointer ());
    Node *parentItem = childItem->getRealParent ();

    if (parentItem == m_node)
        return QModelIndex ();

    assert (parentItem);
    int row = 0;
    if (parentItem->getRealParent ())
        row = parentItem->getRealParent ()->getIndexOfChildNode (parentItem);

    //std::cout << row << std::endl;
    return createIndex(row, 0, parentItem);
}

int IkSolverTreeModel::rowCount (const QModelIndex &parent) const
{
    const Node *parentItem;

    if (!parent.isValid())
        parentItem = m_node;
    else
        parentItem = static_cast <Node *> (parent.internalPointer());

    int ret = 0;
    if (parentItem)
        ret = parentItem->getChildNodeCount();

    //std::cout << ret << std::endl;
    return ret;
}

int IkSolverTreeModel::columnCount (const QModelIndex &parent) const
{
    return 2;
}
