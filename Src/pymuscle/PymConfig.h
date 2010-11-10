#ifndef PYMCONFIG_H_INCLUDED
#define PYMCONFIG_H_INCLUDED

#include "PymJointAnchor.h"

#define MAX_NUM_JOINT_ANCHORS    (100)
#define MAX_NUM_ANCHORED_JOINTS  (50)

struct _pym_config_t {
  pym_rb_t		*body;
  int			 nBody;
  pym_mf_t		*fiber;
  int			 nFiber;
  double		 h;	/* simulation time step */
  double		 mu;	/* ground friction coefficient */
  int			 nSimFrame;
  int			 curFrame;
  char			 trajName[128]; /* e.g. Walk0, Walk1, Nav0, Jump0, ... */
  double		 slant;	/* ground slant angle */
  int			 na;	/* number of joint anchors (points) */
  pym_joint_anchor_t	 pymJa[MAX_NUM_JOINT_ANCHORS];
  int			 nJoint;	/* number of anchored joints */
  pym_anchored_joint_t	 anchoredJoints[MAX_NUM_ANCHORED_JOINTS];
  double		 curBipCom[3];
  /* simulated biped COM position; is part of opt variable */
  double		 bipCom[3];	
  double		 bipRefCom[3];	/* reference biped COM position */
  /* Convex hull output points (thread-unsafe) */
  Point_C		 chInput[1000];
  int			 chInputLen;
  Point_C		 chOutput[1001];
  int			 chOutputLen;

  ///* Convex hull output points (thread-safe) */
  //Point_C		 renChInput[100];
  //int			 renChInputLen;
  //Point_C		 renChOutput[101];
  //int			 renChOutputLen;

  /* Total # of contacts through three frames */
  int			 prevTotContacts;
  int			 curTotContacts;
  int			 nextTotContacts;
  int renderFibers;
};

#endif // PYMCONFIG_H_INCLUDED
