#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Symbolic form of a mass matrix and a Coriolis matrix for a rigid body
"""
from numpy import *
from math import pow, sin, cos

def SymbolicM(q, I):
	M = zeros((6,6))

	Ixx, Iyy, Izz, Iww = I
	cx, cy, cz, phi, theta, psix = q

	M[0, 0] = Iww
	M[0, 1] = 0.0
	M[0, 2] = 0.0
	M[0, 3] = 0.0
	M[0, 4] = 0.0
	M[0, 5] = 0.0
	M[1, 0] = 0.0
	M[1, 1] = Iww
	M[1, 2] = 0.0
	M[1, 3] = 0.0
	M[1, 4] = 0.0
	M[1, 5] = 0.0
	M[2, 0] = 0.0
	M[2, 1] = 0.0
	M[2, 2] = Iww
	M[2, 3] = 0.0
	M[2, 4] = 0.0
	M[2, 5] = 0.0
	M[3, 0] = 0.0
	M[3, 1] = 0.0
	M[3, 2] = 0.0
	M[3, 3] = Ixx+Iyy
	M[3, 4] = 0.0
	M[3, 5] = cos(theta)*(Ixx+Iyy)
	M[4, 0] = 0.0
	M[4, 1] = 0.0
	M[4, 2] = 0.0
	M[4, 3] = 0.0
	M[4, 4] = Ixx+Izz-Ixx*pow(cos(phi),2.0)+Iyy*pow(cos(phi),2.0)
	M[4, 5] = -cos(phi)*sin(phi)*sin(theta)*(Ixx-Iyy)
	M[5, 0] = 0.0
	M[5, 1] = 0.0
	M[5, 2] = 0.0
	M[5, 3] = cos(theta)*(Ixx+Iyy)
	M[5, 4] = -cos(phi)*sin(phi)*sin(theta)*(Ixx-Iyy)
	M[5, 5] = Ixx+Iyy-Iyy*pow(sin(theta),2.0)+Izz*pow(sin(theta),2.0)-Ixx*pow(sin(phi),2.0)*pow(sin(theta),2.0)+Iyy*pow(sin(phi),2.0)*pow(sin(theta),2.0)

	return M


def SymbolicMinv(q, I):
	Minv = zeros((6,6))

	Ixx, Iyy, Izz, Iww = I
	cx, cy, cz, phi, theta, psix = q

	Minv[0, 0] = 1/Iww
	Minv[0, 1] = 0.0
	Minv[0, 2] = 0.0
	Minv[0, 3] = 0.0
	Minv[0, 4] = 0.0
	Minv[0, 5] = 0.0
	Minv[1, 0] = 0.0
	Minv[1, 1] = 1/Iww
	Minv[1, 2] = 0.0
	Minv[1, 3] = 0.0
	Minv[1, 4] = 0.0
	Minv[1, 5] = 0.0
	Minv[2, 0] = 0.0
	Minv[2, 1] = 0.0
	Minv[2, 2] = 1/Iww
	Minv[2, 3] = 0.0
	Minv[2, 4] = 0.0
	Minv[2, 5] = 0.0
	Minv[3, 0] = 0.0
	Minv[3, 1] = 0.0
	Minv[3, 2] = 0.0
	Minv[3, 3] = -((Ixx*Ixx)*(1.0/2.0)+(Iyy*Iyy)*(1.0/2.0)-Izz*Izz)/((Ixx+Iyy)*(Ixx+Izz)*(Iyy+Izz))+(1/pow(sin(theta),2.0)*((Ixx*Ixx)*cos(phi*2.0)*(-1.0/2.0)+(Iyy*Iyy)*cos(phi*2.0)*(1.0/2.0)-(Ixx*Ixx)*cos(phi*2.0-theta*2.0)*(1.0/4.0)-(Ixx*Ixx)*cos(phi*2.0+theta*2.0)*(1.0/4.0)+(Iyy*Iyy)*cos(phi*2.0-theta*2.0)*(1.0/4.0)+(Iyy*Iyy)*cos(phi*2.0+theta*2.0)*(1.0/4.0)+Ixx*Ixx+Iyy*Iyy+Ixx*Iyy*2.0+Ixx*Izz*2.0+Iyy*Izz*2.0)*(1.0/2.0))/((Ixx+Iyy)*(Ixx+Izz)*(Iyy+Izz))
	Minv[3, 4] = (1/pow(sin(theta),2.0)*(Ixx-Iyy)*(pow(sin(phi+theta),2.0)*2.0-pow(sin(phi-theta),2.0)*2.0)*(-1.0/8.0))/((Ixx+Izz)*(Iyy+Izz))
	Minv[3, 5] = (Ixx*cos(phi*2.0-theta)*(-1.0/4.0)+Iyy*cos(phi*2.0-theta)*(1.0/4.0)+cos(theta)*(Ixx*(1.0/2.0)+Iyy*(1.0/2.0)+Izz)-Ixx*cos(phi*2.0+theta)*(1.0/4.0)+Iyy*cos(phi*2.0+theta)*(1.0/4.0))/((pow(cos(theta),2.0)-1.0)*(Ixx+Izz)*(Iyy+Izz))
	Minv[4, 0] = 0.0
	Minv[4, 1] = 0.0
	Minv[4, 2] = 0.0
	Minv[4, 3] = (1/pow(sin(theta),2.0)*(Ixx-Iyy)*(pow(sin(phi+theta),2.0)*2.0-pow(sin(phi-theta),2.0)*2.0)*(-1.0/8.0))/((Ixx+Izz)*(Iyy+Izz))
	Minv[4, 4] = (Ixx*(1.0/2.0)+Iyy*(1.0/2.0)+Izz)/((Ixx+Izz)*(Iyy+Izz))-(1/pow(sin(theta),2.0)*(Ixx*cos(phi*2.0-theta*2.0)*(1.0/4.0)+Ixx*cos(phi*2.0+theta*2.0)*(1.0/4.0)-Iyy*cos(phi*2.0-theta*2.0)*(1.0/4.0)-Iyy*cos(phi*2.0+theta*2.0)*(1.0/4.0)-Ixx*cos(phi*2.0)*(1.0/2.0)+Iyy*cos(phi*2.0)*(1.0/2.0))*(1.0/2.0))/((Ixx+Izz)*(Iyy+Izz))
	Minv[4, 5] = (sin(phi*2.0)*(Ixx-Iyy)*(1.0/2.0))/(sin(theta)*(Ixx+Izz)*(Iyy+Izz))
	Minv[5, 0] = 0.0
	Minv[5, 1] = 0.0
	Minv[5, 2] = 0.0
	Minv[5, 3] = (Ixx*cos(phi*2.0-theta)*(-1.0/4.0)+Iyy*cos(phi*2.0-theta)*(1.0/4.0)+cos(theta)*(Ixx*(1.0/2.0)+Iyy*(1.0/2.0)+Izz)-Ixx*cos(phi*2.0+theta)*(1.0/4.0)+Iyy*cos(phi*2.0+theta)*(1.0/4.0))/((pow(cos(theta),2.0)-1.0)*(Ixx+Izz)*(Iyy+Izz))
	Minv[5, 4] = (sin(phi*2.0)*(Ixx-Iyy)*(1.0/2.0))/(sin(theta)*(Ixx+Izz)*(Iyy+Izz))
	Minv[5, 5] = (1/pow(sin(theta),2.0)*(Ixx+Iyy+Izz*2.0-Ixx*cos(phi*2.0)+Iyy*cos(phi*2.0))*(1.0/2.0))/((Ixx+Izz)*(Iyy+Izz))

	return Minv

def SymbolicCqd(q, qd, I):
	Cqd = zeros((6))

	Ixx, Iyy, Izz, Iww = I
	cx, cy, cz, phi, theta, psix = q
	dcx, dcy, dcz, dphi, dtheta, dpsi = qd

	Cqd[0] = Iww*dcx*(dcx+dcy+dcz+dphi+dpsi+dtheta)
	Cqd[1] = Iww*dcy*(dcx+dcy+dcz+dphi+dpsi+dtheta)
	Cqd[2] = Iww*dcz*(dcx+dcy+dcz+dphi+dpsi+dtheta)
	Cqd[3] = (dphi+dpsi*cos(theta))*(Ixx+Iyy)*(dcx+dcy+dcz+dphi+dpsi+dtheta)
	Cqd[4] = Ixx*(dtheta*dtheta)*(1.0/2.0)+Iyy*(dtheta*dtheta)*(1.0/2.0)+Izz*(dtheta*dtheta)+Ixx*dcx*dtheta*(1.0/2.0)+Ixx*dcy*dtheta*(1.0/2.0)+Ixx*dcz*dtheta*(1.0/2.0)+Iyy*dcx*dtheta*(1.0/2.0)+Iyy*dcy*dtheta*(1.0/2.0)+Iyy*dcz*dtheta*(1.0/2.0)+Izz*dcx*dtheta+Izz*dcy*dtheta+Izz*dcz*dtheta+Ixx*dphi*dtheta*(1.0/2.0)+Ixx*dpsi*dtheta*(1.0/2.0)+Iyy*dphi*dtheta*(1.0/2.0)+Iyy*dpsi*dtheta*(1.0/2.0)+Izz*dphi*dtheta+Izz*dpsi*dtheta+Ixx*(dpsi*dpsi)*cos(phi*2.0+theta)*(1.0/4.0)-Iyy*(dpsi*dpsi)*cos(phi*2.0+theta)*(1.0/4.0)-Ixx*(dtheta*dtheta)*cos(phi*2.0)*(1.0/2.0)+Iyy*(dtheta*dtheta)*cos(phi*2.0)*(1.0/2.0)-Ixx*(dpsi*dpsi)*cos(phi*2.0-theta)*(1.0/4.0)+Iyy*(dpsi*dpsi)*cos(phi*2.0-theta)*(1.0/4.0)+Ixx*dcx*dpsi*cos(phi*2.0+theta)*(1.0/4.0)+Ixx*dcy*dpsi*cos(phi*2.0+theta)*(1.0/4.0)+Ixx*dcz*dpsi*cos(phi*2.0+theta)*(1.0/4.0)-Iyy*dcx*dpsi*cos(phi*2.0+theta)*(1.0/4.0)-Iyy*dcy*dpsi*cos(phi*2.0+theta)*(1.0/4.0)-Iyy*dcz*dpsi*cos(phi*2.0+theta)*(1.0/4.0)+Ixx*dphi*dpsi*cos(phi*2.0+theta)*(1.0/4.0)-Iyy*dphi*dpsi*cos(phi*2.0+theta)*(1.0/4.0)+Ixx*dpsi*dtheta*cos(phi*2.0+theta)*(1.0/4.0)-Iyy*dpsi*dtheta*cos(phi*2.0+theta)*(1.0/4.0)-Ixx*dcx*dtheta*cos(phi*2.0)*(1.0/2.0)-Ixx*dcy*dtheta*cos(phi*2.0)*(1.0/2.0)-Ixx*dcz*dtheta*cos(phi*2.0)*(1.0/2.0)+Iyy*dcx*dtheta*cos(phi*2.0)*(1.0/2.0)+Iyy*dcy*dtheta*cos(phi*2.0)*(1.0/2.0)+Iyy*dcz*dtheta*cos(phi*2.0)*(1.0/2.0)-Ixx*dphi*dtheta*cos(phi*2.0)*(1.0/2.0)-Ixx*dpsi*dtheta*cos(phi*2.0)*(1.0/2.0)+Iyy*dphi*dtheta*cos(phi*2.0)*(1.0/2.0)+Iyy*dpsi*dtheta*cos(phi*2.0)*(1.0/2.0)-Ixx*dcx*dpsi*cos(phi*2.0-theta)*(1.0/4.0)-Ixx*dcy*dpsi*cos(phi*2.0-theta)*(1.0/4.0)-Ixx*dcz*dpsi*cos(phi*2.0-theta)*(1.0/4.0)+Iyy*dcx*dpsi*cos(phi*2.0-theta)*(1.0/4.0)+Iyy*dcy*dpsi*cos(phi*2.0-theta)*(1.0/4.0)+Iyy*dcz*dpsi*cos(phi*2.0-theta)*(1.0/4.0)-Ixx*dphi*dpsi*cos(phi*2.0-theta)*(1.0/4.0)+Iyy*dphi*dpsi*cos(phi*2.0-theta)*(1.0/4.0)-Ixx*dpsi*dtheta*cos(phi*2.0-theta)*(1.0/4.0)+Iyy*dpsi*dtheta*cos(phi*2.0-theta)*(1.0/4.0)
	Cqd[5] = Ixx*(dpsi*dpsi)*(3.0/4.0)+Iyy*(dpsi*dpsi)*(3.0/4.0)+Izz*(dpsi*dpsi)*(1.0/2.0)+Ixx*(dphi*dphi)*cos(theta)+Iyy*(dphi*dphi)*cos(theta)+Ixx*dcx*dpsi*(3.0/4.0)+Ixx*dcy*dpsi*(3.0/4.0)+Ixx*dcz*dpsi*(3.0/4.0)+Iyy*dcx*dpsi*(3.0/4.0)+Iyy*dcy*dpsi*(3.0/4.0)+Iyy*dcz*dpsi*(3.0/4.0)+Izz*dcx*dpsi*(1.0/2.0)+Izz*dcy*dpsi*(1.0/2.0)+Izz*dcz*dpsi*(1.0/2.0)+Ixx*dphi*dpsi*(3.0/4.0)+Iyy*dphi*dpsi*(3.0/4.0)+Izz*dphi*dpsi*(1.0/2.0)+Ixx*dpsi*dtheta*(3.0/4.0)+Iyy*dpsi*dtheta*(3.0/4.0)+Izz*dpsi*dtheta*(1.0/2.0)+Ixx*(dtheta*dtheta)*cos(phi*2.0+theta)*(1.0/4.0)-Iyy*(dtheta*dtheta)*cos(phi*2.0+theta)*(1.0/4.0)+Ixx*(dpsi*dpsi)*cos(phi*2.0)*(1.0/4.0)-Iyy*(dpsi*dpsi)*cos(phi*2.0)*(1.0/4.0)+Ixx*(dpsi*dpsi)*cos(theta*2.0)*(1.0/4.0)+Iyy*(dpsi*dpsi)*cos(theta*2.0)*(1.0/4.0)-Izz*(dpsi*dpsi)*cos(theta*2.0)*(1.0/2.0)-Ixx*(dpsi*dpsi)*cos(phi*2.0-theta*2.0)*(1.0/8.0)-Ixx*(dpsi*dpsi)*cos(phi*2.0+theta*2.0)*(1.0/8.0)+Iyy*(dpsi*dpsi)*cos(phi*2.0-theta*2.0)*(1.0/8.0)+Iyy*(dpsi*dpsi)*cos(phi*2.0+theta*2.0)*(1.0/8.0)-Ixx*(dtheta*dtheta)*cos(phi*2.0-theta)*(1.0/4.0)+Iyy*(dtheta*dtheta)*cos(phi*2.0-theta)*(1.0/4.0)+Ixx*dcx*dphi*cos(theta)+Ixx*dcy*dphi*cos(theta)+Ixx*dcz*dphi*cos(theta)+Iyy*dcx*dphi*cos(theta)+Iyy*dcy*dphi*cos(theta)+Iyy*dcz*dphi*cos(theta)+Ixx*dphi*dpsi*cos(theta)+Iyy*dphi*dpsi*cos(theta)+Ixx*dphi*dtheta*cos(theta)+Iyy*dphi*dtheta*cos(theta)+Ixx*dcx*dtheta*cos(phi*2.0+theta)*(1.0/4.0)+Ixx*dcy*dtheta*cos(phi*2.0+theta)*(1.0/4.0)+Ixx*dcz*dtheta*cos(phi*2.0+theta)*(1.0/4.0)-Iyy*dcx*dtheta*cos(phi*2.0+theta)*(1.0/4.0)-Iyy*dcy*dtheta*cos(phi*2.0+theta)*(1.0/4.0)-Iyy*dcz*dtheta*cos(phi*2.0+theta)*(1.0/4.0)+Ixx*dphi*dtheta*cos(phi*2.0+theta)*(1.0/4.0)+Ixx*dpsi*dtheta*cos(phi*2.0+theta)*(1.0/4.0)-Iyy*dphi*dtheta*cos(phi*2.0+theta)*(1.0/4.0)-Iyy*dpsi*dtheta*cos(phi*2.0+theta)*(1.0/4.0)+Ixx*dcx*dpsi*cos(phi*2.0)*(1.0/4.0)+Ixx*dcy*dpsi*cos(phi*2.0)*(1.0/4.0)+Ixx*dcz*dpsi*cos(phi*2.0)*(1.0/4.0)-Iyy*dcx*dpsi*cos(phi*2.0)*(1.0/4.0)-Iyy*dcy*dpsi*cos(phi*2.0)*(1.0/4.0)-Iyy*dcz*dpsi*cos(phi*2.0)*(1.0/4.0)+Ixx*dphi*dpsi*cos(phi*2.0)*(1.0/4.0)-Iyy*dphi*dpsi*cos(phi*2.0)*(1.0/4.0)+Ixx*dpsi*dtheta*cos(phi*2.0)*(1.0/4.0)-Iyy*dpsi*dtheta*cos(phi*2.0)*(1.0/4.0)+Ixx*dcx*dpsi*cos(theta*2.0)*(1.0/4.0)+Ixx*dcy*dpsi*cos(theta*2.0)*(1.0/4.0)+Ixx*dcz*dpsi*cos(theta*2.0)*(1.0/4.0)+Iyy*dcx*dpsi*cos(theta*2.0)*(1.0/4.0)+Iyy*dcy*dpsi*cos(theta*2.0)*(1.0/4.0)+Iyy*dcz*dpsi*cos(theta*2.0)*(1.0/4.0)-Izz*dcx*dpsi*cos(theta*2.0)*(1.0/2.0)-Izz*dcy*dpsi*cos(theta*2.0)*(1.0/2.0)-Izz*dcz*dpsi*cos(theta*2.0)*(1.0/2.0)+Ixx*dphi*dpsi*cos(theta*2.0)*(1.0/4.0)+Iyy*dphi*dpsi*cos(theta*2.0)*(1.0/4.0)-Izz*dphi*dpsi*cos(theta*2.0)*(1.0/2.0)+Ixx*dpsi*dtheta*cos(theta*2.0)*(1.0/4.0)+Iyy*dpsi*dtheta*cos(theta*2.0)*(1.0/4.0)-Izz*dpsi*dtheta*cos(theta*2.0)*(1.0/2.0)-Ixx*dcx*dpsi*cos(phi*2.0-theta*2.0)*(1.0/8.0)-Ixx*dcx*dpsi*cos(phi*2.0+theta*2.0)*(1.0/8.0)-Ixx*dcy*dpsi*cos(phi*2.0-theta*2.0)*(1.0/8.0)-Ixx*dcy*dpsi*cos(phi*2.0+theta*2.0)*(1.0/8.0)-Ixx*dcz*dpsi*cos(phi*2.0-theta*2.0)*(1.0/8.0)-Ixx*dcz*dpsi*cos(phi*2.0+theta*2.0)*(1.0/8.0)+Iyy*dcx*dpsi*cos(phi*2.0-theta*2.0)*(1.0/8.0)+Iyy*dcx*dpsi*cos(phi*2.0+theta*2.0)*(1.0/8.0)+Iyy*dcy*dpsi*cos(phi*2.0-theta*2.0)*(1.0/8.0)+Iyy*dcy*dpsi*cos(phi*2.0+theta*2.0)*(1.0/8.0)+Iyy*dcz*dpsi*cos(phi*2.0-theta*2.0)*(1.0/8.0)+Iyy*dcz*dpsi*cos(phi*2.0+theta*2.0)*(1.0/8.0)-Ixx*dcx*dtheta*cos(phi*2.0-theta)*(1.0/4.0)-Ixx*dcy*dtheta*cos(phi*2.0-theta)*(1.0/4.0)-Ixx*dcz*dtheta*cos(phi*2.0-theta)*(1.0/4.0)+Iyy*dcx*dtheta*cos(phi*2.0-theta)*(1.0/4.0)+Iyy*dcy*dtheta*cos(phi*2.0-theta)*(1.0/4.0)+Iyy*dcz*dtheta*cos(phi*2.0-theta)*(1.0/4.0)-Ixx*dphi*dpsi*cos(phi*2.0-theta*2.0)*(1.0/8.0)-Ixx*dphi*dpsi*cos(phi*2.0+theta*2.0)*(1.0/8.0)+Iyy*dphi*dpsi*cos(phi*2.0-theta*2.0)*(1.0/8.0)+Iyy*dphi*dpsi*cos(phi*2.0+theta*2.0)*(1.0/8.0)-Ixx*dphi*dtheta*cos(phi*2.0-theta)*(1.0/4.0)-Ixx*dpsi*dtheta*cos(phi*2.0-theta)*(1.0/4.0)-Ixx*dpsi*dtheta*cos(phi*2.0-theta*2.0)*(1.0/8.0)-Ixx*dpsi*dtheta*cos(phi*2.0+theta*2.0)*(1.0/8.0)+Iyy*dphi*dtheta*cos(phi*2.0-theta)*(1.0/4.0)+Iyy*dpsi*dtheta*cos(phi*2.0-theta)*(1.0/4.0)+Iyy*dpsi*dtheta*cos(phi*2.0-theta*2.0)*(1.0/8.0)+Iyy*dpsi*dtheta*cos(phi*2.0+theta*2.0)*(1.0/8.0)

	return Cqd
