

syms   cx   cy   cz   v1   v2   v3 real
syms  dcx  dcy  dcz  dv1  dv2  dv3 real
syms ddcx ddcy ddcz ddv1 ddv2 ddv3 real
syms Ixx Iyy Izz real % inertia tensor diagonal elements
syms fx fy fz real  % force
syms tx ty tz real  % torque
syms w x y z real % quaternion
syms m % mass
  
q = [ cx cy cz v1 v2 v3 ]';
qd = [ dcx dcy dcz dv1 dv2 dv3 ]';
qdd = [ ddcx ddcy ddcz ddv1 ddv2 ddv3 ]';
v = [v1 v2 v3]'
dv = [dv1 dv2 dv3]'
tau = [fx fy fz tx ty tz]' % actuating force

Y = [q' qd']';
Icm0 = diag([Ixx, Iyy, Izz]);
theta = sqrt(v1^2 + v2^2 + v3^2);

qbase = sin(theta/2)/theta;
x = qbase*v1;
y = qbase*v2;
z = qbase*v3;
w = cos(theta/2);
qt = [w x y z]';
qtinv = [w -x -y -z]';
qtd = [ -sin(theta/2)/2*theta^(-1/2)*dot(v,dv)     (cos(theta/2)/2*theta^(1/2)*dot(v,dv) - sin(theta/2)*theta^(-1/2)*dot(v,dv))/theta^2*v' + sin(theta/2)/theta*dv'   ]';

A = [ 1-2*y*y-2*z*z   2*x*y+2*w*z     2*x*z-2*w*y    ;
      2*x*y-2*w*z     1-2*x*x-2*z*z   2*y*z+2*w*x    ;
      2*x*z+2*w*y     2*y*z-2*w*x     1-2*x*x-2*y*y    ];
      
A = simplify(A);

Icm = simplify(A*Icm0*A');

M = [ m*eye(3)   zeros(3)
      zeros(3)  Icm ]
Minv = [eye(3)/m  zeros(3)
        zeros(3)  simplify(A*inv(Icm0)*A') ]

gamma = theta*cot(theta/2)
omega = simplify(2*quatmultiply(qtd', qtinv'))'
%omega = subs(omega, v1^2+v2^2+v3^2, 'thetasq');
%omega = subs(omega, '1/thetasq^(1/2)', '1/theta');
%omega = subs(omega, 'sin(thetasq^(1/2)/2)', 'sin(theta/2)');
%omega = subs(omega, 'sin( thetasq^(1/2) )', 'sin(theta1)');
omega = omega(2:4)
C = [ 0 0 0 cross(omega, Icm*omega)']'

%Minv*(tau - C)

jacobian(omega,v)