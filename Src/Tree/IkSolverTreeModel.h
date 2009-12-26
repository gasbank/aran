#ifndef IKSOLVERTREEMODEL_H
#define IKSOLVERTREEMODEL_H

class IkSolverTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    IkSolverTreeModel (QObject *parent = 0);
    ~IkSolverTreeModel ();

    void setNode(const Node *node);

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
    const Node *m_node;

    QList<QVariant> m_header;
    QStringList m_members;
};

#endif // IKSOLVERTREEMODEL_H
