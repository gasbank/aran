#include "TreePch.h"
#include "IkSolverNodeModel.h"

QString VectorR3ToString(const VectorR3 &v)
{
    return QString("%1, %2, %3").arg(v.x, 0, 'f', 3).arg(v.y, 0, 'f', 3).arg(v.z, 0, 'f', 3);
}

IkSolverNodeModel::IkSolverNodeModel (QObject *parent)
    : QAbstractItemModel (parent)
    , m_node(0)
{
    m_header << "Member" << "Value";
    m_members << "Name" << "Parent" << "Children" << "Type" << "Global position" << "Relative position" << "Joint angle" << "Rotation axis" << "Size" << "Frozen" << "Range";
}

IkSolverNodeModel::~IkSolverNodeModel ()
{
}

int IkSolverNodeModel::columnCount (const QModelIndex &parent) const
{
    return 2;
}

QVariant IkSolverNodeModel::data (const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    if (index.column() == 0)
    {
        return m_members[index.row()];
    }
    else if (index.column() == 1)
    {
        if (m_node)
        {
            if (index.row () == 0)
                return QVariant (m_node->getName ());
            else if (index.row() == 1)
            {
                if (m_node->getRealParent ())
                    return QVariant (m_node->getRealParent ()->getName ());
                else
                    return QVariant ();
            }
            else if (index.row () == 2)
            {
                return QVariant ((int)m_node->getChildNodeCount ());
            }
            else if (index.row () == 3)
            {
                char name[64];
                QString type;
                if (m_node->isEndeffector ())
                    type += tr("Endeffector(%1)").arg(m_node->getEffectorNum ());
                if (m_node->isJoint ())
                {
                    if (type.length ())
                        type += " / ";
                    type += tr("Joint(%1)").arg(m_node->getJointNum ());
                }

                return QVariant (type);
            }
            else if (index.row () == 4)
            {
                return QVariant (VectorR3ToString (m_node->getGlobalPosition ()));
            }
            else if (index.row () == 5)
            {
                return QVariant (VectorR3ToString (m_node->getRelativePosition ()));
            }
            else if (index.row () == 6)
            {
                return QVariant (m_node->getJointAngle ());
            }
            else if (index.row () == 7)
            {
                return QVariant (VectorR3ToString (m_node->getRotationAxis ()));
            }
            else if (index.row () == 8)
            {
                return QVariant (m_node->getSize ());
            }
            else if (index.row () == 9)
            {
                return QVariant (m_node->isFrozen ());
            }
            else if (index.row () == 10)
            {
                return QVariant (QString("%1 ~ %2").arg(m_node->getMinTheta()).arg(m_node->getMaxTheta()));
            }
            else
            {
                return QVariant ();
            }
        }
        else
        {
            return QVariant();
        }
    }
    else
    {
        return QVariant();
    }
}

Qt::ItemFlags IkSolverNodeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant IkSolverNodeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_header.value(section);

    return QVariant();
}

QModelIndex IkSolverNodeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, 0);
}

QModelIndex IkSolverNodeModel::parent (const QModelIndex &index) const
{
    return QModelIndex();
}

int IkSolverNodeModel::rowCount (const QModelIndex &parent) const
{
    if (!parent.isValid ())
    {
        return m_members.count ();
    }
    else
    {
        return 0;
    }
}

void IkSolverNodeModel::setNode(const Node *node)
{
    m_node = node;
    emit reset();
}
