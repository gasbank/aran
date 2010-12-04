#ifndef __BwDebugPrintOptionsWindow_H_
#define __BwDebugPrintOptionsWindow_H_

class BwAppContext;
class BwOpenGlWindow;

class BwDebugPrintOptionsWindow : public Fl_Check_Browser
{
public:
  BwDebugPrintOptionsWindow(int, int, int, int, const char*, BwAppContext& ac);
  ~BwDebugPrintOptionsWindow(void);

  int handle(int);
private:
  void update_debug_msg_stream(int pdmt, int cidx);
  BwAppContext &m_ac;
};

#endif // #ifndef __BwDebugPrintOptionsWindow_H_