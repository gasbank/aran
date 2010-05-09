from OneRbImp import *
from FiberEffectImp import *

p = [1,2,3];
q = [1,0,0,0];
pd = [0,0,1];
qd = [0,0,0,1];
m = 10;
Idiag = 10,20,30
Fr = 10,10,5
Tr = -10,-5,7

yd_Ri, dyd_RidY = OneRbImp(p, q, pd, qd, m, Idiag, Fr, Tr)

print yd_Ri
print dyd_RidY


KSE = 1000.
KPE = 900.
b = 0.1
xrest = 0.2
T = 10.
A = 0.

Td, dTddy_orgins, dTddT = FiberEffectImp_1(p, q, pd, qd, m, Idiag, [0,0,0.5],
                                           [1,2,3.1], q, pd, qd, m, Idiag, [1,1,2],
                                           KSE, KPE, b, xrest, T, A)

print Td
print dTddy_orgins
print dTddT


yd_Q_xxx, dyd_Q_xxxdy_orgins, dyd_Q_xxxdT = FiberEffectImp_2(
    0,
    p, q, pd, qd, m, Idiag, [0,0,0.5],
    [1,2,3.1], q, pd, qd, m, Idiag, [1,1,2],
    KSE, KPE, b, xrest, T, A)

print yd_Q_xxx
print dyd_Q_xxxdy_orgins
print dyd_Q_xxxdT


yd_Q_xxx, dyd_Q_xxxdy_orgins, dyd_Q_xxxdT = FiberEffectImp_2(
    1,
    p, q, pd, qd, m, Idiag, [0,0,0.5],
    [1,2,3.1], q, pd, qd, m, Idiag, [1,1,2],
    KSE, KPE, b, xrest, T, A)

print yd_Q_xxx
print dyd_Q_xxxdy_orgins
print dyd_Q_xxxdT
