function [M,C] = create_system_manual()

syms   cx   cy   cz   phi   theta   psix real
syms  dcx  dcy  dcz  dphi  dtheta  dpsix real
syms ddcx ddcy ddcz ddphi ddtheta ddpsix real
syms Ixx Iyy Izz Iww real
syms px py pz real
syms fx fy fz real
syms mass s1 s2 s3

q = [ cx cy cz phi theta psix ]';
qd = [ dcx dcy dcz dphi dtheta dpsix ]';
qdd = [ ddcx ddcy ddcz ddphi ddtheta ddpsix ]';

% z-x-z moving frame convention
D1 = [ cos(phi)  -sin(phi)   0;
      sin(phi)     cos(phi)  0;
          0          0       1 ];
C1 = [ 1       0              0;
       0   cos(theta)    -sin(theta);
       0   sin(theta)     cos(theta) ];
B1 = [ cos(psix)   -sin(psix)   0;
       sin(psix)   cos(psix)    0;
          0             0       1 ];

A = B1*C1*D1;

Hcm = diag([mass*(s2^2+s3^2) mass*(s1^2+s3^2)  mass*(s1^2+s2^2)/12]);

H = A*Hcm*A';

omega = [ dphi*sin(theta)*sin(psix)+dtheta*cos(psix) ;
          dphi*sin(theta)*cos(psix)-dtheta*sin(psix) ;
          dphi*cos(theta)+dpsix ];

M = [ eye(3)*mass zeros(3,3);
      zeros(3,3)  H ];
C = [ zeros(3,1);
      cross(omega, H*omega) ];

M = simplify(M)
C = simplify(C)

end