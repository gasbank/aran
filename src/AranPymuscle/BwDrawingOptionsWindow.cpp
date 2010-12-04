#include "BwPch.h"
#include "BwDrawingOptionsWindow.h"
#include "BwAppContext.h"
#include "BwOpenGlWindow.h"

BwDrawingOptionsWindow::BwDrawingOptionsWindow( int x, int y, int w, int h, const char* c, BwAppContext& ac, BwOpenGlWindow& openGlWindow )
: Fl_Check_Browser(x, y, w, h, c)
, m_ac(ac)
, m_openGlWindow(openGlWindow)
{
#define DO_REGISTER(n) add(#n, (int)m_ac.drawing_options[pym_do_ ## n])
  // SHOULD HAVE THE SAME ORDER of pym_do_xxx
	DO_REGISTER(grid);
	DO_REGISTER(hud);
	DO_REGISTER(joint);
	DO_REGISTER(endeffector);
	DO_REGISTER(joint_axis);
	DO_REGISTER(contact);
	DO_REGISTER(contact_force);
	DO_REGISTER(root_node);
  DO_REGISTER(wireframe);
  DO_REGISTER(reference);
  DO_REGISTER(fiber);
#undef DO_REGISTER
}

BwDrawingOptionsWindow::~BwDrawingOptionsWindow(void)
{
}

int BwDrawingOptionsWindow::handle( int eventType )
{
	if (eventType == FL_PUSH || eventType == FL_RELEASE || eventType == FL_KEYDOWN || eventType == FL_KEYUP)
	{
#define DO_REGISTER(n) m_ac.drawing_options[pym_do_ ## n] = checked(pym_do_ ## n) ? true : false
    DO_REGISTER(grid);
    DO_REGISTER(hud);
    DO_REGISTER(joint);
    DO_REGISTER(endeffector);
    DO_REGISTER(joint_axis);
    DO_REGISTER(contact);
    DO_REGISTER(contact_force);
    DO_REGISTER(root_node);
    DO_REGISTER(wireframe);
    DO_REGISTER(reference);
    DO_REGISTER(fiber);
#undef DO_REGISTER
		m_openGlWindow.redraw();
	}
	return Fl_Check_Browser::handle(eventType);
}