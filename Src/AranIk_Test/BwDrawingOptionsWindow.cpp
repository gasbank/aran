#include "BwPch.h"
#include "BwDrawingOptionsWindow.h"
#include "BwAppContext.h"
#include "BwOpenGlWindow.h"

BwDrawingOptionsWindow::BwDrawingOptionsWindow( int x, int y, int w, int h, const char* c, BwAppContext& ac, BwOpenGlWindow& openGlWindow )
: Fl_Check_Browser(x, y, w, h, c)
, m_ac(ac)
, m_openGlWindow(openGlWindow)
{
	add("Grid",				(int)m_ac.bDrawGrid);
	add("HUD",				(int)m_ac.bDrawHud);
	add("Joint",			(int)m_ac.bDrawJointIndicator);
	add("Endeffector",		(int)m_ac.bDrawEndeffectorIndicator);
	add("Joint Axis",		(int)m_ac.bDrawJointAxisIndicator);
	add("Contact",			(int)m_ac.bDrawContactIndicator);
	add("Contact Force",	(int)m_ac.bDrawContactForaceIndicator);
	add("Root Node",		(int)m_ac.bDrawRootNodeIndicator);
}

BwDrawingOptionsWindow::~BwDrawingOptionsWindow(void)
{
}

int BwDrawingOptionsWindow::handle( int eventType )
{
	if (eventType == FL_PUSH || eventType == FL_RELEASE || eventType == FL_KEYDOWN || eventType == FL_KEYUP)
	{
		m_ac.bDrawGrid						= checked(1) ? true : false;
		m_ac.bDrawHud						= checked(2) ? true : false;
		m_ac.bDrawJointIndicator			= checked(3) ? true : false;
		m_ac.bDrawEndeffectorIndicator		= checked(4) ? true : false;
		m_ac.bDrawJointAxisIndicator		= checked(5) ? true : false;
		m_ac.bDrawContactIndicator			= checked(6) ? true : false;
		m_ac.bDrawContactForaceIndicator	= checked(7) ? true : false;
		m_ac.bDrawRootNodeIndicator			= checked(8) ? true : false;
		m_openGlWindow.redraw();
	}
	return Fl_Check_Browser::handle(eventType);
}