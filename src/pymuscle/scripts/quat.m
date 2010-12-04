%syms a  b  c  d cx cy cz real
%
%v = [cx cy cz]';
%X = [ 1-2*c^2-2*d^2    2*(b*c+a*d)    2*(b*d-a*c);
%      2*(b*c-a*d)    1-2*b^2-2*d^2    2*(c*d+a*b);
%      2*(b*d+a*c)    2*(c*d-a*b)    1-2*b^2-2*c^2 ];
%vp = X*v;
%
%Qd = [ diff(vp,a) diff(vp,b) diff(vp,c) diff(vp,d)];
%Qd(3,:)'


syms px py pz qx qy qz qw real % Position and orientation
syms vx vy vz qdx qdy qdz qdw real % First derivatives
syms ax ay az qddx qddy qddz qddw real % Second derivatives
syms Fx Fy Fz real % External forces
syms Tx Ty Tz real % external torques
syms m real % mass of a body
syms Ixx Iyy Izz real % Inertia of principle axis

p = [px py pz]'; % Position
q = [qw qx qy qz]'; % Orientation
qbar = [qw -qx -qy -qz]'; % Conjugate quaternion
v = [vx vy vz]'; % Velocity
qd = [qdw qdx qdy qdz]'; % Orientation rate of change
a = [ax ay az]'; % Acceleration
qdd = [qddw qddx qddy qddz]'; % Orientation acceleration
F = [Fx Fy Fz]'; % External force
T = [Tx Ty Tz]'; % External torque

Y = [p' q' v' qd']'; % State vector
Yd = [v' qd' a' qdd']'; % Time derivative of a state vector
I = diag([Ixx Iyy Izz]);
Iinv = inv(I);
qtemp = quatmultiply(qbar', qd')';

qinner = Iinv*(T - 4*cross(qtemp(1:3), I*qtemp(1:3)))

% Function f maps Y to Yd
f = [ v
      qd
      F/m
      quatmultiply(qd', qtemp')' + 0.5*quatmultiply(q', [0 qinner'])' ];

f = simplify(f)

dfdY = simplify(jacobian(f, Y))