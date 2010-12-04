#include "TreePch.h"
#include "NodeProperties.h"
#include "NodePropertiesModel.h"

NodeProperties::NodeProperties (QWidget *parent)
    : QWidget (parent)
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing (2);
    layout->setContentsMargins (2,2,2,2);
    m_propView = new QTreeView(this);
    m_propView->setItemsExpandable (false);
    m_propView->setUniformRowHeights (false);
    m_propView->hide();
    m_model = new NodePropertiesModel(this);
    m_propView->setModel(m_model);
    m_alternativeMsg = new QLabel("Not availble.", this);
    layout->addWidget(m_alternativeMsg);
    layout->addWidget(m_propView);
    setLayout(layout);
}

void NodeProperties::setNode(const ArnNode *node)
{
    m_node = node;
    if (!node)
    {
        m_propView->hide();
        m_alternativeMsg->show();
    }
    else
    {
        m_model->setNode(node);
        m_propView->show();
        m_alternativeMsg->hide();
    }
}

void NodeProperties::nodeChanged (QModelIndex newIdx, QModelIndex prevIdx)
{
    setNode (reinterpret_cast<ArnNode *> (newIdx.internalPointer ()));
}
