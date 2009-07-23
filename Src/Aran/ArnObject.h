#pragma once

class Uncopyable
{
protected:
	Uncopyable() {}
	~Uncopyable() {}
private:
	Uncopyable(const Uncopyable&);
	Uncopyable& operator=(const Uncopyable&);

};

class ArnObject : private Uncopyable
{
public:
	virtual					~ArnObject(void);

	NODE_DATA_TYPE			getType() const { return m_type; }
	virtual const char*		getName() const = 0;
	unsigned int			getObjectId() const { return m_objectId; }
protected:
							ArnObject(NODE_DATA_TYPE type);
private:
	const NODE_DATA_TYPE	m_type;
	const unsigned int		m_objectId;
	static unsigned int		ms_objectId;
};
