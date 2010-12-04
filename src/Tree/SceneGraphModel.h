#ifndef SCENEGRAPHMODEL_H
#define SCENEGRAPHMODEL_H

class SceneGraphModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    SceneGraphModel(QObject *parent = 0);
    ~SceneGraphModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void setRootItem (const ArnNode *rootItem);

private:
    const ArnNode *m_rootItem;

    QList<QVariant> m_header;
};

#endif // SCENEGRAPHMODEL_H
