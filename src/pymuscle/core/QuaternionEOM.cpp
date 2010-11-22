#include "PymCorePch.h"
#include "LinearR3.h"
#include "QuaternionEOM.h"

typedef AranMath::Quaternion Quat;

// Calculate approximated first derivative of q using q0, q1 and h
Quat calc_quat_first_deri__zz(const Quat &q1, const Quat &q0, const double h) {
  Quat q0inv = q0.inverse();
  Quat multed = q0inv * q1;
  multed.w /= h;
  multed.x /= h;
  multed.y /= h;
  multed.z /= h;
  return multed;
}

// Calculate X where X is qdd = X*q2. Input is q0, q1 and h.
ArnMatrix calc_quat_second_deri_coeff__zz(const Quat &q1, const Quat &q0, const double h) {
  Quat q1inv = q1.inverse();
  ArnMatrix q1inv_mat = q1inv.get_matrix_mult();
  ArnMatrix q0_mat = q0.get_matrix_mult();
  ArnMatrix X = q1inv_mat * q0_mat * q1inv_mat;
  X *= 1.0/h;
  return X;
}

// Calculate approximated first derivative of q using q0, q1 and h
Quat calc_quat_first_deri(const Quat &q1, const Quat &q0, const double h) {
  return (q1 - q0)/h;
}

VectorR3 calc_angular_velocity(const Quat &q1, const Quat &q0, const double h) {
  Quat qd = calc_quat_first_deri(q1, q0, h);
  Quat omega_4 = q1.get_conj() * qd * 2;
  return VectorR3(omega_4.x, omega_4.y, omega_4.z);
}
VectorR3 CrossProduct(const VectorR3 &v1, const VectorR3 &v2) {
  VectorR3 ret(v1);
  ret *= v2; // *= is Cross product operator in VectorR3
  return ret;
}
Quat operator * (const Quat &q, const ArnVec3 &v) {
  return q * Quat(0, v.x, v.y, v.z);
}
Quat operator * (const Quat &q, const VectorR3 &v) {
  return q * Quat(0, v.x, v.y, v.z);
}
Quat qdd(Quat q, Quat qd, VectorR3 tau, Matrix3x3 I, Matrix3x3 Iinv, double t) {
  Quat qbarqd = q.get_conj()*qd;
  VectorR3 qbarqd3(qbarqd.x, qbarqd.y, qbarqd.z);
  Quat qdqbarqd = qd*qbarqd;
  VectorR3 omegadot = Iinv * (tau - CrossProduct(qbarqd3, I*qbarqd3)*4);
  Quat omegadot4(0, omegadot.x, omegadot.y, omegadot.z);

  return qdqbarqd + q * omegadot4 / 2;
}
void quaternion_eom_av(Quat &q1, ArnVec3 &omega1, Quat q0, ArnVec3 omega0, double h) {
  Quat qd0 = q0*omega0/2;
  Quat qdd0 = qdd(q0, qd0, VectorR3::Zero, Matrix3x3::Identity, Matrix3x3::Identity, h);
  q1 = q0 + qd0*h + qdd0*h*h/2;
  Quat qd1 = qd0 + qdd0*h;
  Quat omega1_4 = q1.get_conj()*qd1*2;
  omega1.x = omega1_4.x;
  omega1.y = omega1_4.y;
  omega1.z = omega1_4.z;
}
void quaternion_eom_av_rkn4(Quat &q1, VectorR3 &omega1, Quat q0, VectorR3 omega0, double t, double h) {
  Quat qd0 = q0*omega0/2;
  Quat k1 = qdd(q0, qd0, VectorR3::Zero, Matrix3x3::Identity, Matrix3x3::Identity, t);
  Quat k2 = qdd(q0+qd0*h/2+k1*h*h/8, qd0+k1*h/2, VectorR3::Zero, Matrix3x3::Identity, Matrix3x3::Identity, t);
  Quat k3 = qdd(q0+qd0*h/2+k1*h*h/8, qd0+k2*h/2, VectorR3::Zero, Matrix3x3::Identity, Matrix3x3::Identity, t);
  Quat k4 = qdd(q0+qd0*h+k3*h*h/2, qd0+k3*h, VectorR3::Zero, Matrix3x3::Identity, Matrix3x3::Identity, t);
  q1 = q0+qd0*h+(k1+k2+k3)*h*h/6;
  Quat qd1 = qd0 + (k1+k2*2+k3*2+k4)*h/6;
  Quat omega1_4 = q1.get_conj()*qd1*2;
  omega1.x = omega1_4.x;
  omega1.y = omega1_4.y;
  omega1.z = omega1_4.z;
}

struct State
{
  Quat x; // position
  Quat v; // velocity
};
struct Derivative
{
  Derivative() : dx(Quat(0,0,0,0)), dv(Quat(0,0,0,0)) {}
  Quat dx; // derivative of position: velocity
  Quat dv; // derivative of velocity: acceleration
};
Quat acceleration(const State &state, double t)
{
  return qdd(state.x, state.v, VectorR3::Zero, Matrix3x3::Identity, Matrix3x3::Identity, t);
}
Derivative evaluate(const State &initial, double t, double dt, const Derivative &d)
{
  State state;
  state.x = initial.x + d.dx*dt;
  state.v = initial.v + d.dv*dt;

  Derivative output;
  output.dx = state.v;
  output.dv = acceleration(state, t+dt);
  return output;
}
void integrate(State &state, double t, double dt)
{
  Derivative k1 = evaluate(state, t, 0, Derivative());
  Derivative k2 = evaluate(state, t+dt*0.5, dt*0.5, k1);
  Derivative k3 = evaluate(state, t+dt*0.5, dt*0.5, k2);
  Derivative k4 = evaluate(state, t+dt, dt, k3);

  const Quat dxdt = (k1.dx + (k2.dx + k3.dx)*2.0 + k4.dx) / 6.0;
  const Quat dvdt = (k1.dv + (k2.dv + k3.dv)*2.0 + k4.dv) / 6.0;

  state.x = state.x + dxdt * dt;
  state.v = state.v + dvdt * dt;
}
void quaternion_eom_av_rk4(Quat &q1, VectorR3 &omega1, Quat q0, VectorR3 omega0, double t, double h) {
  Quat qd0 = q0*omega0/2;
  State state;
  state.x = q0;
  state.v = qd0;
  integrate(state, t, h);
  q1 = state.x;
  Quat qd1 = state.v;
  Quat omega1_4 = q1.get_conj()*qd1*2;
  omega1.x = omega1_4.x;
  omega1.y = omega1_4.y;
  omega1.z = omega1_4.z;
}

Quat quaternion_eom(Quat q0, Quat q1, double h) {
  q0.normalize();
  q1.normalize();
  Matrix3x3 I(Matrix3x3::Identity);
  Matrix3x3 Iinv(Matrix3x3::Identity);
  VectorR3 tau(VectorR3::Zero);

  Quat q0inv = q0.inverse();
  Quat qd = calc_quat_first_deri(q1, q0, h);
  Quat qbar = q1.get_conj();
  Quat qbarqd = qbar * qd;
  Quat qdqbarqd = qd*qbarqd;
  VectorR3 qbarqd3(qbarqd.x, qbarqd.y, qbarqd.z);
  VectorR3 Iqbarqd3 = I*qbarqd3;
  VectorR3 rho(qbarqd3);
  rho *= Iqbarqd3; // Cross product: rho = qbarqd3 CROSS Iqbarqd

  VectorR3 Iinv_tau = Iinv*tau;
  Quat Iinv_tau_4(0, Iinv_tau.x, Iinv_tau.y, Iinv_tau.z);
  Quat a = q1*Iinv_tau_4/2.0;

  VectorR3 rho3(rho.x, rho.y, rho.z);
  VectorR3 Iinv_4_rho = Iinv*4*rho;
  Quat Iinv_4_rho_4(0, Iinv_4_rho.x, Iinv_4_rho.y, Iinv_4_rho.z);
  Quat b = q1 * Iinv_4_rho_4/(-2.0);

  Quat q2 = (qdqbarqd + a + b)*h*h + q1*2 - q0;
  q2.normalize();
  return q2;
}

void quaternion_eom_test() {
  Quat q0(1,0,0,0);
  Quat q1(1,0,0,0);
  const double h = 0.01;
  Quat q2 = quaternion_eom(q0, q1, h);
  cout << "q0: " << q0 << endl;
  cout << "q1: " << q1 << endl;
  cout << "q2: " << q2 << endl;
}
