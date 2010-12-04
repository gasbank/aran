/*
 * PhysicsThreadMain.c: Dedicated physics simulation thread
 *                      for realtime simulator
 * 2010 Geoyeob Kim
 */
#include "PymPch.h"
#include <pthread.h>
#include "include/PrsGraphCapi.h"
#include "PymStruct.h"
#include "ConvexHullCapi.h"
#include "MathUtil.h"
#include "PymConfig.h"
#include "PymCmdLineParser.h"
#include "RigidBody.h"
#include "MuscleFiber.h"
#include "PymuscleConfig.h"
#include "StateDependents.h"
#include "Config.h"
#include "PymCmdLineParser.h"
#include "PymDebugMessageFlags.h"
#include "TrajParser.h"
#include "PhysicsThreadMain.h"
#include "Optimize.h"
#include "DebugPrintDef.h"

#include "pymrscore.h"
#include "pymrsphysics.h"

extern pthread_mutex_t	count_mutex;
extern pthread_mutex_t	main_mutex;
extern pthread_cond_t	count_threshold_cv;
extern pthread_cond_t	physics_thread_finished;

void *PhysicsThreadMain(void *t) {
  pym_rs_t			*rs	= (pym_rs_t *)t;
  pym_physics_thread_context_t	*phyCon	= &rs->phyCon;

  printf("******* %s %d *************\n",
	 rs->pymCfg.body[0].b.name, rs->pymCfg.body[0].b.rotParam);
  PymRsInitPhysics(rs);
  printf("******* %s %d *************\n",
	 rs->pymCfg.body[0].b.name, rs->pymCfg.body[0].b.rotParam);
  PymRsResetPhysics(rs);
  int i = 0;
  while (1) {
    if (phyCon->stop) {
      /* stop flag signaled from the main thread */
      break;
    }
    //pthread_mutex_lock(&main_mutex);
    int ret = PymRsFrameMove(rs, i);
    //pthread_mutex_unlock(&main_mutex);
    //++i;
  }
  PymRsDestroyPhysics(rs);
  printf("Physics thread terminated.\n");
  return 0;
}
