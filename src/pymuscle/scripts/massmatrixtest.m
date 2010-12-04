syms   cx   cy   cz   phi   theta   psix real
syms  dcx  dcy  dcz  dphi  dtheta  dpsix real
syms ddcx ddcy ddcz ddphi ddtheta ddpsix real
syms Ixx Iyy Izz Iww real
syms px py pz real
syms fx fy fz real

qdd = [ ddphi ddtheta ddpsix ]';

D1 = [ cos(phi)  -sin(phi)  0;
      sin(phi) cos(phi) 0;
      0 0 1 ];
C1 = [ 1 0 0;
      0 cos(theta) -sin(theta);
      0 sin(theta) cos(theta) ];
B1 = [ cos(psix) -sin(psix) 0;
     sin(psix) cos(psix) 0;
     0 0 1 ];

A = B1*C1*D1;


Mi3 = diag([Ixx Iyy Izz]);
X = simplify(A*Mi3*A');


Cqd_G = subs(X, qdd, zeros(3,1));
Mqdd = X-Cqd_G;
M = sym(zeros(3,3))
for j=1:3
    j
    z = sym(zeros(3,1));
    z(j) = sym(1);
    M(:,j) = simplify(subs(Mqdd, qdd, z));
end
