% Optimization-based rigid body simulator
% 2010 Geoyeob Kim
%
% Formulation for implicit integration
% "General muscle-fiber form"
%

syms px py pz qx qy qz qw real % Position and orientation quaternion
syms vx vy vz qdx qdy qdz qdw real % First derivatives
syms ax ay az qddx qddy qddz qddw real % Second derivatives
syms Fx Fy Fz real % External resultant force
syms Tx Ty Tz real % External resultant torque
syms m real % Mass of a body
syms Ixx Iyy Izz real % Inertia of principle axis ( I equals diag([Ixx Iyy Izz]) )

% Vectors defined.
p = [px py pz]'; % Position
q = [qw qx qy qz]'; % Orientation
qbar = [qw -qx -qy -qz]'; % Conjugate quaternion
v = [vx vy vz]'; % Velocity
qd = [qdw qdx qdy qdz]'; % Orientation rate of change
a = [ax ay az]'; % Acceleration
qdd = [qddw qddx qddy qddz]'; % Orientation acceleration
I = diag([Ixx Iyy Izz]); % 3x3 inertia tensor
Iinv = diag([1/Ixx  1/Iyy  1/Izz]); % Inverse of the inertia tensor
F = [Fx Fy Fz]'; % External resultant force in global coordinates
T = [Tx Ty Tz]'; % External resultant torque in body coordinates

%% 'r_i' computation
% i-th rigid body motion only with inertial and external forces
% r_i = [v_i qd_i atil_i qddtil_i]
%

% qbarqd, omegadot: intermediate variables for calculating qdd
qbarqd = simplify(quat_mult(qbar, qd));
omegatildot = simplify(Iinv*(T+0 - 4*cross(qbarqd(2:4), I*qbarqd(2:4))));
r_i = [ v
        qd
        (F+0)/m
        quat_mult(qd, qbarqd) + 0.5*quat_mult(q, [0; omegatildot]) ];
r_i = simplify(r_i)

%% 'q_i' computation

% Body-fixed position of the muscle fiber (in body cooridnates)
syms fibbx fibby fibbz real
syms fibbFx fibbFy fibbFz real

fibb = [fibbx fibby fibbz]';
fibbF = [fibbFx fibbFy fibbFz]';
q_i = Compute__q_i(q, m, Iinv, fibb, fibbF);


%% 'CapQ_i (Q_i)' computation
%
% A fiber 'i' connects the body 'ki' and 'ji' as origin and insertion,
% respectively. Origin and insertion place is fibb_ki and fibb_ji
% in respective body coordinates.
%
syms px_ki py_ki pz_ki qx_ki qy_ki qz_ki qw_ki real
syms vx_ki vy_ki vz_ki qdx_ki qdy_ki qdz_ki qdw_ki real
syms fibbx_ki fibby_ki fibbz_ki real

syms px_ji py_ji pz_ji qx_ji qy_ji qz_ji qw_ji real
syms vx_ji vy_ji vz_ji qdx_ji qdy_ji qdz_ji qdw_ji real
syms fibbx_ji fibby_ji fibbz_ji real

syms A_i xrest KSE KPE b Ten real % Muscle fiber related params

p_ki = [px_ki py_ki pz_ki]'; % Position
q_ki = [qw_ki qx_ki qy_ki qz_ki]'; % Orientation
qbar_ki = [qw_ki -qx_ki -qy_ki -qz_ki]'; % Conjugate quaternion
v_ki = [vx_ki vy_ki vz_ki]'; % Velocity
qd_ki = [qdw_ki qdx_ki qdy_ki qdz_ki]'; % Orientation rate of change
fibb_ki = [ fibbx_ki fibby_ki fibbz_ki ]';

p_ji = [px_ji py_ji pz_ji]'; % Position
q_ji = [qw_ji qx_ji qy_ji qz_ji]'; % Orientation
qbar_ji = [qw_ji -qx_ji -qy_ji -qz_ji]'; % Conjugate quaternion
v_ji = [vx_ji vy_ji vz_ji]'; % Velocity
qd_ji = [qdw_ji qdx_ji qdy_ji qdz_ji]'; % Orientation rate of change
fibb_ji = [ fibbx_ji fibby_ji fibbz_ji ]';

omega_ki = 2*quat_mult(qbar_ki, qd_ki);
omega_ki = simplify(omega_ki(2:4));
fibbW_ki_temp = quat_mult(q_ki, quat_mult([0; fibb_ki], qbar_ki));
fibbW_ki = simplify(p + fibbW_ki_temp(2:4));
fibbdW_ki_temp = quat_mult(q_ki, quat_mult([0; cross(omega_ki, fibb_ki)], qbar_ki));
fibbdW_ki = simplify(v_ki + fibbdW_ki_temp(2:4));

omega_ji = 2*quat_mult(qbar_ji, qd_ji);
omega_ji = simplify(omega_ji(2:4));
fibbW_ji_temp = quat_mult(q_ji, quat_mult([0; fibb_ji], qbar_ji));
fibbW_ji = simplify(p + fibbW_ji_temp(2:4));
fibbdW_ji_temp = quat_mult(q_ji, quat_mult([0; cross(omega_ji, fibb_ji)], qbar_ji));
fibbdW_ji = simplify(v_ji + fibbdW_ji_temp(2:4));

% Tension
% (ORIGIN)------xd POSITIVE DIRECTION-------->(INSERTION)
fibDiffW = simplify(fibbW_ji - fibbW_ki);
fiblen = simplify(sqrt(dot(fibDiffW, fibDiffW)));
fibDirW = fibDiffW / fiblen;
deltax = simplify(fiblen - xrest);
xd = dot(fibDirW, fibbdW_ji - fibbdW_ki);
T_i = simplify(KSE/b*(KPE*deltax + b*xd - (1+KPE/KSE)*Ten + A_i), 10);

% q_i affects origin body (q^k_i)
q_i_org = 0;
% q_i affects insertion body (q^j_i)
q_i_ins = 0;


