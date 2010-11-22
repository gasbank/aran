#include "BwPch.h"
#include "BwTopWindow.h"
#include "BwMain.h"
#include "BwOpenGlWindow.h"
#include "BwAppContext.h"
#include "BwPlaybackSlider.h"

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
    if (!m_ac.bSimulate) {
      /* Frame by frame history navigation */
      if (key == FL_Page_Up) {
        m_ac.frames = max(0, m_ac.frames-1);
        m_ac.playbackSlider->value((int)m_ac.frames);
        sprintf(m_ac.frameStr, "%d", m_ac.frames);
        m_ac.playbackSlider->redraw();
        m_ac.frameLabel->redraw_label();

        pym_config_t *pymCfg = &m_ac.pymRs->pymCfg;
        memcpy(pymCfg->body, &m_ac.rb_history[m_ac.frames][0], sizeof(pym_rb_t)*pymCfg->nBody);
        m_ac.glWindow->redraw();

        return 1;
      } else if (key == FL_Page_Down) {
        m_ac.frames = min(m_ac.frames+1, m_ac.playbackSlider->getAvailableFrames());
        m_ac.playbackSlider->value((int)m_ac.frames);
        sprintf(m_ac.frameStr, "%d", m_ac.frames);
        m_ac.playbackSlider->redraw();
        m_ac.frameLabel->redraw_label();

        pym_config_t *pymCfg = &m_ac.pymRs->pymCfg;
        memcpy(pymCfg->body, &m_ac.rb_history[m_ac.frames][0], sizeof(pym_rb_t)*pymCfg->nBody);
        m_ac.glWindow->redraw();

        return 1;
      }
    }
	}
  else if (eventType == FL_ENTER) {
    return 1;
  } else if (eventType == FL_ENTER) {
    take_focus();
    return 1;
  } else if (eventType == FL_FOCUS) {
    return 1;
  } else if (eventType == FL_UNFOCUS) {
    return 1;
  }
	return Fl_Window::handle(eventType);
}

void BwTopWindow::setShapeWindow( BwOpenGlWindow* sw )
{
	m_shapeWindow = sw;
}

void BwTopWindow::setSceneList( Fl_Browser* sl )
{
	m_sceneList = sl;
}

