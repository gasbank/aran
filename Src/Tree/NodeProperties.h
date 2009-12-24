#ifndef NODEPROPERTIES_H
#define NODEPROPERTIES_H

class NodePropertiesModel;

class NodeProperties : public QWidget
{
    Q_OBJECT
public:
    explicit NodeProperties (QWidget *parent = 0);

    void setNode(const ArnNode *node);
signals:

public slots:
    void nodeChanged (QModelIndex newIdx, QModelIndex prevIdx);

private:
    QTreeView *m_propView;
    QLabel *m_alternativeMsg;
    NodePropertiesModel *m_model;
    const ArnNode *m_node;
};

#endif // NODEPROPERTIES_H
