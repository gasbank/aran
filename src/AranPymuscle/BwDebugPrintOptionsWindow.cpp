#include "BwPch.h"
#include "BwDebugPrintOptionsWindow.h"
#include "BwAppContext.h"
#include "BwOpenGlWindow.h"
#include "pymrscore.h"

int is_devnull(FILE *f) {
  if (f == PymGetDevnull())
    return 1;
  else
    return 0;
}

BwDebugPrintOptionsWindow::BwDebugPrintOptionsWindow( int x, int y, int w, int h, const char* c, BwAppContext& ac )
  : Fl_Check_Browser(x, y, w, h, c)
  , m_ac(ac)
{
#define PYMADD(name,idx) add(name, !is_devnull(m_ac.pymRs->dmstreams[idx]) )
  PYMADD("Traj dev", PDMTE_FBYF_REF_TRAJ_DEVIATION_REPORT);
  PYMADD("Joint dislocation",  PDMTE_FBYF_ANCHORED_JOINT_DISLOCATION_REPORT);
  PYMADD("Ref COM dev", PDMTE_FBYF_REF_COM_DEVIATION_REPORT);
  PYMADD("Active corner pnt", PDMTE_FBYF_ACTIVE_CORNER_POINTS);
  PYMADD("Step result summary", PDMTE_FBYF_STEP_RESULT_SUMMARY);
  PYMADD("Solution vector", PDMTE_FBYF_SOLUTION_VECTOR);
  PYMADD("COM force deviation", PDMTE_FBYF_COM_FORCE_DEVIATION_REPORT);
#undef PYMADD
}

BwDebugPrintOptionsWindow::~BwDebugPrintOptionsWindow(void)
{
}

int BwDebugPrintOptionsWindow::handle( int eventType )
{
  if (eventType == FL_PUSH || eventType == FL_RELEASE || eventType == FL_KEYDOWN || eventType == FL_KEYUP)
  {
    update_debug_msg_stream(PDMTE_FBYF_REF_TRAJ_DEVIATION_REPORT, 1);
    update_debug_msg_stream(PDMTE_FBYF_ANCHORED_JOINT_DISLOCATION_REPORT, 2);
    update_debug_msg_stream(PDMTE_FBYF_REF_COM_DEVIATION_REPORT, 3);
    update_debug_msg_stream(PDMTE_FBYF_ACTIVE_CORNER_POINTS, 4);
    update_debug_msg_stream(PDMTE_FBYF_STEP_RESULT_SUMMARY, 5);
    update_debug_msg_stream(PDMTE_FBYF_SOLUTION_VECTOR, 6);
    update_debug_msg_stream(PDMTE_FBYF_COM_FORCE_DEVIATION_REPORT, 7);
  }
  return Fl_Check_Browser::handle(eventType);
}

void BwDebugPrintOptionsWindow::update_debug_msg_stream( pym_debug_message_type_e pdmt, int cidx )
{
  if (checked(cidx)) {
    m_ac.pymRs->dmstreams[pdmt] = stdout;
  } else {
    m_ac.pymRs->dmstreams[pdmt] = PymGetDevnull();
  }
}
