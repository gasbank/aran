function expmapderi3()
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
dRdq

v = [v1 v2 v3]';
theta = sqrt(v1^2 + v2^2 + v3^2);
qs = [sin(0.5*theta)/theta*v1   sin(0.5*theta)/theta*v2   sin(0.5*theta)/theta*v3   cos(0.5*theta)]';

dqdv = simplify(jacobian(qs, v));

dRsdq = subs(dRdq, q, qs);
diff(dRsdq, v1)
end
