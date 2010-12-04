#ifndef __BWOPENGLWINDOW_H__
#define __BWOPENGLWINDOW_H__

class BwAppContext;

class BwOpenGlWindow : public Fl_Gl_Window
{
 public:
		BwOpenGlWindow(int x,int y,int w,int h, const char *l,
			       BwAppContext& ac);
  virtual	~BwOpenGlWindow();
  virtual int	handle(int);
  virtual void	resize(int,int,int,int);
  void setCamCen(double cx, double cy, double cz);
  const ArnVec3 &omega() const { return m_omega; }
 private:
  void		draw();
  void		handle_push();
  void		handle_release();
  void		handle_drag();
  void		handle_move();
  void		handle_mousewheel();
  int		handle_keydown();
  int		handle_keyup();
  BwAppContext& m_ac;
  int		sides;
  int		overlay_sides;
  /* Mouse drag origin point */
  int		m_dragX;
  int		m_dragY;
  /* Mouse panning origin point */
  std::pair<int, int> mousePosition;
  bool		m_drag;
  double	m_cam_r;
  double	m_cam_phi;
  double	m_cam_dphi;
  double	m_cam_theta;
  double	m_cam_dtheta;
  double m_cam_cen[3];
  bool m_screenshot_fbyf;
  ArnVec3 m_omega;
};

struct HitRecord
{
  GLuint numNames;	// Number of names in the name stack for this hit record
  GLuint minDepth;	// Minimum depth value of primitives (range 0 to 2^32-1)
  GLuint maxDepth;	// Maximum depth value of primitives (range 0 to 2^32-1)
  GLuint contents;	// Name stack contents
};

#endif // #ifndef __BWOPENGLWINDOW_H__
