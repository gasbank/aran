#include "TreePch.h"
#include "NodePropertiesModel.h"

NodePropertiesModel::NodePropertiesModel (QObject *parent)
    : QAbstractItemModel (parent)
    , m_node(0)
{
    m_header << "Member" << "Value";
    m_members << "Name" << "Parent" << "Children" << "Type"
            << "Auto Local R" << "Auto Local S" << "Auto Local T";
}

NodePropertiesModel::~NodePropertiesModel ()
{

}

int NodePropertiesModel::columnCount (const QModelIndex &parent) const
{
    return 2;
}

QVariant NodePropertiesModel::data (const QModelIndex &index, int role) const
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
            const ArnXformable * xformable =
                    dynamic_cast<const ArnXformable *> (m_node);
            ArnVec3 scale, trans;
            ArnQuat rot;
            if (xformable)
            {
                const ArnMatrix &t = xformable->getAutoLocalXform();
                ArnMatrixDecompose(&scale, &rot, &trans, &t);
            }

            if (index.row () == 0)
                return QVariant (m_node->getName ());
            else if (index.row() == 1)
            {
                if (m_node->getParent ())
                    return QVariant (m_node->getParent ()->getName ());
                else
                    return QVariant ();
            }
            else if (index.row () == 2)
            {
                return QVariant ((int)m_node->getChildren ().size ());
            }
            else if (index.row () == 3)
            {
                char name[64];
                ArnGetNameFromRtNdt (name, m_node->getType ());
                return QVariant (name);
            }
            else if ((index.row () == 4) && xformable)
            {
                ArnVec3 euler (ArnQuatToEuler (&rot));
                QString str (tr ("%1, %2, %3").arg (euler.x).arg (euler.y).arg (euler.z));
                return QVariant (str);
            }
            else if ((index.row () == 5) && xformable)
            {
                QString str (tr ("%1, %2, %3").arg (scale.x).arg (scale.y).arg (scale.z));
                return QVariant (str);
            }
            else if ((index.row () == 6) && xformable)
            {
                ArnVec3 euler (ArnQuatToEuler (&rot));
                QString str (tr ("%1, %2, %3").arg (trans.x).arg (trans.y).arg (trans.z));
                return QVariant (str);
            }
            else
                return QVariant ();
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

Qt::ItemFlags NodePropertiesModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant NodePropertiesModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_header.value(section);

    return QVariant();
}

QModelIndex NodePropertiesModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, 0);
}

QModelIndex NodePropertiesModel::parent (const QModelIndex &index) const
{
    return QModelIndex();
}

int NodePropertiesModel::rowCount (const QModelIndex &parent) const
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

void NodePropertiesModel::setNode(const ArnNode *node)
{
    m_node = node;
    emit reset();
}
