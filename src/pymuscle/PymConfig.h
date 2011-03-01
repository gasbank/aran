#ifndef PYMCONFIG_H_INCLUDED
#define PYMCONFIG_H_INCLUDED

#include "PymJointAnchor.h"

const static int MAX_RIGID_BODIES = 100;
const static int MAX_NUM_JOINT_ANCHORS = 100;
const static int MAX_NUM_ANCHORED_JOINTS = 50;

enum optimization_cost_terms {
  oct_normal_force,
  oct_normal_force_nonneg_comp, // normal force nonnegativity compensation term
  oct_contact_point_zpos,
  oct_contact_point_movement,
  oct_contact_point_zpos_epsilon,
  oct_rb_reference_deviation,
  oct_rb_previous_deviation,
  oct_biped_com_deviation,
  oct_torque_around_com,
  oct_ligament_actuation,
  oct_actuated_muscle_actuation,
  oct_joint_dislocation,
  oct_uniform_tension_cost,
  oct_uniform_actuation_cost,
  oct_com_force_deviation_cost,
  oct_biped_eps_fjoint,
  oct_rb_eps_freecom,
  oct_biped_eps_efjoint_xy,
  oct_biped_eps_efjoint_z,
  oct_cornerpoint_eps_z,
  oct_biped_eps_sp_deviation,

  oct_count
};

enum pym_rot_param_t;

union pym_rb_t;
union pym_mf_t;

struct pym_config_t {
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
  double		 bipCom0[3];
  double		 bipCom[3];
  double		 bipRefCom[3];	/* reference biped COM position */
  /* Convex hull output points (thread-unsafe) */
  bool use_relaxed_ch_as_constraint;
  // Convex hull of (physical) contact points
  Point_C	 chInput[1000], chOutput[1001];
  int			 chInputLen, chOutputLen;
  // Modified convex hull of virtual contact points
  Point_C	 chVInput[1000], chVOutput[1001];
  int			 chVInputLen, chVOutputLen;
  double chi_d_norm_sum;
  ///* Convex hull output points (thread-safe) */
  //Point_C		 renChInput[100];
  //int			 renChInputLen;
  //Point_C		 renChOutput[101];
  //int			 renChOutputLen;

  /* Total # of contacts through three frames */
  int			 prevTotContacts;
  int			 curTotContacts;
  int renderFibers;
  /* Optimization parameters */
  double opt_cost_coeffs[oct_count];
  double joint_dislocation_threshold;
  bool joint_dislocation_enabled;
  bool real_muscle;
  bool zero_cost_func;
  bool temp_ref_mode;
  bool render_hud;
  bool freeze_pose;
  bool support_polygon_constraint;
  /* biped-wide rotation parameterization */
  pym_rot_param_t rotparam;
  int nrp; /* dimension of rotation parameterization (i.e. quaternion=4, exprot=3) */

  int joint_constraints; // Fiber attached between joint anchor points if this value is not 0.
};

#endif // PYMCONFIG_H_INCLUDED