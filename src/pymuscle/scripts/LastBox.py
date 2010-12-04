#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

A standalone box simulator
"""

from numpy import *
from math import *
import matplotlib.pyplot as pit

sx, sy, sz = 0.5, 0.3, 0.2
m = 1.
Icm0 = diag([m*(sy*sy + sz*sz)/12.,
			 m*(sx*sx + sz*sz)/12.,
			 m*(sx*sx + sy*sy)/12.])
A = identity(3)
omega = zeros(3)
Icm = dot(dot(A, Icm0), A.T)

M = vstack([ hstack([ Icm,          zeros((3,3))  ]),
             hstack([ zeros((3,3)), m*identity(3) ])
             ])
C = hstack([ cross(omega, dot(Icm, omega)), zeros(3) ])

F = array([0, 0, 0, 0, 0, -9.81*m])

print Icm
print M
print C

a = dot(linalg.inv(M), F - C)

print a
