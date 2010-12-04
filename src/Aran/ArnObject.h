#pragma once

class ARAN_API Uncopyable
{
protected:
	Uncopyable() {}
	virtual ~Uncopyable() {}
private:
	Uncopyable(const Uncopyable&);
	Uncopyable& operator=(const Uncopyable&);

};

#define ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING

class ARAN_API ArnObject : private Uncopyable
{
public:
	virtual							~ArnObject(void);
	NODE_DATA_TYPE					getType() const { return m_type; }
	virtual const char*				getName() const = 0;
	unsigned int					getObjectId() const { return m_objectId; }
#ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
	static unsigned int				getCtorCount() { return ms_ctorCount; }
	static unsigned int				getDtorCount() { return ms_dtorCount; }
	static ArnObject*				getObjectById(unsigned int objId);
	static void						printInstances();
#endif // #ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
protected:
									ArnObject(NODE_DATA_TYPE type);
private:
	const NODE_DATA_TYPE			m_type;
	const unsigned int				m_objectId;
	static unsigned int				ms_objectId;
#ifdef ARNOBJECT_GLOBAL_MANAGEMENT_FOR_DEBUGGING
	static unsigned int				ms_ctorCount;
	static unsigned int				ms_dtorCount;
	static std::set<ArnObject*>		ms_instances;
#endif
};
