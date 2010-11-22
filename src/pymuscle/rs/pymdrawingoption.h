#ifndef __PYMDRAWINGOPTION_H_
#define __PYMDRAWINGOPTION_H_
enum drawing_option {
  pym_do_unused,
  pym_do_grid, // should start from 1
  pym_do_hud,
  pym_do_joint,
  pym_do_endeffector,
  pym_do_joint_axis,
  pym_do_contact,
  pym_do_contact_force,
  pym_do_root_node,
  pym_do_wireframe,
  pym_do_reference,
  pym_do_fiber,

  pym_do_count,
};
#endif // #ifndef __PYMDRAWINGOPTION_H_
