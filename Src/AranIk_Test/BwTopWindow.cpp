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
		int key = Fl::event_key();
		if (key < 256)
			m_ac.bHoldingKeys[key] = true;

		if (key == FL_KP + '7')
		{
			m_ac.viewMode = VM_TOP;
			printf("  View mode set to top.\n");
			m_shapeWindow->redraw();
		}
		else if (key == FL_KP + '3')
		{
			m_ac.viewMode = VM_LEFT;
			printf("  View mode set to left.\n");
			m_shapeWindow->redraw();
		}
		else if (key == FL_KP + '1')
		{
			m_ac.viewMode = VM_FRONT;
			printf("  View mode set to front.\n");
			m_shapeWindow->redraw();
		}
		else if (key == FL_KP + '4')
		{
			m_ac.viewMode = VM_CAMERA;
			printf("  View mode set to camera.\n");
			m_shapeWindow->redraw();
		}
	}
	return Fl_Window::handle(eventType);
}

void BwTopWindow::setShapeWindow( BwOpenGlWindow* sw )
{
	m_shapeWindow = sw;
}
