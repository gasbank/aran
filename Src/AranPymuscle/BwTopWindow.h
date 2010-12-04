#pragma once

class BwAppContext;
class BwOpenGlWindow;

class BwTopWindow : public Fl_Window
{
public:
							BwTopWindow(int w, int h, const char* c, BwAppContext& ac);
	virtual					~BwTopWindow();
	virtual int				handle(int eventType);
	void					setShapeWindow(BwOpenGlWindow* sw);
	BwOpenGlWindow*			getShapeWindow() const { return m_shapeWindow; }
	void					setSceneList(Fl_Browser* sl);
private:
	BwAppContext&			m_ac;
	BwOpenGlWindow*			m_shapeWindow;
	Fl_Browser*		m_sceneList;
};
