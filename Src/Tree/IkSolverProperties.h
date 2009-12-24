#ifndef IKSOLVERPROPERTIES_H
#define IKSOLVERPROPERTIES_H

class ArnIkSolver;

class IkSolverProperties : public QWidget
{
    Q_OBJECT
public:
    explicit IkSolverProperties (QWidget *parent = 0);

public slots:
    void setIkSolvers (const std::vector <ArnIkSolver *> &ikSolvers);

private slots:
    void currentIndexChanged (int ikSolverIdx);

private:
    QComboBox *m_ikSolverList;
    QTreeView *m_ikTreeView;
    QTreeView *m_propView;
};

#endif // IKSOLVERPROPERTIES_H
