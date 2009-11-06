#include "BwPch.h"
#include "BwDrawingOptionsWindow.h"
#include "BwAppContext.h"
#include "BwOpenGlWindow.h"

BwDrawingOptionsWindow::BwDrawingOptionsWindow( int x, int y, int w, int h, const char* c, BwAppContext& ac, BwOpenGlWindow& openGlWindow )
: Fl_Check_Browser(x, y, w, h, c)
, m_ac(ac)
, m_openGlWindow(openGlWindow)
{
	add("Grid", (int)m_ac.bRenderGrid);
	add("HUD", (int)m_ac.bRenderHud);
}

BwDrawingOptionsWindow::~BwDrawingOptionsWindow(void)
{
}

int BwDrawingOptionsWindow::handle( int eventType )
{
	if (eventType == FL_PUSH || eventType == FL_RELEASE || eventType == FL_KEYDOWN || eventType == FL_KEYUP)
	{
		m_ac.bRenderGrid = checked(1) ? true : false;
		m_ac.bRenderHud = checked(2) ? true : false;
		m_openGlWindow.redraw();
	}
	return Fl_Check_Browser::handle(eventType);
}