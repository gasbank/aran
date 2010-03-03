#ifndef IkSolverNodeModel_H
#define IkSolverNodeModel_H

class IkSolverNodeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    IkSolverNodeModel (QObject *parent = 0);
    ~IkSolverNodeModel ();

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

#endif // IkSolverNodeModel_H
