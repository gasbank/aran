function [dRdq, dqdv_sim, dRdv_sim] = expmapderi2()
syms  x  y  z  w real % quaternion
syms v1 v2 v3    real % exponential map parameters and its length
syms theta       real % sqrt(v1^2 + v2^2 + v3^2)
syms cx cy cz    real % translation part

q = [ x  y  z  w]';
R = [ 1-2*y*y-2*z*z   2*x*y+2*w*z     2*x*z-2*w*y     cx;
      2*x*y-2*w*z     1-2*x*x-2*z*z   2*y*z+2*w*x     cy;
      2*x*z+2*w*y     2*y*z-2*w*x     1-2*x*x-2*y*y   cz;
      0               0               0               1   ];

dRdq = sym(zeros(4,4,4));
for i=1:4
    dRdq(:,:,i) = diff(R,q(i));
end

v = [v1 v2 v3]';
theta = sqrt(v1^2 + v2^2 + v3^2);
q_sub = [sin(0.5*theta)/theta*v1   sin(0.5*theta)/theta*v2   sin(0.5*theta)/theta*v3   cos(0.5*theta)]';

dqdv = jacobian(q_sub, v);
syms thetasq theta ct2 st2 real
dqdv_sim = subs(dqdv, v1^2 + v2^2 + v3^2, thetasq);
dqdv_sim = subs(dqdv_sim, 1/thetasq^(1/2), 1/theta);
dqdv_sim = subs(dqdv_sim, thetasq^(1/2), theta);
dqdv_sim = subs(dqdv_sim, 1/thetasq^(3/2), 1/theta^3);
dqdv_sim = subs(dqdv_sim, cos(theta/2), ct2);
dqdv_sim = subs(dqdv_sim, sin(theta/2), st2);

dRdv_sim = sym(zeros(4,4,3));
for i = 1:3
    val = sym(zeros(4,4));
    for j = 1:4
        val = val + dRdq(:,:,j) * dqdv_sim(j,i);
    end
    dRdv_sim(:,:,i) = val;
end

dRdv = sym(zeros(4,4,3));
for i = 1:3
    val = sym(zeros(4,4));
    for j = 1:4
        val = val + dRdq(:,:,j) * dqdv(j,i);
    end
    dRdv(:,:,i) = val;
end

d2Rdv1dv1 = diff(dRdv(:,:,1), v1);
syms thetasq theta ct2 st2 xxxxx real
d2Rdv1dv1 = subs(d2Rdv1dv1, [x y z w], q_sub);
d2Rdv1dv1 = subs(d2Rdv1dv1, v1^2 + v2^2 + v3^2, thetasq);
d2Rdv1dv1 = subs(d2Rdv1dv1, 1/thetasq^(1/2), 1/theta);
d2Rdv1dv1 = subs(d2Rdv1dv1, thetasq^(1/2), theta);
d2Rdv1dv1 = subs(d2Rdv1dv1, 1/thetasq^(3/2), 1/theta^3);
d2Rdv1dv1 = subs(d2Rdv1dv1, 1/thetasq^(5/2), 1/theta^5);
%d2Rdv1dv1 = subs(d2Rdv1dv1, cos(theta/2), ct2);
%d2Rdv1dv1 = subs(d2Rdv1dv1, sin(theta/2), st2);
d2Rdv1dv1 = subs(d2Rdv1dv1, 1/thetasq, 1/theta^2);
d2Rdv1dv1 = subs(d2Rdv1dv1, thetasq, theta^2);
d2Rdv1dv1
findsym(d2Rdv1dv1)
end
