function [M, Mqdd, Cqd, G, gf, cc] = create_system_expmap(R_sub, dRdv, d2Rdvdv)

syms   cx   cy   cz   v1   v2   v3 real
syms  dcx  dcy  dcz  dv1  dv2  dv3 real
syms ddcx ddcy ddcz ddv1 ddv2 ddv3 real
syms Ixx Iyy Izz Iww real
syms px py pz real
syms fx fy fz real

q = [ cx cy cz v1 v2 v3 ]';
qd = [ dcx dcy dcz dv1 dv2 dv3 ]';
qdd = [ ddcx ddcy ddcz ddv1 ddv2 ddv3 ]';

%v = [cx cy cz]';
%Wi = [ A v; zeros(1,3) 1 ];

dWidq = sym(zeros(4,4,6));
dWidq(1,4,1) = 1; % dWidx
dWidq(2,4,2) = 1; % dWidy
dWidq(3,4,3) = 1; % dWidy
dWidq(:,:,4:6) = dRdv;

Wi = R_sub;

ddWi = 0;
for j=1:6
    tj = 0;
    for k=1:6
        if j <= 3 || k <= 3
            % Do nothing for translational parts
        else
            if (j > k)
                tj = tj + d2Rdvdv(:,:,k-3,j-3)*qd(k);
            else
                tj = tj + d2Rdvdv(:,:,j-3,k-3)*qd(k);
            end
        end
    end
    tj = tj * qd(j);
    tj = tj + dWidq(:,:,j)*qdd(j);
    ddWi = ddWi + tj;
end
ddWi = simplify(ddWi);

Mi = diag([Ixx Iyy Izz Iww]);

X = sym(zeros(6,1));
for j=1:6
    X(j) = simplify(trace(dWidq(:,:,j) * Mi * ddWi'),10);
end

%
% M(q)*qdd + C(q,qd)*qd + G(q)
%
Cqd_G = simplify(subs(X, qdd, zeros(6,1)), 10);
G = simplify(subs(Cqd_G, qd, zeros(6,1)), 10);
Cqd = simplify(Cqd_G - G, 10);
Mqdd = simplify(X - Cqd_G, 10);
M = sym(zeros(6,6));
for j=1:6
    z = zeros(6,1);
    z(j) = 1;
    M(:,j) = simplify(subs(Mqdd, qdd, z));
end


% Externally applied force and its local position of the body
f = [fx fy fz 0]';
p = [px py pz 1]';
gf = sym(zeros(6,1));
for j=1:6
    gf(j) = dot(dWidq(:,:,j) * p, f);
end

% Penetration constraint
cc = sym(zeros(6,1));
for j=1:6
    pt = Wi*p;
    cc(j) = diff(pt(3), q(j));
end
