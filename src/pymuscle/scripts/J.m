syms phi theta real

Jv = [ 1  sin(phi)*tan(theta)    cos(phi)*tan(theta);
       0  cos(phi)               -sin(phi);
       0  sin(phi)/cos(theta)    cos(phi)/cos(theta) ];

simplify(inv(Jv))


Jv2 = [ 0     cos(phi)   sin(theta)*sin(phi);
        0    -sin(phi)   sin(theta)*cos(phi);
        1        0       cos(theta)           ];
simplify(inv(Jv2))
