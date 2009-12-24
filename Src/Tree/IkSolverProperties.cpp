#include "TreePch.h"
#include "IkSolverProperties.h"

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
    //m_model = new NodePropertiesModel(this);
    //m_propView->setModel(m_model);
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
    const ArnIkSolver *ikSolver = m_ikSolverList->itemData(ikSolverIdx, Qt::UserRole).value<const ArnIkSolver *>();
    if (ikSolver)
    {
        std::cout << ikSolver->getSkeleton ()->getName () << std::endl;
    }
}
