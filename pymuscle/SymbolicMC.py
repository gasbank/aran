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
	
	M[0,0] = Iww
	M[0,1] = 0.0
	M[0,2] = 0.0
	M[0,3] = 0.0
	M[0,4] = 0.0
	M[0,5] = 0.0
	M[1,0] = 0.0
	M[1,1] = Iww
	M[1,2] = 0.0
	M[1,3] = 0.0
	M[1,4] = 0.0
	M[1,5] = 0.0
	M[2,0] = 0.0
	M[2,1] = 0.0
	M[2,2] = Iww
	M[2,3] = 0.0
	M[2,4] = 0.0
	M[2,5] = 0.0
	M[3,0] = 0.0
	M[3,1] = 0.0
	M[3,2] = 0.0
	M[3,3] = Ixx+Iyy
	M[3,4] = 0.0
	M[3,5] = cos(theta)*(Ixx+Iyy)
	M[4,0] = 0.0
	M[4,1] = 0.0
	M[4,2] = 0.0
	M[4,3] = 0.0
	M[4,4] = Ixx+Izz-Ixx*pow(cos(phi),2.0)+Iyy*pow(cos(phi),2.0)
	M[4,5] = -cos(phi)*sin(phi)*sin(theta)*(Ixx-Iyy)
	M[5,0] = 0.0
	M[5,1] = 0.0
	M[5,2] = 0.0
	M[5,3] = cos(theta)*(Ixx+Iyy)
	M[5,4] = -cos(phi)*sin(phi)*sin(theta)*(Ixx-Iyy)
	M[5,5] = Ixx+Iyy-Iyy*pow(sin(theta),2.0)+Izz*pow(sin(theta),2.0)-Ixx*pow(sin(phi),2.0)*pow(sin(theta),2.0)+Iyy*pow(sin(phi),2.0)*pow(sin(theta),2.0)
	
	return M

def SymbolicMinv(q, I):
	Minv = zeros((6,6))

	Ixx, Iyy, Izz, Iww = I
	cx, cy, cz, phi, theta, psix = q

	Minv[0,0] = 1/Iww
	Minv[0,1] = 0.0
	Minv[0,2] = 0.0
	Minv[0,3] = 0.0
	Minv[0,4] = 0.0
	Minv[0,5] = 0.0
	Minv[1,0] = 0.0
	Minv[1,1] = 1/Iww
	Minv[1,2] = 0.0
	Minv[1,3] = 0.0
	Minv[1,4] = 0.0
	Minv[1,5] = 0.0
	Minv[2,0] = 0.0
	Minv[2,1] = 0.0
	Minv[2,2] = 1/Iww
	Minv[2,3] = 0.0
	Minv[2,4] = 0.0
	Minv[2,5] = 0.0
	Minv[3,0] = 0.0
	Minv[3,1] = 0.0
	Minv[3,2] = 0.0
	Minv[3,3] = -((Ixx*Ixx)*cos(phi*2.0)*(-1.0/2.0)+(Iyy*Iyy)*cos(phi*2.0)*(1.0/2.0)+(Ixx*Ixx)*cos(theta*2.0)*(1.0/2.0)+(Iyy*Iyy)*cos(theta*2.0)*(1.0/2.0)-(Izz*Izz)*cos(theta*2.0)-(Ixx*Ixx)*cos(phi*2.0-theta*2.0)*(1.0/4.0)-(Ixx*Ixx)*cos(phi*2.0+theta*2.0)*(1.0/4.0)+(Iyy*Iyy)*cos(phi*2.0-theta*2.0)*(1.0/4.0)+(Iyy*Iyy)*cos(phi*2.0+theta*2.0)*(1.0/4.0)+(Ixx*Ixx)*(1.0/2.0)+(Iyy*Iyy)*(1.0/2.0)+Izz*Izz+Ixx*Iyy*2.0+Ixx*Izz*2.0+Iyy*Izz*2.0)/((cos(theta*2.0)-1.0)*(Ixx+Iyy)*(Ixx+Izz)*(Iyy+Izz))
	Minv[3,4] = ((Ixx-Iyy)*(cos(phi*2.0-theta*2.0)-cos(phi*2.0+theta*2.0))*(1.0/4.0))/((cos(theta*2.0)-1.0)*(Ixx+Izz)*(Iyy+Izz))
	Minv[3,5] = (Ixx*cos(phi*2.0-theta)*(-1.0/2.0)+Iyy*cos(phi*2.0-theta)*(1.0/2.0)+Ixx*cos(theta)+Iyy*cos(theta)+Izz*cos(theta)*2.0-Ixx*cos(phi*2.0+theta)*(1.0/2.0)+Iyy*cos(phi*2.0+theta)*(1.0/2.0))/((cos(theta*2.0)-1.0)*(Ixx+Izz)*(Iyy+Izz))
	Minv[4,0] = 0.0
	Minv[4,1] = 0.0
	Minv[4,2] = 0.0
	Minv[4,3] = ((Ixx-Iyy)*(cos(phi*2.0-theta*2.0)-cos(phi*2.0+theta*2.0))*(1.0/4.0))/((cos(theta*2.0)-1.0)*(Ixx+Izz)*(Iyy+Izz))
	Minv[4,4] = -(Ixx*(1.0/2.0)+Iyy*(1.0/2.0)+Izz-Ixx*cos(phi*2.0-theta*2.0)*(1.0/4.0)-Ixx*cos(phi*2.0+theta*2.0)*(1.0/4.0)+Iyy*cos(phi*2.0-theta*2.0)*(1.0/4.0)+Iyy*cos(phi*2.0+theta*2.0)*(1.0/4.0)+Ixx*cos(phi*2.0)*(1.0/2.0)-Iyy*cos(phi*2.0)*(1.0/2.0)-Ixx*cos(theta*2.0)*(1.0/2.0)-Iyy*cos(theta*2.0)*(1.0/2.0)-Izz*cos(theta*2.0))/((cos(theta*2.0)-1.0)*(Ixx+Izz)*(Iyy+Izz))
	Minv[4,5] = ((cos(phi*2.0+theta)-cos(phi*2.0-theta))*(Ixx-Iyy)*(1.0/2.0))/((cos(theta*2.0)-1.0)*(Ixx+Izz)*(Iyy+Izz))
	Minv[5,0] = 0.0
	Minv[5,1] = 0.0
	Minv[5,2] = 0.0
	Minv[5,3] = (Ixx*cos(phi*2.0-theta)*(-1.0/2.0)+Iyy*cos(phi*2.0-theta)*(1.0/2.0)+Ixx*cos(theta)+Iyy*cos(theta)+Izz*cos(theta)*2.0-Ixx*cos(phi*2.0+theta)*(1.0/2.0)+Iyy*cos(phi*2.0+theta)*(1.0/2.0))/((cos(theta*2.0)-1.0)*(Ixx+Izz)*(Iyy+Izz))
	Minv[5,4] = ((cos(phi*2.0+theta)-cos(phi*2.0-theta))*(Ixx-Iyy)*(1.0/2.0))/((cos(theta*2.0)-1.0)*(Ixx+Izz)*(Iyy+Izz))
	Minv[5,5] = -(Ixx+Iyy+Izz*2.0-Ixx*cos(phi*2.0)+Iyy*cos(phi*2.0))/((cos(theta*2.0)-1.0)*(Ixx+Izz)*(Iyy+Izz))

	return Minv

def SymbolicCqd(q, qd, I):
	Cqd = zeros((6))

	Ixx, Iyy, Izz, Iww = I
	cx, cy, cz, phi, theta, psix = q
	dcx, dcy, dcz, dphi, dtheta, dpsix = qd

	Cqd[0] = 0.0
	Cqd[1] = 0.0
	Cqd[2] = 0.0
	Cqd[3] = Ixx*(dpsix*dpsix)*sin(phi*2.0)*(1.0/4.0)-Iyy*(dpsix*dpsix)*sin(phi*2.0)*(1.0/4.0)-Ixx*(dtheta*dtheta)*sin(phi*2.0)*(1.0/2.0)+Iyy*(dtheta*dtheta)*sin(phi*2.0)*(1.0/2.0)-Ixx*(dpsix*dpsix)*sin(phi*2.0-theta*2.0)*(1.0/8.0)-Ixx*(dpsix*dpsix)*sin(phi*2.0+theta*2.0)*(1.0/8.0)+Iyy*(dpsix*dpsix)*sin(phi*2.0-theta*2.0)*(1.0/8.0)+Iyy*(dpsix*dpsix)*sin(phi*2.0+theta*2.0)*(1.0/8.0)-Ixx*dpsix*dtheta*sin(theta)-Iyy*dpsix*dtheta*sin(theta)+Ixx*dpsix*dtheta*sin(phi*2.0+theta)*(1.0/2.0)-Iyy*dpsix*dtheta*sin(phi*2.0+theta)*(1.0/2.0)-Ixx*dpsix*dtheta*sin(phi*2.0-theta)*(1.0/2.0)+Iyy*dpsix*dtheta*sin(phi*2.0-theta)*(1.0/2.0)
	Cqd[4] = Ixx*(dpsix*dpsix)*sin(theta*2.0)*(1.0/4.0)+Iyy*(dpsix*dpsix)*sin(theta*2.0)*(1.0/4.0)-Izz*(dpsix*dpsix)*sin(theta*2.0)*(1.0/2.0)+Ixx*(dpsix*dpsix)*sin(phi*2.0-theta*2.0)*(1.0/8.0)-Ixx*(dpsix*dpsix)*sin(phi*2.0+theta*2.0)*(1.0/8.0)-Iyy*(dpsix*dpsix)*sin(phi*2.0-theta*2.0)*(1.0/8.0)+Iyy*(dpsix*dpsix)*sin(phi*2.0+theta*2.0)*(1.0/8.0)+Ixx*dphi*dpsix*sin(theta)+Iyy*dphi*dpsix*sin(theta)-Ixx*dphi*dpsix*sin(phi*2.0+theta)*(1.0/2.0)+Iyy*dphi*dpsix*sin(phi*2.0+theta)*(1.0/2.0)+Ixx*dphi*dtheta*sin(phi*2.0)-Iyy*dphi*dtheta*sin(phi*2.0)+Ixx*dphi*dpsix*sin(phi*2.0-theta)*(1.0/2.0)-Iyy*dphi*dpsix*sin(phi*2.0-theta)*(1.0/2.0)
	Cqd[5] = Ixx*(dtheta*dtheta)*sin(phi*2.0+theta)*(-1.0/4.0)+Iyy*(dtheta*dtheta)*sin(phi*2.0+theta)*(1.0/4.0)-Ixx*(dtheta*dtheta)*sin(phi*2.0-theta)*(1.0/4.0)+Iyy*(dtheta*dtheta)*sin(phi*2.0-theta)*(1.0/4.0)-Ixx*dphi*dtheta*sin(theta)-Iyy*dphi*dtheta*sin(theta)-Ixx*dphi*dtheta*sin(phi*2.0+theta)*(1.0/2.0)+Iyy*dphi*dtheta*sin(phi*2.0+theta)*(1.0/2.0)-Ixx*dphi*dpsix*sin(phi*2.0)*(1.0/2.0)+Iyy*dphi*dpsix*sin(phi*2.0)*(1.0/2.0)-Ixx*dpsix*dtheta*sin(theta*2.0)*(1.0/2.0)-Iyy*dpsix*dtheta*sin(theta*2.0)*(1.0/2.0)+Izz*dpsix*dtheta*sin(theta*2.0)+Ixx*dphi*dpsix*sin(phi*2.0-theta*2.0)*(1.0/4.0)+Ixx*dphi*dpsix*sin(phi*2.0+theta*2.0)*(1.0/4.0)-Iyy*dphi*dpsix*sin(phi*2.0-theta*2.0)*(1.0/4.0)-Iyy*dphi*dpsix*sin(phi*2.0+theta*2.0)*(1.0/4.0)+Ixx*dphi*dtheta*sin(phi*2.0-theta)*(1.0/2.0)-Ixx*dpsix*dtheta*sin(phi*2.0-theta*2.0)*(1.0/4.0)+Ixx*dpsix*dtheta*sin(phi*2.0+theta*2.0)*(1.0/4.0)-Iyy*dphi*dtheta*sin(phi*2.0-theta)*(1.0/2.0)+Iyy*dpsix*dtheta*sin(phi*2.0-theta*2.0)*(1.0/4.0)-Iyy*dpsix*dtheta*sin(phi*2.0+theta*2.0)*(1.0/4.0)

	return Cqd
