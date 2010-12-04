#ifndef __PYMRSRENDER_H__
#define __PYMRSRENDER_H__

typedef enum {
  PYM_UNKNOWN_GROUND,
  PYM_SQUARE_GROUND,
  PYM_CIRCLE_GROUND,
  PYM_TRANSPARENT_GRID,
} pym_ground_type_t;

typedef struct aaa_pym_render_config_t {
  double cam_r;
  double cam_phi;
  double cam_theta;
  double cam_cen[3];
  int vpx, vpy, vpw, vph; /* viewport x, y, width, height */
} pym_render_config_t;

#ifdef __cplusplus
extern "C" {
#endif
  PYMRS_API void PymRsInitRender();
  PYMRS_API void PymRsDestroyRender();
  PYMRS_API void PymRsRender(pym_rs_t *rs, pym_render_config_t *rc);
  PYMRS_API void PymLookUpLeft(double *look, double *up, double *left,
		     double r, double phi, double theta);
  PYMRS_API void pym_sphere_to_cartesian(double *c, double r,
			       double phi, double theta);
  PYMRS_API void DrawBox_pq(const double *p, const double *q,
    const double *const boxSize, int wf);
  PYMRS_API void pym_up_dir_from_sphere(double *u,
    const double *const cam_car,
    double r, double phi, double theta);
  PYMRS_API void pym_configure_light();
#ifdef __cplusplus
}
#endif

#endif // #ifndef __PYMRSRENDER_H__
