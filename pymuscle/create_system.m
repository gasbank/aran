function [M, Mqdd, Cqd, G, gf, cc] = create_system()

syms   cx   cy   cz   phi   theta   psix real
syms  dcx  dcy  dcz  dphi  dtheta  dpsix real
syms ddcx ddcy ddcz ddphi ddtheta ddpsix real
syms Ixx Iyy Izz Iww real
syms px py pz real
syms fx fy fz real

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

%simplify(A)

v = [cx cy cz]';
Wi = [ A v; zeros(1,3) 1 ];

dWi = 0;
for j=1:6
    dWi = dWi + diff(Wi, q(j))*qd(j);
end
dWi = simplify(dWi);

ddWi = sym(zeros(4,4));
for j=1:6
    tj = 0;
    for k=1:6
        tj = tj + diff(diff(Wi, q(k)), q(j))*qd(k); 
    end
    tj = tj * qd(j);
    tj = tj + diff(Wi, q(j))*qdd(j);
    ddWi = ddWi + tj;
end
ddWi = simplify(ddWi);

%Mi_com = diag([Ixx Iyy Izz]);
%Mi = A * Mi_com * A';
%Mi = [ Mi zeros(3,1) ; zeros(1,3) 1 ];
%Mi = simplify(Mi);
%Mi
Mi = diag([Ixx Iyy Izz Iww]);

findsym(Mi)
findsym(ddWi)
X = sym(zeros(6,1));
for j=1:6
    j
    %findsym(diff(Wi, q(j)))
    
    mmm = simplify(diff(Wi, q(j)) * Mi * ddWi', 10);
    
    findsym(mmm)
    findsym(simplify(trace( mmm )))
    X(j) = simplify(trace( mmm ),10);
    %findsym(X(j))
end
findsym(X)
%
% M(q)*qdd + C(q,qd)*qd + G(q)
%
Cqd_G = simplify(subs(X, qdd, zeros(6,1)), 10)
G = simplify(subs(Cqd_G, qd, zeros(6,1)), 10)
Cqd = simplify(Cqd_G - G, 10)
Mqdd = simplify(X - Cqd_G, 10)
M = sym(zeros(6,6));
for j=1:6
    j
    z = sym(zeros(6,1));
    z(j) = sym(1);
    M(:,j) = simplify(subs(Mqdd, qdd, z));
end

M
Cqd
G

% Externally applied force and its local position of the body
f = [fx fy fz 0]';
p = [px py pz 1]';
gf = [];
for j=1:6
    gf = [gf dot(diff(Wi, q(j)) * p, f)];
end
gf = gf';
gf

% Penetration constraint
cc = [];
for j=1:6
    pt = Wi*p;
    cc = [cc diff(pt(3), q(j))];
end
cc

