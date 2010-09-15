#ifndef __PHYSICSTHREADMAIN_H_
#define __PHYSICSTHREADMAIN_H_

typedef enum _pym_com_graphs_t {
    PCG_SIM_COMZ = 0,
    PCG_REF_COMZ,
    PCG_COMDEV = 0,
} pym_com_graphs_t;

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
    /* renderer-accessable memory area */
    pym_rb_t *renBody;
    pym_mf_t *renFiber;
    double   bipCom[3];

    PRSGRAPH comZGraph;      /* z-axis of COM position graph */
    PRSGRAPH comDevGraph;    /* COM position deviation graph */
} pym_physics_thread_context_t;

void *PhysicsThreadMain(void *t);

#endif
