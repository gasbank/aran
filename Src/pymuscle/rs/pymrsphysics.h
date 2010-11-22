#ifndef __PYMRSPHYSICS_H__
#define __PYMRSPHYSICS_H__

#ifdef __cplusplus
extern "C" {
#endif
  PYMRS_API int PymRsInitPhysics(pym_rs_t *rs);
  PYMRS_API int PymRsDestroyPhysics(pym_rs_t *rs);
  PYMRS_API int PymRsResetPhysics(pym_rs_t *rs);
  PYMRS_API int PymRsFrameMove(pym_rs_t *rs, int fidx, char *result_msg);
#ifdef __cplusplus
}
#endif

#endif // #ifndef __PYMRSPHYSICS_H__
