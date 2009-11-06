#pragma once

class BwAppContext;
class BwOpenGlWindow;

class BwDrawingOptionsWindow : public Fl_Check_Browser
{
public:
	BwDrawingOptionsWindow(int, int, int, int, const char*, BwAppContext& ac, BwOpenGlWindow& openGlWindow);
	~BwDrawingOptionsWindow(void);

	int handle(int);
private:
	BwAppContext& m_ac;
	BwOpenGlWindow& m_openGlWindow;
};
