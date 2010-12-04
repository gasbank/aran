#include "BwPch.h"
#include "BwRbTrackingOptionsWindow.h"
#include "BwAppContext.h"
#include "BwOpenGlWindow.h"
#include "pymrscore.h"

BwRbTrackingOptionsWindow::BwRbTrackingOptionsWindow( int x, int y, int w, int h, const char* c, BwAppContext& ac )
  : Fl_Check_Browser(x, y, w, h, c)
  , m_ac(ac)
{
}

BwRbTrackingOptionsWindow::~BwRbTrackingOptionsWindow(void)
{
}



int BwRbTrackingOptionsWindow::handle( int eventType )
{
  if (eventType == FL_PUSH || eventType == FL_RELEASE || eventType == FL_KEYDOWN || eventType == FL_KEYUP)
  {
  }
  return Fl_Check_Browser::handle(eventType);
}

void BwRbTrackingOptionsWindow::update_debug_msg_stream( pym_debug_message_type_e pdmt, int cidx )
{
  if (checked(cidx)) {
    m_ac.pymRs->dmstreams[pdmt] = stdout;
  } else {
    m_ac.pymRs->dmstreams[pdmt] = PymGetDevnull();
  }
}

void BwRbTrackingOptionsWindow::init_items()
{
  static int run_once = 0;
  if (run_once)
    return;
  run_once = 1;
  if (m_ac.pymRs) {
    pym_config_t *pymCfg = &m_ac.pymRs->pymCfg;
    const int nb = pymCfg->nBody;
    for (int i = 0; i < nb; ++i) {
      add(pymCfg->body[i].b.name, 1);
    }
  }
}
