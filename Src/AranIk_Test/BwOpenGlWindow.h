#pragma once

class BwAppContext;

class BwOpenGlWindow : public Fl_Gl_Window
{
public:
								BwOpenGlWindow(int x,int y,int w,int h, const char *l, BwAppContext& ac);
	virtual						~BwOpenGlWindow();
	virtual int					handle(int);
	virtual void				resize(int,int,int,int);
	
private:
	void						draw();
	void						draw_overlay();
	BwAppContext&					m_ac;
	int							sides;
	int							overlay_sides;
};
