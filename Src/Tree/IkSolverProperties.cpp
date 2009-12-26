#include "TreePch.h"
#include "IkSolverProperties.h"
#include "IkSolverTreeModel.h"

IkSolverProperties::IkSolverProperties (QWidget *parent)
    : QWidget (parent)
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing (2);
    layout->setContentsMargins (2,2,2,2);
    m_ikSolverList = new QComboBox(this);
    m_ikSolverList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_ikTreeView = new QTreeView(this);
    m_propView = new QTreeView(this);
    m_treeModel = new IkSolverTreeModel(this);
    m_ikTreeView->setModel(m_treeModel);
    //m_alternativeMsg = new QLabel("Not availble.", this);

    QHBoxLayout *list = new QHBoxLayout();
    list->addWidget (new QLabel ("IK Solver of "));
    list->addWidget (m_ikSolverList);
    list->addStretch();

    layout->addLayout (list);
    layout->addWidget (m_ikTreeView);
    layout->addWidget (m_propView);
    setLayout (layout);

    connect (m_ikSolverList, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));
}

void IkSolverProperties::setIkSolvers (const std::vector <ArnIkSolver *> &ikSolvers)
{
    m_ikSolverList->clear();
    foreach (const ArnIkSolver *ikSolver, ikSolvers)
    {
        m_ikSolverList->addItem (ikSolver->getSkeleton ()->getName (), qVariantFromValue(ikSolver));
    }
}

void IkSolverProperties::currentIndexChanged (int ikSolverIdx)
{
    const ArnIkSolver *ikSolver = 0;
    if (ikSolverIdx >= 0)
        ikSolver = m_ikSolverList->itemData(ikSolverIdx, Qt::UserRole).value<const ArnIkSolver *>();

    if (ikSolver)
    {
        m_treeModel->setNode (ikSolver->getTree ()->GetRoot ());
    }
    else
    {
        m_treeModel->setNode (0);
    }
}

int IkSolverProperties::getCurrentIkSolverIndex () const
{
    return m_ikSolverList->currentIndex ();
}
