#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Generic box-shaped 6-DOF 3D rigid body

Supported parameterization modes
  Linear position                : Cartesian coordinates (3-DOF)
  Angular position (orientation) : Euler, Quaternion, Exponential map
"""

import copy
from math import sqrt, pi, ceil, sin
from scipy import sparse
from numpy import hstack, identity, hstack, vstack, array, zeros, ones, linalg, dot, set_printoptions, nan

from SymbolicTensor import *
from MathUtil import *
from quat import *
from dynamics import BoxInertia
from MathUtil import *
from dRdv_real import *
import GlobalContext
from ExpBodyMoEq_real import MassMatrixAndCqdVector
from dRdv_real import GeneralizedForce
import ExpBody
import ZV
import Parameters

class RigidBody:
    def __init__(self, name, pName, mass, boxsize, q, qd, dc, rotParam):
        """
        Parameter details
        -----------------
        
        name     : descriptive body name
        pName    : descriptive parent body name (None if root)
        mass     : mass of the body (in kilograms)
        size     : size tuple (sx, sy, sz) of rigid body (in meters)
        q        : linear & angular position of current instant (6 or 7 dimension determined by rotParam)
        qd       : linear & angular velocity of current instant (6 or 7 dimension determined by rotParam)
        chi_0    : linear & angular position of previos instant (6 or 7 dimension determined by rotParam)
        rotParam : type of rotation parameterization
        dc       : drawing color (three RGB channel values range from 0.0 to 1.0)
        h        : simulation time step (in seconds)
        """
        assert len(boxsize) == 3
        assert(rotParam in ['EULER_XYZ', 'EULER_ZXZ', 'QUAT_WFIRST', 'EXP'])
        sx, sy, sz = boxsize
        
        self.name    = name
        self.pName   = pName
        self.mass    = mass
        self.boxsize = boxsize
        self.q       = q               # Stored in World Coordinates
        self.qd      = qd              # Stored in World or body coordinates
        self.qdCo    = 'WC'            # linear & angular velocity coordinate frame type. 'WC' or 'BC'
        self.dc      = dc
        self.rho     = mass / (sx*sy*sz) # density of the body
        self.I       = SymbolicTensor(sx, sy, sz, self.rho) # Computes diagonal elements of 4x4 tensor matrix of box-shaped rigid body
        self.corners = [ ( sx/2,  sy/2,  sz/2),
                         ( sx/2,  sy/2, -sz/2),
                         ( sx/2, -sy/2,  sz/2),
                         ( sx/2, -sy/2, -sz/2),
                         (-sx/2,  sy/2,  sz/2),
                         (-sx/2,  sy/2, -sz/2),
                         (-sx/2, -sy/2,  sz/2),
                         (-sx/2, -sy/2, -sz/2) ]
        self.rotParam  = rotParam
        if rotParam in ['EULER_XYZ', 'EULER_ZXZ', 'EXP']:
            self.nd = 6
        elif rotParam in ['QUAT_WFIRST']:
            self.nd = 7
        self.chi_0    = q - GlobalContext.GetSimTimeStep()*qd
        assert self.nd == len(q) and self.nd == len(qd) and self.nd == len(self.chi_0)
        
        self.ME       = identity(3) * self.mass
        self.IN       = BoxInertia(self.boxsize, self.mass)
        self.M_BC     = self.getMassMatrix_BC()
        self.cf       = []
        self.isDirty  = True               # State dependent values need to be updated.
        self.fibers   = []                 # Muscle fibers related to this RB.
        self.chi_ref  = self.q.copy()       # Reference state
    def __str__(self):
        ret = ''
        ret += 'RigidBody %s (parent:%s) isDirty=%s\n' % (self.name, self.pName, self.isDirty)
        ret += 'Current  %s%s <%s>\n' % (str(self.q[0:3]), str(self.q[3:self.nd]), self.rotParam)
        ret += 'Previous %s%s\n' % (str(self.chi_0[0:3]), str(self.chi_0[3:self.nd]))
        if hasattr(self, 'extForce'):
            ret += '   extForce: %s' % self.extForce
        return ret
        
    def getStabilityStr(self):
        """
        Returns how stable the exponential map orientation value.
        (It is unstable if theta is close to 2n*pi where n=+-1, +-2, ...)
        """
        assert self.rotParam == 'EXP'
        th = linalg.norm(self.q[3:6])
        thStability = 1.-abs(th-2*pi*int(th/(2*pi)))/(2*pi)
        return 'theta=%s / stability=%s' % (str(th), str(thStability))
    def updateFiberRelatedValues(self):
        assert not self.isDirty
        # Fiber related code
        chi_1 = self.q
        self.R = []
        for f in self.fibers:
            attPosLocal     = f.getAttPosLocalOf(self)
            attDirection    = f.getForceDirectionOf(self)[0:3]
            attDirectionLen = linalg.norm(attDirection)
            if attDirectionLen > 1e-6:
                attDirectionNormalized = attDirection / attDirectionLen
                Rj = GeneralizedForce(chi_1[3:6], attDirectionNormalized, attPosLocal)
            else:
                Rj = GeneralizedForce(chi_1[3:6], attDirection, attPosLocal)
            self.R.append(Rj)
    def updateCurrentStateDependentValues(self):
        #assert self.isDirty
        
        h = GlobalContext.GetSimTimeStep()
        frameReport = ''
        chi_1 = self.q
        vel_wc = (chi_1 - self.chi_0) / h
        
        if self.rotParam == 'EXP':
            self.M, self.C = MassMatrixAndCqdVector(chi_1[0:3], chi_1[3:6], vel_wc[0:3], vel_wc[3:6], self.I)
            self.Minv = linalg.inv(self.M)
            GRAV = GlobalContext.GetGravitationalAcceleration()
            self.f_g = GeneralizedForce(chi_1[3:6],
                                        (0., 0., -GRAV * self.mass),
                                        (0., 0., 0.))
            self.W = ZV.GetWFrom6Dof(chi_1)
            self.W_0 = ZV.GetWFrom6Dof(self.chi_0)
            self.dRdv_tensor = ZV.dRdv_tensor(chi_1)
        
            self.pc = []
            if hasattr(self, 'p') and len(self.p) > 0:
                for j in self.p:
                    pcj = array(list(self.corners[j]) + [1.])
                    pcj_1_W = dot(self.W, pcj)
                    self.pc.append(pcj_1_W)
            if len(self.pc):
                frameReport += ' '*4 + ' - %s previously anticipated (current) contact points' % self.name + '\n'
                for pf, pfi in zip(self.pc, self.p):
                    frameReport += ' '*10 + str(pfi) + ':' + str(pf) + '\n'
                
            # Calculate estimated position of contact points
            # assuming that there are no contact/muscle/external forces exist
            self.p      = [] # List of anticipated contact points indices
            self.pc_fix = [] # Fixing points of contact points
            chi_2_nocf = 2*self.q - self.chi_0 + dot(self.Minv, h**2*(- self.C + self.f_g))
            W_2_nocf = ZV.GetWFrom6Dof(chi_2_nocf)
            for j in xrange(8):
                pcj = array(list(self.corners[j]) + [1.])
                pcj_2_nocf_W = dot(W_2_nocf, pcj)
                pcj_1_W = dot(self.W, pcj)
                #print 'est cpz =', pcj_2_nocf_W[2]
                if pcj_2_nocf_W[2] <= 0: # If estimated position is penetrating the ground...
                    self.p.append(j)
                    pcj_fix = (pcj_1_W + pcj_2_nocf_W)/2
                    pcj_fix[2] = 0
                    self.pc_fix.append(pcj_fix)
            if len(self.pc_fix):
                frameReport += ' '*4 + ' - %s anticipated contact points fixing positions (pc_fix)' % self.name + '\n'
                for pf, pfi in zip(self.pc_fix, self.p):
                    frameReport += ' '*10 + str(pfi) + ':' + str(pf) + '\n'
            
            self.np = len(self.p) # eight contact points (worst case)        
            np = self.np
            nd = self.nd
            nm = len(self.fibers)
            Asubrowsizes = [nd, nd*np, 4*np, np, 4*np, nd]
            Asubcolsizes = [nd,          # chi^{(l+1)}
                            nd*np,       # f_c
                            5*np,        # c_c
                            4*np,        # \tilde{p}_c^{(l+1)}
                            4*np,        # \Delta \tilde{p}_c
                            np,          # \epsilon_c
                            np,          # \mu f_{c,z}
                            nd,          # \Delta \chi
                            1,           # \epsilon_\Delta
                            nd*nm        # f_T
                            ]
            self.Ari = [0]
            self.Aci = [0]
            for a in Asubrowsizes:
                self.Ari.append( self.Ari[-1] + a )
            for a in Asubcolsizes:
                self.Aci.append( self.Aci[-1] + a )
            
            self.Z    = []
            self.V    = []
            self.Q    = []
            
            for j in self.p:
                pcj = array(list(self.corners[j]) + [1.])
                Zj, Vj, Qj = ZV.ZVQ(self.q, pcj, array([0.,0,1,0]), self.W, self.dRdv_tensor)
                self.Z.append(Zj)
                self.V.append(Vj)
                self.Q.append(Qj)
        
        if self.rotParam == 'EXP':
            self.rotMat = RotationMatrixFromV(self.q[3:6])
        elif self.rotParam == 'EULER_XYZ':
            self.rotMat = RotationMatrixFromEulerAngles_xyz(self.q[3], self.q[4], self.q[5])
        elif self.rotParam == 'EULER_ZXZ':
            self.rotMat = RotationMatrixFromEulerAngles_zxz(self.q[3], self.q[4], self.q[5])
        elif self.rotParam == 'QUAT_WFIRST':
            self.rotMat = RotationMatrixFromQuaternion(self.q[3], self.q[4], self.q[5], self.q[6])
        self.isDirty = False
        return frameReport
    
    def getAMatrix(self):
        '''
        Builds and returns an equality constraint coefficient matrix for MOSEK.
        
        AMatrix * <optimization vector> = Eta
        
        '''
        assert not self.isDirty
        h  = GlobalContext.GetSimTimeStep()
        MU = GlobalContext.GetFrictionCoefficient()
        nd = self.nd
        np = len(self.p)
        ri = self.Ari
        ci = self.Aci
        A = sparse.lil_matrix((ri[-1], ci[-1]))
        A[ ri[0]:ri[1], ci[0]:ci[1] ] = self.M/(h**2)
        for j in xrange(np):
            A[ ri[0]:ri[1]            , ci[1]+nd*j:ci[1]+nd*(j+1) ] = -sparse.identity(nd) # Contact force
        for j in xrange(len(self.fibers)):
            A[ ri[0]:ri[1]            , ci[9]+nd*j:ci[9]+nd*(j+1) ] = -sparse.identity(nd) # Muscle force
        if np > 0:
            A[ ri[1]:ri[2], ci[1]:ci[2] ] = -sparse.identity(nd*np)
        for j in xrange(np):
            A[ ri[1]+nd*j:ri[1]+nd*(j+1), ci[2]+5*j:ci[2]+5*(j+1) ] = self.Q[j]
        if np > 0:
            A[ ri[2]:ri[3],             ci[3]:ci[4]             ] = sparse.identity(4*np)
            A[ ri[2]:ri[3],             ci[4]:ci[5]             ] = -sparse.identity(4*np)
        for j in xrange(np):
            A[ ri[3]+j                , ci[1]+nd*j:ci[1]+nd*(j+1) ] = array([0.,0,MU,0,0,0])
        if np > 0:
            A[ ri[3]:ri[4],             ci[6]:ci[7]            ] = -sparse.identity(np)
        for j in xrange(np):    
            A[ ri[4]+4*j:ri[4]+4*(j+1), ci[0]:ci[1]             ] = self.Z[j]
        if np > 0:    
            A[ ri[4]:ri[5],             ci[3]:ci[4]             ] = -sparse.identity(4*np)
        A[ ri[5]:ri[6],             ci[0]:ci[1]             ] = sparse.identity(nd)
        A[ ri[5]:ri[6],             ci[7]:ci[8]             ] = -sparse.identity(nd)
        return A
    def getEta(self):
        '''
        Builds and returns an equality constraint vector for MOSEK.
        
        AMatrix * <optimization vector> = Eta
        
        '''
        assert not self.isDirty
        nd = self.nd
        np = len(self.p)
        ri = self.Ari
        ci = self.Aci
        h  = GlobalContext.GetSimTimeStep()
        eta = sparse.lil_matrix((ri[-1], 1))
        eta[ ri[0]:ri[1], 0 ] = ( dot(self.M, 2*self.q - self.chi_0)/(h**2) + (- self.C + self.f_g) ).reshape(nd,1)
        for j in xrange(np):
            eta[ ri[2]+4*j:ri[2]+4*(j+1), 0 ] = self.pc_fix[j].reshape(4,1)
            eta[ ri[4]+4*j:ri[4]+4*(j+1), 0 ] = -self.V[j].reshape(4,1)
        #eta[ ri[5]:ri[6], 0 ] = self.q.reshape(nd,1)
        eta[ ri[5]:ri[6], 0 ] = self.chi_ref.reshape(6,1)
        return eta
    def setChi_1(self, chi_1):
        self.chi_0     = self.q.copy() # Copy chi_1 to chi_0
        self.q = array(chi_1)   # Copy new chi_1 to chi_1
        frameReport = ''
        self.isDirty   = True
        return frameReport
    def reparameterize(self):
        assert self.rotParam == 'EXP'
        # Reparameterize exp map rot
        th_1 = linalg.norm(self.q[3:6])
        frameReport = ''
        '''
        if th_1 > pi:
            qDiff = self.q[3:6] - self.chi_0[3:6]
            self.q[3:6] = (1-2*pi/th_1)*self.q[3:6]
            self.chi_0[3:6]     = self.q[3:6] - qDiff
            frameReport += '  Exp map rotation reparameterized.\n'
        '''
        th_0 = linalg.norm(self.chi_0[3:6])
        if th_1 > pi and th_0 > pi:
            self.chi_0[3:6] = (1-2*pi/th_0)*self.chi_0[3:6]
            self.q[3:6] = (1-2*pi/th_1)*self.q[3:6]
            frameReport += '  Exp map rotation reparameterized.\n'
        self.isDirty   = True
        return frameReport

    def setQd(self, linvel, omega_wc):
        assert self.rotParam == 'EXP'
        self.qd         = hstack([linvel, VdotFromOmega(self.q[3:6], omega_wc)])
        
    def globalPos(self, localPos):
        '''
        Transform local coordinate position(point) to global coordinate position
        '''
        return self.q[0:3] + dot(self.getRotMat(), localPos)
    def localVec(self, globalVec):
        '''
        Transform global coordinate vector to local coordinate vector
        '''
        return dot(linalg.inv(self.getRotMat()), globalVec)
    
    def getRotMat(self):
        assert not self.isDirty
        return self.rotMat
    
    def getCorners_WC(self):
        cornersWc = []
        for c in self.corners:
            cw = self.q[0:3] + dot(self.getRotMat(), c)
            cornersWc.append(cw)
        return cornersWc
    
    def getMassMatrix_BC(self):
        ret = vstack([  hstack([ self.ME, zeros((3,3)) ]),
                        hstack([ zeros((3,3)), self.IN ]) ])
        return ret
    def getMassMatrix_WC(self, q=None):
        if q==None:
            q = self.q
        ret = vstack([  hstack([ self.ME     , zeros((3,3))    ]),
                        hstack([ zeros((3,3)), self.getIN_WC(q) ])   ])
        return ret
    def getCoriolisVector_WC(self, q=None, omega_wc =None):
        if q==None:
            q = self.q
        if omega_wc==None:
            omega_wc = self.omega_wc
        return hstack([zeros(3), array(cross(self.omega_wc, dot(self.getIN_WC(q), self.omega_wc)))])
    def getIN_WC(self, q=None):
        if q==None:
            A = self.getRotMat()
        else:
            A = RotationMatrixFromV(q[3:6])
        return dot(dot(A, self.IN), A.T)

    
    def globalPos(self, localPos):
        '''
        Transform local coordinate position(point) to global coordinate position
        '''
        return self.q[0:3] + dot(self.getRotMat(), localPos)
    def localVec(self, globalVec):
        '''
        Transform global coordinate vector to local coordinate vector
        '''
        Ainv = linalg.inv(self.getRotMat())
        return dot(Ainv, globalVec)
        
    def getCorners_WC(self):
        assert self.rotParam == 'QUAT_WFIRST'
        cornersWc = []
        for c in self.corners:
            cw = self.q[0:3] + dot(self.getRotMat(), c)
            cornersWc.append(cw)
        return cornersWc
    
    def getMassMatrix_BC(self):
        ret = vstack([  hstack([ self.ME, zeros((3,3)) ]),
                        hstack([ zeros((3,3)), self.IN ]) ])
        return ret
    def getMassMatrix_WC(self, q=None):
        if q==None:
            q = self.q
        ret = vstack([  hstack([ self.ME     , zeros((3,3))    ]),
                        hstack([ zeros((3,3)), self.getIN_WC(q) ])   ])
        return ret
    def getCoriolisVector_BC(self):
        assert self.rotParam == 'QUAT_WFIRST'
        omega = QuatToAngularVel_BC(self.q[3:7], self.qd[3:7])
        return hstack([zeros(3), array(cross(omega, dot(self.IN, omega)))])
    def getExtVector_BC(self):
        assert self.rotParam == 'QUAT_WFIRST'
        omega = QuatToAngularVel_BC(self.q[3:7], self.qd[3:7])
        V1_world = array([0,0,-9.81]) * self.mass
        
        V1_body = quat_rot(quat_conj(self.q[3:7]), V1_world)
        V2_body = -array(cross(omega, dot(self.IN, omega)))
        return hstack([V1_body, V2_body])

    def getIN_WC(self, q=None):
        assert self.rotParam == 'QUAT_WFIRST'
        if q==None:
            q = self.q
            A = RotationMatrixFromQuaternion(q[3],q[4],q[5],q[6])
        else:
            A = self.getRotMat()
        return dot(dot(A, self.IN), A.T)
    
    def getExtVector_WC(self, q=None, qd=None):
        assert self.rotParam == 'QUAT_WFIRST'
        if q==None:
            q = self.q
        if qd==None:
            qd = self.qd
        omega = QuatToAngularVel_WC(q[3:7], qd[3:7])
        #omega = QuatToAngularVel_BC(self.q[3:7], self.qd[3:7])
        
        V1_world = array([0,0,-9.81]) * self.mass
        #V1_world = array([0,0,-9.81]) * 0
        
        #V2_body = -array(cross(omega, dot(self.IN, omega)))
        V2_world = -array(cross(omega, dot(self.getIN_WC(q), omega)))
        return hstack([V1_world, V2_world])
        
    def explicitStep(self, h, acc):
        assert self.rotParam == 'QUAT_WFIRST'
        self.q  += self.qd  * h
        self.qd += array(acc) * h
        
    def getLinearAngularVelocity_BC(self):
        assert self.rotParam == 'QUAT_WFIRST'
        if self.qdCo == 'BC':
            qdLin = self.qd[0:3]
        else:
            qdLin = quat_rot(quat_conj(self.q[3:7]), self.qd[0:3])
            
        return hstack([ qdLin, QuatToAngularVel_BC( self.q[3:7], self.qd[3:7] ) ])
    def getLinearAngularVelocity_WC(self):
        assert self.rotParam == 'QUAT_WFIRST'
        if self.qdCo == 'BC':
            qdLin = quat_rot(self.q[3:7], self.qd[0:3])
        else:
            qdLin = self.qd[0:3]
            
        return hstack([ qdLin, QuatToAngularVel_WC( self.q[3:7], self.qd[3:7] ) ])


if __name__ == '__main__':
    rb = RigidBody('Test rigid body',
                   'Test parent rigid body',
                   1.0,
                   (1.0,2.0,3.0),
                   array([10.,20,30,0.1,0.2,0.3]),
                   array([1.,0,0,0,0,0]),
                   (0.1,0.2,0.3),
                   'EXP')
    rb.updateCurrentStateDependentValues()
    print rb
