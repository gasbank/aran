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
	BwAppContext&				m_ac;
	int							sides;
	int							overlay_sides;
};


struct HitRecord
{
	GLuint numNames;	// Number of names in the name stack for this hit record
	GLuint minDepth;	// Minimum depth value of primitives (range 0 to 2^32-1)
	GLuint maxDepth;	// Maximum depth value of primitives (range 0 to 2^32-1)
	GLuint contents;	// Name stack contents
};
