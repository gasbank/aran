#pragma once

class ArnObject
{
public:
	ArnObject(NODE_DATA_TYPE type);
	virtual ~ArnObject(void);
	NODE_DATA_TYPE getType() { return m_type; }
	virtual const char* getName() const = 0;
private:
	const NODE_DATA_TYPE m_type;
};
