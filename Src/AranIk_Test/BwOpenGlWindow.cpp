#include "BwPch.h"
#include "BwOpenGlWindow.h"
#include "BwAppContext.h"
#include "BwMain.h"

BwOpenGlWindow::BwOpenGlWindow(int x,int y,int w,int h,const char *l, BwAppContext& ac)
: Fl_Gl_Window(x, y, w, h, l)
, m_ac(ac)
{
	sides = overlay_sides = 3;

	m_ac.windowWidth = w;
	m_ac.windowHeight = h;
	m_ac.avd.Width	= m_ac.windowWidth;
	m_ac.avd.Height	= m_ac.windowHeight;
}

BwOpenGlWindow::~BwOpenGlWindow()
{
}

void BwOpenGlWindow::resize( int x, int y, int w, int h )
{
	m_ac.windowWidth = w;
	m_ac.windowHeight = h;
	m_ac.avd.Width	= m_ac.windowWidth;
	m_ac.avd.Height	= m_ac.windowHeight;
	//printf("Resized OpenGL widget size is %d x %d.\n", m_ac.windowWidth, m_ac.windowHeight);
	Fl_Gl_Window::resize(x, y, w, h);
}

void BwOpenGlWindow::draw()
{
	// the valid() property may be used to avoid reinitializing your
	// GL transformation for each redraw:
	if (!valid()) {
		valid(1);
		glLoadIdentity();
		glViewport(0, 0, w(), h());
	}

	RenderScene(m_ac);

}

void BwOpenGlWindow::draw_overlay()
{

}

int BwOpenGlWindow::handle( int eventType )
{
	static std::pair<int, int> mousePosition;

	if (eventType == FL_PUSH)
	{
		return 1;
	}
	else if (eventType == FL_RELEASE)
	{
		return 1;
	}
	else if (eventType == FL_ENTER)
	{
		take_focus();
		return 1;
	}
	else if (eventType == FL_FOCUS)
	{
		return 1;
	}
	else if (eventType == FL_UNFOCUS)
	{
		return 1;
	}
	else if (eventType == FL_MOVE)
	{
		mousePosition.first = Fl::event_x();
		mousePosition.second = Fl::event_y();

		//printf("%d %d\n", mousePosition.first, mousePosition.second);

		if (m_ac.bPanningButtonDown)
		{
			int dx = mousePosition.first - m_ac.panningStartPoint.first;
			int dy = mousePosition.second - m_ac.panningStartPoint.second;
			const float aspectRatio = (float)m_ac.windowWidth / m_ac.windowHeight;
			m_ac.dPanningCenter.first = -(2.0f * m_ac.orthoViewDistance * aspectRatio / m_ac.windowWidth) * dx;
			m_ac.dPanningCenter.second = -(-2.0f * m_ac.orthoViewDistance / m_ac.windowHeight) * dy;

			//printf("%f   %f\n", m_ac.dPanningCenter.first, m_ac.dPanningCenter.second);

			redraw();
		}
	}
	else if (eventType == FL_MOUSEWHEEL)
	{
		if (m_ac.viewMode == VM_TOP || m_ac.viewMode == VM_LEFT || m_ac.viewMode == VM_FRONT)
		{
			if (Fl::event_dy() > 0)
			{
				++m_ac.orthoViewDistance;
				redraw();
			}
			else if (Fl::event_dy() < 0)
			{
				if (m_ac.orthoViewDistance > 1)
					--m_ac.orthoViewDistance;
				redraw();
			}
		}
	}
	else if (eventType == FL_KEYDOWN)
	{
		int key = Fl::event_key();
		if (key == 32) // SPACE key
		{	
			if (!m_ac.bPanningButtonDown)
			{
				m_ac.bPanningButtonDown = true;
				m_ac.panningStartPoint = mousePosition;

				/*
				const float aspectRatio = (float)m_ac.windowWidth / m_ac.windowHeight;
				float worldX = (2.0f * m_ac.orthoViewDistance * aspectRatio / m_ac.windowWidth) * mousePosition.first + (m_ac.panningCenter.first - m_ac.orthoViewDistance * aspectRatio);
				float worldY = (-2.0f * m_ac.orthoViewDistance / m_ac.windowHeight) * mousePosition.second + (m_ac.panningCenter.second + m_ac.orthoViewDistance);
				printf("Panning start point is (%d, %d) or (%.3f, %.3f)\n", mousePosition.first, mousePosition.second, worldX, worldY);
				*/
			}
		}
	}
	else if (eventType == FL_KEYUP)
	{
		int key = Fl::event_key();
		if (key == 32) // SPACE key
		{
			if (m_ac.bPanningButtonDown)
			{
				m_ac.bPanningButtonDown = false;

				m_ac.panningCenter.first += m_ac.dPanningCenter.first;
				m_ac.panningCenter.second += m_ac.dPanningCenter.second;

				m_ac.dPanningCenter.first = 0;
				m_ac.dPanningCenter.second = 0;
				redraw();
			}
		}
	}
	return Fl_Gl_Window::handle(eventType);
}
