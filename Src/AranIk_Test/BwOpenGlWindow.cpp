#include "BwPch.h"
#include "BwOpenGlWindow.h"
#include "BwAppContext.h"
#include "BwMain.h"

BwOpenGlWindow::BwOpenGlWindow(int x, int y, int w, int h, const char *l, BwAppContext& ac)
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
			const float d1 = -(2.0f * m_ac.orthoViewDistance * aspectRatio / m_ac.windowWidth) * dx;
			const float d2 = -(-2.0f * m_ac.orthoViewDistance / m_ac.windowHeight) * dy;

			if (m_ac.viewMode == VM_TOP)
			{
				m_ac.dPanningCenter[0] = d1;
				m_ac.dPanningCenter[1] = d2;
			}
			else if (m_ac.viewMode == VM_RIGHT)
			{
				m_ac.dPanningCenter[1] = d1;
				m_ac.dPanningCenter[2] = d2;
			}
			else if (m_ac.viewMode == VM_BACK)
			{
				m_ac.dPanningCenter[0] = d1;
				m_ac.dPanningCenter[2] = d2;
			}
			

			//printf("%f   %f\n", m_ac.dPanningCenter.first, m_ac.dPanningCenter.second);

			redraw();
		}
	}
	else if (eventType == FL_MOUSEWHEEL)
	{
		if (m_ac.viewMode == VM_TOP || m_ac.viewMode == VM_RIGHT || m_ac.viewMode == VM_BACK)
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

		if (key < 256)
			m_ac.bHoldingKeys[key] = true;

		if (key == 32) // SPACE key
		{
			if (m_ac.bPanningButtonDown)
			{
				m_ac.bPanningButtonDown = false;

				m_ac.panningCenter[0] += m_ac.dPanningCenter[0];
				m_ac.panningCenter[1] += m_ac.dPanningCenter[1];
				m_ac.panningCenter[2] += m_ac.dPanningCenter[2];

				m_ac.dPanningCenter[0] = 0;
				m_ac.dPanningCenter[1] = 0;
				m_ac.dPanningCenter[2] = 0;
				redraw();
			}
		}
		else if (key == FL_KP + '7')
		{
			m_ac.viewMode = VM_TOP;
			printf("  View mode set to top.\n");
			redraw();
		}
		else if (key == FL_KP + '3')
		{
			m_ac.viewMode = VM_RIGHT;
			printf("  View mode set to left.\n");
			redraw();
		}
		else if (key == FL_KP + '1')
		{
			m_ac.viewMode = VM_BACK;
			printf("  View mode set to front.\n");
			redraw();
		}
		else if (key == FL_KP + '4')
		{
			m_ac.viewMode = VM_CAMERA;
			printf("  View mode set to camera.\n");
			redraw();
		}
	}
	return Fl_Gl_Window::handle(eventType);
}
