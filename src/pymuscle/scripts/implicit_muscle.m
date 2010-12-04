%
% Rigid box connected with a muscle fiber
%
% Formulation for implicit integration
%

syms px py pz qx qy qz qw real % Position and orientation quaternion
syms vx vy vz qdx qdy qdz qdw real % First derivatives
syms ax ay az qddx qddy qddz qddw real % Second derivatives
syms Fx Fy Fz real % External resultant force
syms Tx Ty Tz real % External resultant torque
syms m real % Mass of a body
syms Ixx Iyy Izz real % Inertia of principle axis ( I equals diag([Ixx Iyy Izz]) )

% The muscle fiber parameters: serial/parallel spring constants
% and a viscosity constant
syms KSE KPE b xrest real
% Amplitude of muscle tension and its derivative
syms Ten Tend real
% Actuation force acting on the muscle fiber
syms A real
% The muscle fiber world-fixed position in global cooridnates
syms fibWx fibWy fibWz real
% The muscle fiber body-fixed position in body cooridnates
syms fibbx fibby fibbz real

% Vectors defined.
p = [px py pz]'; % Position
q = [qw qx qy qz]'; % Orientation
qbar = [qw -qx -qy -qz]'; % Conjugate quaternion
v = [vx vy vz]'; % Velocity
qd = [qdw qdx qdy qdz]'; % Orientation rate of change
a = [ax ay az]'; % Acceleration
qdd = [qddw qddx qddy qddz]'; % Orientation acceleration
% World-fixed position of the muscle fiber (in global cooridnates)
fibW = [fibWx fibWy fibWz]';
% Body-fixed position of the muscle fiber (in body cooridnates)
fibb = [fibbx fibby fibbz]';

F = [Fx Fy Fz]'; % External resultant force in global coordinates
T = [Tx Ty Tz]'; % External resultant torque in body coordinates

Y = [p' q' Ten' v' qd']'; % State vector
% Time derivative of a state vector (not actually used)
Yd = [v' qd' Tend' a' qdd']';
I = diag([Ixx Iyy Izz]); % 3x3 inertia tensor
Iinv = diag([1/Ixx  1/Iyy  1/Izz]); % Inverse of the inertia tensor

% Angular velocity in body coordinates
omega = 2*quat_mult(qbar, qd);
omega = simplify(omega(2:4));
% fibbW: Position of muscle-body fixed point in global coordinates
fibbWtemp = quat_mult(q, quat_mult([0; fibb], qbar));
fibbW = simplify(p + fibbWtemp(2:4));
% fibbvW: Velocity of muscle-body fixed point in global coordinates
omegax = cross(omega, fibb);
fibbvWtemp = quat_mult(q, quat_mult([0; omegax], qbar));
fibbvW = simplify(v + fibbvWtemp(2:4));

% Fiber length
fiblen = sqrt(dot(fibbW-fibW, fibbW-fibW));
% Unit fiber direction vector from fixed to body in global cooridnates
fibdirW = (fibbW-fibW)/fiblen;
% (x-x*)
deltax = fiblen - xrest;
% Time rate of the muscle fiber length
% NOTE: Attached point's velocity is projected to fiber direction vector.
xd = dot(fibbvW, fibdirW);
% Effects the fixed end of the muscle fiber in global coordinates
TensionToFixedW = Ten*fibdirW;
% Force effects the body itself in global coordinates
TensionToBodyW = -TensionToFixedW;
% Torque effects the body itself in local coordinates
TensionToBody = quat_mult(qbar, quat_mult([0; TensionToBodyW], q));
TensionToBody = simplify(TensionToBody(2:4));
TorqueToBody = cross(fibb, TensionToBody);

% qtemp, qinner: intermediate variables for calculating qdd
qbarqd = simplify(quat_mult(qbar, qd));
omegadot = simplify(Iinv*(T+TorqueToBody - 4*cross(qbarqd(2:4), I*qbarqd(2:4))));

% Function f maps Y to Yd
f = [ v
      qd
      KSE/b*(KPE*deltax + b*xd - (1+KPE/KSE)*Ten + A)
      (F+TensionToBodyW)/m
      quat_mult(qd, qbarqd) + 0.5*quat_mult(q, [0; omegadot]) ];

disp('Starting f, dfdY simplification...')
f = simplify(f);
dfdY = jacobian(f, Y);
disp('Writing to the file...')
outFile = fopen('SymbolicEquations/implicit.txt','w');
fprintf(outFile, '%s', ccode(f));
fprintf(outFile, '%s', ccode(dfdY));
fclose(outFile);
disp('Finished');
