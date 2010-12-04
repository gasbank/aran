#ifndef NODEPROPERTIESMODEL_H
#define NODEPROPERTIESMODEL_H

class NodePropertiesModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    NodePropertiesModel (QObject *parent = 0);
    ~NodePropertiesModel ();

    void setNode(const ArnNode *node);

    QVariant data (const QModelIndex &index, int role) const;
    Qt::ItemFlags flags (const QModelIndex &index) const;
    QVariant headerData (int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index (int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent (const QModelIndex &index) const;
    int rowCount (const QModelIndex &parent = QModelIndex()) const;
    int columnCount (const QModelIndex &parent = QModelIndex()) const;

private:
    const ArnNode *m_node;

    QList<QVariant> m_header;
    QStringList m_members;
};

#endif // NODEPROPERTIESMODEL_H
