#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Optimization-based rigid body simulator
2010 Geoyeob Kim

Nonlinear muscle fiber
"""

from SymbolicTensor import *
from MathUtil import *
import GlobalContext

class MuscleFiber:
    def __init__(self, name, mType, bAttachedPosNormalized, T_0, A, kse, kpe, b, x_r0, x_rl, x_ru, p1Name, p2Name, p1, p2, p1AttPos, p2AttPos):
        """
        Parameters details:
        ---------------------------------------
        name                    : descriptive muscle fiber name
        mType                   : 'MUSCLE' or 'LIGAMENT'
        bAttachedPosNormalized  : fiber attached position represented in normalized coordinates or not.
        T_0                     : (previous instant's) tension force
        kse                     : serial elastic spring constant
        kpe                     : parallel elastic spring constant
        b                       : parallel viscosity
        x_rl                    : lower bound of rest length
        x_ru                    : upper bound of rest length
        p1Name                  : origin body name
        p2Name                  : insertion body name
        p1                      : origin body instance (RigidBody)
        p2                      : insertion body instance (RigidBody)
        p1AttPos                : origin body attached position
        p2AttPos                : insertion body attached position
        """
        self.name     = name
        self.p1Name   = p1Name
        self.orgBody  = p1Name
        self.p1AttPos = p1AttPos
        self.orgPos   = p1AttPos
        self.p2Name   = p2Name
        self.insBody  = p2Name
        self.p2AttPos = p2AttPos
        self.insPos   = p2AttPos
        self.mType    = mType
        assert(mType in ['MUSCLE', 'LIGAMENT'])
        self.bAttachedPosNormalized = bAttachedPosNormalized
        assert(bAttachedPosNormalized in [True, False])
        self.p1  = p1
        self.p2  = p2
        if p1 is not None and p2 is not None:
            assert p1 is not p2
            p1.fibers.append(self)
            p2.fibers.append(self)
        #
        # Dynamics properties
        #
        self.kse     = kse
        self.KSE     = kse
        self.kpe     = kpe
        self.KPE     = kpe
        self.b       = b
        self.xrest   = x_r0
        self.xr_0    = x_r0
        self.T       = T_0
        self.T_0     = T_0
        self.A       = A
    
        self.T_0  = T_0
        self.x_r0 = x_r0
        self.kse  = kse
        self.kpe  = kpe
        self.b    = b
        if x_rl is None or x_ru is None:
            self.x_rl = x_r0 - x_r0/2.0
            self.x_ru = x_r0 + x_r0/2.0
        else:
            self.x_rl = x_rl
            self.x_ru = x_ru
    def setP1andP2(self, p1, p2):
        assert self.p1 == None and self.p2 == None
        self.p1  = p1
        self.p2  = p2
        if p1 is not None and p2 is not None:
            assert p1 is not p2
            p1.fibers.append(self)
            p2.fibers.append(self)
    def __str__(self):
        return 'T_0=' + str(self.T_0) + ' x_r0=' + str(self.x_r0)
    def setT_0(self, T_0):
        self.T_0 = T_0
    def getK(self):
        h = GlobalContext.GetSimTimeStep()
        return array([ self.kse * self.b + self.kse**2 * h + self.kse * h * self.kpe,
                       -self.kse**2 * h,
                       self.kse**2 * h * self.kpe ])
    def gets(self):
        assert not self.p1.isDirty
        assert not self.p2.isDirty
        p1AttPosGbl   = dot(self.p1.W,   array(list(self.p1AttPos)+[1.]))
        p2AttPosGbl   = dot(self.p2.W,   array(list(self.p2AttPos)+[1.]))
        p1AttPosGbl_0 = dot(self.p1.W_0, array(list(self.p1AttPos)+[1.]))
        p2AttPosGbl_0 = dot(self.p2.W_0, array(list(self.p2AttPos)+[1.]))
        x_0 = linalg.norm(p1AttPosGbl_0 - p2AttPosGbl_0)
        x_1 = linalg.norm(p1AttPosGbl   - p2AttPosGbl  )
        h = GlobalContext.GetSimTimeStep()
        s = self.kse * self.b * self.T_0 + self.kse**2 * h * self.kpe * x_1 + self.kse**2 * self.b * (x_1 - x_0)
        #print '******** s =', s, 'x0=', x_0, 'x1=', x_1
        return s
    def getForceDirectionOf(self, p):
        assert not p.isDirty
        if p is self.p1:
            tail, head = self.p1, self.p2
            tailAttPos, headAttPos = self.p1AttPos, self.p2AttPos
        elif p is self.p2:
            tail, head = self.p2, self.p1
            tailAttPos, headAttPos = self.p2AttPos, self.p1AttPos
        else:
            assert False, 'What\'s going on here?'
        tailAttPosGbl = dot(tail.W, array(list(tailAttPos)+[1.]))
        headAttPosGbl = dot(head.W, array(list(headAttPos)+[1.]))
        return headAttPosGbl - tailAttPosGbl
    def getAttPosLocalOf(self, p):
        assert not p.isDirty
        if p is self.p1:
            return self.p1AttPos
        elif p is self.p2:
            return self.p2AttPos
        else:
            assert False, 'What\'s going on here?'

if __name__ == '__main__':
    #name, mType, bAttachedPosNormalized, T_0, A, kse, kpe, b, x_r0, x_rl, x_ru, p1Name, p2Name, p1, p2, p1AttPos, p2AttPos
    mf = MuscleFiber('Test muscle fiber', 'MUSCLE', False, 0, 0, 1, 1, 1, 1, 0.5, 1.5, 'p1name', 'p2name', None, None, array([0.0,0,0]), array([1.,2,3]))
    print mf
