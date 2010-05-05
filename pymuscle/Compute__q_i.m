function q_i = Compute__q_i(q, m, Iinv, fibb, fibbF)
% 'q_i' computation
% i-th rigid body motion affected only by a muscle fiber
% a muscle fiber is connected to the position fibb (in body coordinates)
% and applies the force fibbF (in body coordinates)
% 
% q_i = [0  0  abar_i  qddbar_i]
%

qbar = [q(1) -q(2) -q(3) -q(4)]'; % Conjugate quaternion

% fibbForceW: Force vector in global coordinates
fibbFWtemp = quat_mult(q, quat_mult([0; fibbF], qbar));
fibbFW = simplify(fibbFWtemp(2:4));
% Torque effects the body in body coordinates
torque = cross(fibb, fibbF);

% qtemp, qinner: intermediate variables for calculating qdd
omegabardot = simplify( Iinv*(0+torque) );

% Function f maps Y to Yd
q_i = [ zeros(3,1)
        zeros(4,1)
        (0+fibbFW)/m
        0.5*quat_mult(q, [0; omegabardot]) ];
    
q_i = simplify(q_i);

end