#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Inverse Dynamics Torque Calculator
"""

from numpy import *
from math import *
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
from lemke import *
from SymbolicMC import *
from SymbolicForce import *
from SymbolicPenetration import *
from SymbolicTensor import *
from glprim import *
import sys

q_file = open('/media/vm/devel/aran/pymuscle/traj_q.txt', 'r')
qd_file = open('/media/vm/devel/aran/pymuscle/traj_qd.txt', 'r')
qdd_file = open('/media/vm/devel/aran/pymuscle/traj_qdd.txt', 'r')

metastr = q_file.readline()
qd_file.readline()
qdd_file.readline()

metastr.strip()
noFrame, nb = map(int, metastr.split())

bodyCfg = [ [ 0.493, 0.987, 0.179, 30 ],     # Hips (root)
            [ 0.054, 0.274, 0.054, 3 ],     # LHipJoint
            [ 0.054, 0.274, 0.054, 3 ],     # RHipJoint
            [ 0.145, 0.450, 0.145, 3 ],     # LeftHip
            [ 0.145, 0.450, 0.145, 3 ],     # RightHip
            [ 0.145, 0.450, 0.145, 3 ],     # LeftKnee
            [ 0.145, 0.450, 0.145, 3 ],     # RightKnee
            [ 0.184, 0.210, 0.090, 3 ],     # LeftAnkle
            [ 0.184, 0.210, 0.090, 3 ],     # RightAnkle
            [ 0.184, 0.105, 0.090, 3 ],     # LeftToe
            [ 0.184, 0.105, 0.090, 3 ] ]    # RightToe
I = []
for bc in bodyCfg:
	sx, sy, sz, mass = bc
	rho = mass / (sx*sy*sz)
	I.append(SymbolicTensor(sx, sy, sz, rho))

torque_file = open('/media/vm/devel/aran/pymuscle/torque.txt', 'w')
torque_file.write(metastr)


for i in range(noFrame-2):
	for j in range(nb):
		q_ij = map(float, q_file.readline().strip().split())
		qd_ij = map(float, qd_file.readline().strip().split())
		qdd_ij = map(float, qdd_file.readline().strip().split())
		
		M_ij = SymbolicM(q_ij, I[j])
		Cqd_ij = SymbolicCqd(q_ij, qd_ij, I[j])
		# Calculate generalized torques
		t_ij = dot(M_ij, qdd_ij) + Cqd_ij
		tstr = ('%15e'*6 % tuple(t_ij)) + '\n'
		torque_file.write(tstr)
		
		
		Minv_ij = SymbolicMinv(q_ij, I[j])
		expected_qdd_ij = dot(Minv_ij, t_ij - Cqd_ij)
		print linalg.norm(qdd_ij - expected_qdd_ij)
		
		
torque_file.close()
q_file.close()
qd_file.close()
qdd_file.close()