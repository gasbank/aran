#ifndef __PHYSICSTHREADMAIN_H_
#define __PHYSICSTHREADMAIN_H_

typedef struct _pym_physics_thread_context_t {
    pym_config_t *pymCfg;
    int nBlenderBody;
    pym_cmdline_options_t *cmdopt;
    int *corresMapIndex;
    FILE *outputFile;
    FILE **dmstreams;
    double totalPureOptTime;
    double *trajData;
    int stop;
    double trunkExternalForce[3];
} pym_physics_thread_context_t;

void *PhysicsThreadMain(void *t);

#endif
