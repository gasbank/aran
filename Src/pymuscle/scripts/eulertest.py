from math import cos, sin
import pprint as pp
from numpy import array, dot
phi, theta, psix = [1.758133, -0.060312, -0.148867]

c1,s1 = cos(phi), sin(phi)
c2,s2 = cos(theta), sin(theta)
c3,s3 = cos(psix), sin(psix)

rotX = array([ [1, 0, 0 ],
               [0, c1, -s1],
               [0, s1, c1] ])
rotY = array([ [c2, 0, s2],
               [0,  1,  0 ],
               [-s2,  0, c2 ] ])
rotZ = array([ [c3, -s3, 0 ],
               [s3, c3,  0 ],
               [0,  0,   1 ] ])

print '-------------------------------------'
print dot(rotZ, dot(rotY, rotX))
print '-------------------------------------'

aaa =[ s1*s2*c3+c1*s3,
  c1*s2*c3+s1*s3,
  c2*s3,
  s2,
  c1*s2*s3+s1*c3,
  s1*c2,
  s1*s2,
  c1*s2,
  s2*s3,
  s2*c3,
  c1*c2*s3+s1*c3,
  s1*c2*c3+c1*s3 ]

pp.pprint(aaa)

print
bbb = [ s1*c2,
		s2,
		c1*s2*s3+s1*c3,
		c1*s2*c3+s1*s3,
		c2*s3,
		s1*s2*c3+c1*s3,
		s1*s2,
		c1*s2,
		s2*s3,
		s2*c3,
		c1*c2*s3+s1*c3,
		s1*c2*c3+c1*s3 ]
pp.pprint(bbb)