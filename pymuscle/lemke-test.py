from numpy import *
import lemke

M = array([[-1,0,1],
           [1,-2,0],
           [0,5,0.1]])
q = array([-10.,25.25,-2.5])
z0 = zeros((3))

z, err = lemke.lemke(M, q, z0)

print z
print err