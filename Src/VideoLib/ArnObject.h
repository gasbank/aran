#pragma once

class ArnObject
{
public:
	ArnObject(ArnNodeType type);
	virtual ~ArnObject(void);
	ArnNodeType getType() { return m_type; }
	virtual const char* getName() const = 0;
private:
	const ArnNodeType m_type;
};
