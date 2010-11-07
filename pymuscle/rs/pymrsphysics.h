#ifndef __PYMRSPHYSICS_H__
#define __PYMRSPHYSICS_H__

#ifdef __cplusplus
extern "C" {
#endif
  int PymRsInitPhysics(pym_rs_t *rs);
  int PymRsDestroyPhysics(pym_rs_t *rs);
  int PymRsResetPhysics(pym_rs_t *rs);
  int PymRsFrameMove(pym_rs_t *rs, int i);
#ifdef __cplusplus
}
#endif

#endif // #ifndef __PYMRSPHYSICS_H__
