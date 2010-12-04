syms mx my mz rho sx sy sz real

x = [mx my mz 1]'
%rho = 0.1
%sx = 0.15
%sy = 0.3
%sz = 0.11

Mi = rho*int(int(int(x*x', mx, -sx/2, sx/2), my, -sy/2, sy/2), mz, -sz/2, sz/2)
Ixx = Mi(1,1);
Iyy = Mi(2,2);
Izz = Mi(3,3);
Iww = Mi(4,4);
ccode(Ixx)
ccode(Iyy)
ccode(Izz)
ccode(Iww)
