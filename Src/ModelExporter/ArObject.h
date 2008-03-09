#pragma once

class ArObject
{
public:
	ArObject(void);
	ArObject(const char* name) { setName(name); }
	virtual ~ArObject(void);

	void setName(const char* name)		{ m_name = name; }
	const char* getName() const		{ return m_name.c_str(); }

private:
	/*static const int MAX_NAME_LENGTH = 128;
	char m_name[MAX_NAME_LENGTH];*/
	std::string m_name;
};
