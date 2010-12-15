#ifndef __PHYSICSTHREADMAIN_H_
#define __PHYSICSTHREADMAIN_H_

enum pym_com_graphs_t {
  PCG_SIM_COMZ = 0, /* Simulated COM z pos */
  PCG_REF_COMZ,     /* Reference COM z pos */
  PCG_COMDEV = 0,   /* COM deviation */
  PCG_ACT_ACT = 0,  /* Actuated muscle fiber's actuation */
  PCG_ACT_TEN,      /* Actuated muscle fiber's tension */
  PCG_LIG_ACT = 0,  /* Ligament fiber's actuation */
  PCG_LIG_TEN       /* Ligament fiber's tension */
};

struct pym_cmdline_options_t;

struct pym_physics_thread_context_t {
  pym_config_t *pymCfg;
  pym_cmdline_options_t *cmdopt;
  pym_traj_t *pymTraj;
  FILE *outputFile;
  FILE **dmstreams;
  double totalPureOptTime;
  int stop;
  double trunkExternalForce[3];

  PRSGRAPH comZGraph;      /* z-axis of COM position graph */
  PRSGRAPH comDevGraph;    /* COM position deviation graph */
  PRSGRAPH actGraph;       /* Actuated muscle tension, actuation graph */
  PRSGRAPH ligGraph;       /* Ligament muscle tension, actuation graph */
  std::vector<PRSGRAPH> exprotGraph; /* body rotation graph */
  pym_rb_statedep_t *sd;
};

void *PhysicsThreadMain(void *t);

#endif
