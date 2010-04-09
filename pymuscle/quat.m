syms a  b  c  d cx cy cz real

v = [cx cy cz]';
X = [ 1-2*c^2-2*d^2    2*(b*c+a*d)    2*(b*d-a*c);
      2*(b*c-a*d)    1-2*b^2-2*d^2    2*(c*d+a*b);
      2*(b*d+a*c)    2*(c*d-a*b)    1-2*b^2-2*c^2 ];
vp = X*v;

Qd = [ diff(vp,a) diff(vp,b) diff(vp,c) diff(vp,d)];
Qd(3,:)'
