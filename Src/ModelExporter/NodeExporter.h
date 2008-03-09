#pragma once

class NodeExporter
{
public:
	NodeExporter(IGameNode* node);
	virtual ~NodeExporter(void);

	virtual void make();
	
	IGameNode* getNode() const { return m_node; }
private:
	IGameNode* m_node;
};
