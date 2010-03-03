#ifndef IKSOLVERPROPERTIES_H
#define IKSOLVERPROPERTIES_H

class ArnIkSolver;
class IkSolverTreeModel;
class IkSolverNodeModel;

class IkSolverProperties : public QWidget
{
    Q_OBJECT
public:
    explicit IkSolverProperties (QWidget *parent = 0);

    int getCurrentIkSolverIndex () const;
public slots:
    void setIkSolvers (const std::vector <ArnIkSolver *> &ikSolvers);
    void currentIndexChanged (int ikSolverIdx);

private slots:
    void nodeChanged (QModelIndex newIdx, QModelIndex prevIdx);
    void resizeIkTreeViewColumn(QModelIndex col);

private:
    QComboBox *m_ikSolverList;
    QTreeView *m_ikTreeView;
    IkSolverTreeModel *m_treeModel;
    QTreeView *m_nodePropView;
    IkSolverNodeModel *m_nodeModel;
};

#endif // IKSOLVERPROPERTIES_H
