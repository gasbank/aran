#ifndef __BWRBTRACKINGOPTIONSWINDOW_H_
#define __BWRBTRACKINGOPTIONSWINDOW_H_

class BwAppContext;
class BwOpenGlWindow;

class BwRbTrackingOptionsWindow : public Fl_Check_Browser
{
public:
  BwRbTrackingOptionsWindow(int, int, int, int, const char*, BwAppContext& ac);
  ~BwRbTrackingOptionsWindow(void);

  int handle(int);
  void init_items();
private:
  void update_debug_msg_stream(pym_debug_message_type_e pdmt, int cidx);
  BwAppContext &m_ac;
};

#endif // #ifndef __BWRBTRACKINGOPTIONSWINDOW_H_