#include "BwPch.h"
#include "BwTopWindow.h"
#include "BwMain.h"
#include "BwOpenGlWindow.h"
#include "BwAppContext.h"

BwTopWindow::BwTopWindow( int w,int h, const char* c, BwAppContext& ac )
: Fl_Window(w, h, c)
, m_shapeWindow(0)
, m_ac(ac)
{
}


BwTopWindow::~BwTopWindow()
{
}

int BwTopWindow::handle( int eventType )
{
	if (eventType == FL_KEYUP)
	{
		int key = Fl::event_key();
		if (key < 256)
			m_ac.bHoldingKeys[key] = false;
	}
	else if (eventType == FL_KEYDOWN)
	{
		
	}
	return Fl_Window::handle(eventType);
}

void BwTopWindow::setShapeWindow( BwOpenGlWindow* sw )
{
	m_shapeWindow = sw;
}

void BwTopWindow::setSceneList( Fl_Select_Browser* sl )
{
	m_sceneList = sl;
}

void BwTopWindow::setDrawingOptionsWindow( Fl_Check_Browser* dow )
{
	m_drawingOptionsWindow = dow;
}