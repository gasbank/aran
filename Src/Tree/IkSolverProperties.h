#ifndef IKSOLVERPROPERTIES_H
#define IKSOLVERPROPERTIES_H

class ArnIkSolver;
class IkSolverTreeModel;

class IkSolverProperties : public QWidget
{
    Q_OBJECT
public:
    explicit IkSolverProperties (QWidget *parent = 0);

    int getCurrentIkSolverIndex () const;
public slots:
    void setIkSolvers (const std::vector <ArnIkSolver *> &ikSolvers);
    void currentIndexChanged (int ikSolverIdx);

private:
    QComboBox *m_ikSolverList;
    QTreeView *m_ikTreeView;
    IkSolverTreeModel *m_treeModel;
    QTreeView *m_propView;
};

#endif // IKSOLVERPROPERTIES_H
