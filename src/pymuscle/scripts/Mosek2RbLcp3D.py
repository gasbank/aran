# MosekRbLcp3D.py
# 2010 Geoyeob Kim
# As a part of the thesis implementation
#
# A rigid body with flat ground
#
# Rigid body state is parameterized by ExpMap (6-DOF)
#
import sys
from numpy import array, zeros, ones, linalg, hstack, dot,set_printoptions,nan
import mosek
from math import sqrt, pi, ceil, sin
from scipy import sparse
from ExpBodyMoEq_real import MassMatrixAndCqdVector
from dRdv_real import GeneralizedForce
import ExpBody
import ZV
import copy
# Since the actual value of Infinity is ignores, we define it solely
# for symbolic purposes:
inf = 0.0
MU = 1.
GRAV = 9.81
h = 0.01

class RigidBody:
    def __init__(self, bodyName, chi_0, chi_1, mass, boxsize):
        self.chi_0 = copy.deepcopy(chi_0)
        vel_wc = (chi_1 - chi_0) / h
        self.expBody = ExpBody.ExpBody(bodyName, None, mass, boxsize,
                                       copy.deepcopy(chi_1),
                                       vel_wc[0:3],
                                       vel_wc[3:6],
                                       [0.1,0.2,0.3])
        self.isDirty     = True
        self.nd          = 6 # 6-DOF
        self.fibers      = []
        self.chi_ref = copy.deepcopy(chi_0)
    def __str__(self):
        return '%s%s' % (str(self.expBody.q[0:3]), str(self.expBody.q[3:6]))
    def getStabilityStr(self):
        th = linalg.norm(self.expBody.q[3:6])
        thStability = 1.-abs(th-2*pi*int(th/(2*pi)))/(2*pi)
        return 'theta=%s / stability=%s' % (str(th), str(thStability))
    
    def updateFiberRelatedValues(self):
        assert not self.isDirty
        # Fiber related code
        chi_1 = self.expBody.q
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
        assert self.isDirty
        frameReport = ''
        chi_1 = self.expBody.q
        vel_wc = (chi_1 - self.chi_0) / h
        self.M, self.C = MassMatrixAndCqdVector(chi_1[0:3], chi_1[3:6], vel_wc[0:3], vel_wc[3:6], self.expBody.I)
        self.Minv = linalg.inv(self.M)
        self.f_g = GeneralizedForce(chi_1[3:6],
                                    (0., 0., -GRAV * self.expBody.mass),
                                    (0., 0., 0.))
        self.W = ZV.GetWFrom6Dof(chi_1)
        self.W_0 = ZV.GetWFrom6Dof(self.chi_0)
        self.dRdv_tensor = ZV.dRdv_tensor(chi_1)
        
        self.pc = []
        if hasattr(self, 'p') and len(self.p) > 0:
            for j in self.p:
                pcj = array(list(self.expBody.corners[j]) + [1.])
                pcj_1_W = dot(self.W, pcj)
                self.pc.append(pcj_1_W)
        if len(self.pc):
            frameReport += ' '*4 + ' - %s previously anticipated (current) contact points' % self.expBody.name + '\n'
            for pf, pfi in zip(self.pc, self.p):
                frameReport += ' '*10 + str(pfi) + ':' + str(pf) + '\n'
                
        # Calculate estimated position of contact points
        # assuming that there are no contact/muscle/external forces exist
        self.p      = [] # List of anticipated contact points indices
        self.pc_fix = [] # Fixing points of contact points
        chi_2_nocf = 2*self.expBody.q - self.chi_0 + dot(self.Minv, h**2*(- self.C + self.f_g))
        W_2_nocf = ZV.GetWFrom6Dof(chi_2_nocf)
        for j in xrange(8):
            pcj = array(list(self.expBody.corners[j]) + [1.])
            pcj_2_nocf_W = dot(W_2_nocf, pcj)
            pcj_1_W = dot(self.W, pcj)
            #print 'est cpz =', pcj_2_nocf_W[2]
            if pcj_2_nocf_W[2] <= 0: # If estimated position is penetrating the ground...
                self.p.append(j)
                pcj_fix = (pcj_1_W + pcj_2_nocf_W)/2
                pcj_fix[2] = 0
                self.pc_fix.append(pcj_fix)
        if len(self.pc_fix):
            frameReport += ' '*4 + ' - %s anticipated contact points fixing positions (pc_fix)' % self.expBody.name + '\n'
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
            pcj = array(list(self.expBody.corners[j]) + [1.])
            Zj, Vj, Qj = ZV.ZVQ(self.expBody.q, pcj, array([0.,0,1,0]), self.W, self.dRdv_tensor)
            self.Z.append(Zj)
            self.V.append(Vj)
            self.Q.append(Qj)
        self.isDirty = False
        return frameReport
    
    
    def getAMatrix(self):
        assert not self.isDirty
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
        assert not self.isDirty
        nd = self.nd
        np = len(self.p)
        ri = self.Ari
        ci = self.Aci
        
        eta = sparse.lil_matrix((ri[-1], 1))
        eta[ ri[0]:ri[1], 0 ] = ( dot(self.M, 2*self.expBody.q - self.chi_0)/(h**2) + (- self.C + self.f_g) ).reshape(nd,1)
        for j in xrange(np):
            eta[ ri[2]+4*j:ri[2]+4*(j+1), 0 ] = self.pc_fix[j].reshape(4,1)
            eta[ ri[4]+4*j:ri[4]+4*(j+1), 0 ] = -self.V[j].reshape(4,1)
        #eta[ ri[5]:ri[6], 0 ] = self.expBody.q.reshape(nd,1)
        eta[ ri[5]:ri[6], 0 ] = self.chi_ref.reshape(6,1)
        return eta
    def setChi_1(self, chi_1):
        self.chi_0     = copy.deepcopy(self.expBody.q) # Copy chi_1 to chi_0
        self.expBody.q = array(chi_1)   # Copy new chi_1 to chi_1
        frameReport = ''
        self.isDirty   = True
        return frameReport
    def reparameterize(self):
        # Reparameterize exp map rot
        th_1 = linalg.norm(self.expBody.q[3:6])
        frameReport = ''
        '''
        if th_1 > pi:
            qDiff = self.expBody.q[3:6] - self.chi_0[3:6]
            self.expBody.q[3:6] = (1-2*pi/th_1)*self.expBody.q[3:6]
            self.chi_0[3:6]     = self.expBody.q[3:6] - qDiff
            frameReport += '  Exp map rotation reparameterized.\n'
        '''
        th_0 = linalg.norm(self.chi_0[3:6])
        if th_1 > pi and th_0 > pi:
            self.chi_0[3:6] = (1-2*pi/th_0)*self.chi_0[3:6]
            self.expBody.q[3:6] = (1-2*pi/th_1)*self.expBody.q[3:6]
            frameReport += '  Exp map rotation reparameterized.\n'
        self.isDirty   = True
        return frameReport

class Fiber:
    def __init__(self, T_0, kse, kpe, b, x_r0, x_rl, x_ru, p1, p2, p1AttPos, p2AttPos):
        # p1 : Origin body
        # p2 : Insertion body
        self.T_0  = T_0
        self.x_r0 = x_r0
        self.kse  = kse
        self.kpe  = kpe
        self.b    = b
        self.x_rl = x_rl
        self.x_ru = x_ru
        assert p1 is not p2
        self.p1  = p1
        self.p2  = p2
        p1.fibers.append(self)
        p2.fibers.append(self)
        self.p1AttPos = p1AttPos
        self.p2AttPos = p2AttPos
    def __str__(self):
        return 'T_0=' + str(self.T_0) + ' x_r0=' + str(self.x_r0)
    def setT_0(self, T_0):
        self.T_0 = T_0
    def getK(self):
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
        return self.kse * self.b * self.T_0 + self.kse**2 * h * self.kpe * x_1 + self.kse**2 * self.b * (x_1 - x_0)
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

# Define a stream printer to grab output from MOSEK
def streamprinter(text):
    sys.stdout.write(text)
    sys.stdout.flush()

def PrintOptimizedResult(p1, x_opt):
    ci = p1.Aci # Sub-column indices
    chi_1 = x_opt[ ci[0]:ci[1] ]
    f_c   = x_opt[ ci[1]:ci[2] ]
    c_c   = x_opt[ ci[2]:ci[3] ]
    print 'Optimal solution'
    print '-'*80
    print '       BODY 1'
    print '     chi^(i+1) =', chi_1[0:3], chi_1[3:6]
    print '           f_c ='
    for j in xrange(len(p1.p)):
        print ' '*10, f_c[6*j : 6*j+3]

def Main ():
    # Configure Numpy printing options
    set_printoptions(threshold=nan, linewidth=378, precision=8)
    
    print 'Initialize a rigid body with initial conditions...'
    #
    #                  ________
    #                 |       |
    #                 |  p2   |    ----- 4.5
    #                 |_______|    _____ 4.0
    #                     |
    #                     | f1
    #                  ___|____    _____ 1.0
    #                 |       |
    #                 |  p1   |    ----- 0.5
    #                 |_______|
    #  --------------------------------------------------
    #  //////////////////////////////////////////////////
    #
    p1 = RigidBody('BODY 0', array([0.,0,0.5,0,0,0]), array([0.,0,0.5,0,0,0]), 10, array([1.,1.,1.]))   # Bottom box
    p2 = RigidBody('BODY 1', array([0.,0,4.5,0,0,0]), array([0.,0,4.5,0,0,0]), 2, array([1.,1.,1.]))   # Top box
    p1AttPos = array([0.,0,0.5])
    p2AttPos = array([0.,0,-0.5])
    f1 = Fiber(0.,    # T_0
               10.,    # kse
               10.,    # kpe
               10.,    # b
               3.0,   # x_r0
               1.,   # x_rl
               10.,   # x_ru
               p1, p2, p1AttPos, p2AttPos)
    
    # Make a MOSEK environment
    env = mosek.Env ()
    #mosek.iparam.intpnt_num_threads = 4

    nFrame = int(sys.argv[1])
    printMsg = '-d' in sys.argv[1:]
    # Attach a printer to the environment
    if printMsg:
        env.set_Stream (mosek.streamtype.log, streamprinter)
    nd = 6
    
    plist = [p1, p2]
    flist = [f1]
    
    '''
    # Regression test
    plist = [p1,p2]
    flist = []
    '''
    nb = len(plist)
    nm = len(flist)
    for ff in xrange(nFrame):
        
        if ff < 100:
            plist[1].chi_ref = array([0.,0,4.5+0.05*ff,0,0,0])
        
        if ff==76:
            pass
        frameReport = ''
        frameReport += '='*40 + ' FRAME ' + str(ff) + ' START ' + '='*40 + '\n'
        for i in xrange(nb):
            frameReport += '  BODY %d:\n' % i
            frameReport += '    chi_0 = ' + str(plist[i].chi_0) + '\n'
            frameReport += '    chi_1 = ' + str(plist[i].expBody.q) + '\n'
        for j in xrange(nm):
            frameReport += '  MUSCLE %d:\n' % i
            frameReport += '    T_0 = ' + str(flist[j].T_0) + '\n'
            frameReport += '    x_r = ' + str(flist[j].x_r0) + '\n'
        frameReport += '  Updating current state dependent values...' + '\n'
        for i in xrange(nb):
            frameReport += plist[i].updateCurrentStateDependentValues()
        frameReport += '  Done.' + '\n'
        frameReport += '  Updating fiber related values...' + '\n'
        for i in xrange(nb):
            plist[i].updateFiberRelatedValues() # Should be called after all updateCurrentStateDependentValues() calls.
        frameReport += '  Done.' + '\n'
        frameReport += '  Optimizing to compute next state...' + '\n'
        subFrameReport, x_opt, cost, Ari, Aci = Optimize(env, printMsg, plist, flist)
        frameReport += subFrameReport
        if x_opt is None:
            frameReport += 'Mosek optimizer failed?!\n'
            '''
            print '='*30, 'MATRIX A^T', '='*30
            print p1_A.T
            '''
            print frameReport
            sys.exit(-123)
        for i in xrange(nb):
            if len(plist[i].p) > 0:
                frameReport += ' '*4 + ' - BODY %d calculated contact forces\n' % i
                for pfi, j in zip(plist[i].p, xrange(len(plist[i].p))):
                    frameReport += ' '*10 + str(pfi) + ':' + str( x_opt[ Aci[i] + plist[i].Aci[1] + nd*j : Aci[i] + plist[i].Aci[1] + nd*j + 3] ) + '\n'
        for j in xrange(nm):
            frameReport += ' '*4 + ' - MUSCLE %d optimization result\n' % j
            frameReport += ' '*4 + '     Tension   = ' + str( x_opt[Aci[nb  ]+j] ) + '\n'
            frameReport += ' '*4 + '     Actuation = ' + str( x_opt[Aci[nb+1]+j] ) + '\n'
            frameReport += ' '*4 + '     Rest len. = ' + str( x_opt[Aci[nb+2]+j] ) + '\n'
        frameReport += '  Done. Cost = ' + str(cost) + '\n'
        for i in xrange(nb):
            frameReport += plist[i].setChi_1( x_opt[ Aci[i] + plist[i].Aci[0] : Aci[i] + plist[i].Aci[1] ] )
            frameReport += plist[i].reparameterize()
            frameReport += '  BODY %d Current state updated to:\n' % i
            frameReport += '  chi_0 = ' + str(plist[i].chi_0) + '\n'
            frameReport += '  chi_1 = ' + str(plist[i].expBody.q) + '\n'
        for j in xrange(nm):
            flist[j].setT_0( x_opt[ Aci[nb+0] + j ] )
            flist[j].x_r0 =  x_opt[ Aci[nb+2] + j ]

        # Print buffered frame report entirely
        print frameReport

def Optimize(env, printMsg, plist, flist):
    # Create a task
    task = env.Task(0,0)

    # Attach a printer to the task
    if printMsg:
        task.set_Stream (mosek.streamtype.log, streamprinter)
    Alist = [pi.getAMatrix() for pi in plist]
    nplist = [len(pi.p) for pi in plist]
    nmlist = [len(pi.fibers) for pi in plist]
    nb = len(plist)     # Number of RBs
    nd = 6              # 6-DOF
    nm = len(flist)     # Numer of muscle fibers
    # Calculate subblock matrices' row/col indices
    Asubrowsizes = [Ai.shape[0] for Ai in Alist] + [nd*nmj for nmj in nmlist] + [ nm ]
    Asubcolsizes = [Ai.shape[1] for Ai in Alist] + [ nm, nm, nm, 1, 1 ]
    Ari = [0]
    Aci = [0]
    for a in Asubrowsizes:
        Ari.append( Ari[-1] + a )
    for a in Asubcolsizes:
        Aci.append( Aci[-1] + a )
    
    # Objective function coefficients
    c   = zeros(Aci[-1])
    for i in xrange(nb):
        for j in xrange(nplist[i]):
            c[ Aci[i] + plist[i].Aci[3] + 4*j + 2 ] = 0                 # Estimated position of z-coordinate of contact point
        c[ Aci[i] + plist[i].Aci[5] : Aci[i] + plist[i].Aci[6] ] = 0    # minimize the movement of candidate contact points
        c[ Aci[i] + plist[i].Aci[8] ] = 0 # minimize the movement of COMs
    c[ Aci[nb+3]:Aci[nb+4] ] = 0.0 # minimize aggregate tension
    c[ Aci[nb+4]:Aci[nb+5] ] = 1 # minimize aggregate actuation force
    
    # Matrix and vector for equality constraints
    A = sparse.lil_matrix((Ari[-1],Aci[-1]))
    for i in xrange(nb):
        A[ Ari[i]:Ari[i+1]       , Aci[i]:Aci[i+1]                ] = Alist[i]
        if nmlist[i] > 0:
            A[ Ari[nb+i]:Ari[nb+i+1] , Aci[i+1]-nd*nmlist[i]:Aci[i+1] ] = -sparse.identity(nd*nmlist[i])
        for j in xrange(nmlist[i]):
            # Transform index: Calculate global fiber index of
            #                  j-th attached fiber on Body i
            gbl_j = flist.index( plist[i].fibers[j] )
            assert gbl_j >= 0
            A[ Ari[nb+i]+nd*j:Ari[nb+i]+nd*(j+1), Aci[nb] + gbl_j] = plist[i].R[j].reshape(6,1)
    for j in xrange(nm):
        Kj = flist[j].getK()
        A[ Ari[2*nb] + j, Aci[nb + 0] + j ] = Kj[0]
        A[ Ari[2*nb] + j, Aci[nb + 1] + j ] = Kj[1]
        A[ Ari[2*nb] + j, Aci[nb + 2] + j ] = Kj[2]
        
    Eta = sparse.lil_matrix((Ari[-1], 1))
    for i in xrange(nb):
        Eta[ Ari[i]:Ari[i+1], 0 ] = plist[i].getEta()
    for j in xrange(nm):
        Eta[ Ari[2*nb] + j, 0 ] = flist[j].gets()
    
    NUMVAR = A.shape[1]
    NUMCON = A.shape[0]
    NUMANZ = A.nnz
    # Give MOSEK an estimate of the size of the input data. 
    #  This is done to increase the speed of inputting data. 
    #  However, it is optional. 
    task.putmaxnumvar(NUMVAR)
    task.putmaxnumcon(NUMCON)
    task.putmaxnumanz(NUMANZ)

    # Configure all constraints defined by the above matrix and vector are equality constraints
    bkc = [ mosek.boundkey.fx, ] * A.shape[0]
    Etaden = array(Eta.todense())
    blc = [ Etaden[i,0] for i in xrange(Etaden.shape[0]) ]
    buc = copy.deepcopy(blc)



    # Optimization variable boundary configuration
    # ( Default: unbounded, i.e., (-inf,inf) )
    bkx = [ mosek.boundkey.fr, ] * A.shape[1]
    blx = [ -inf, ] * A.shape[1]
    bux = [  inf, ] * A.shape[1]
    
    nonnegatives = []
    fixed_ones   = []
    fixed_zeros  = []
    
    for i in xrange(nb):
        nonnegatives += [ Aci[i] + plist[i].Aci[0] +  2                                  ] # chi_2_z
        nonnegatives += [ Aci[i] + plist[i].Aci[1] + nd*j + 2 for j in xrange(nplist[i]) ] # f_c_z
        fixed_zeros  += [ Aci[i] + plist[i].Aci[2] +  5*j + 2 for j in xrange(nplist[i]) ] # c_c_z (TODO: Assumes flat ground)
        fixed_zeros  += [ Aci[i] + plist[i].Aci[2] +  5*j + 3 for j in xrange(nplist[i]) ] # c_c_w
        nonnegatives += [ Aci[i] + plist[i].Aci[2] +  5*j + 4 for j in xrange(nplist[i]) ] # c_c_n
        nonnegatives += [ Aci[i] + plist[i].Aci[3] +  4*j + 2 for j in xrange(nplist[i]) ] # p_c_2_z
        fixed_zeros  += [ Aci[i] + plist[i].Aci[3] +  4*j + 2 for j in xrange(nplist[i]) ] # p_c_2_z (TODO: How to allow contact break?)
        fixed_ones   += [ Aci[i] + plist[i].Aci[3] +  4*j + 3 for j in xrange(nplist[i]) ] # p_c_2_w
        nonnegatives += range(Aci[i] + plist[i].Aci[5], Aci[i] + plist[i].Aci[6]) # eps_fric
        nonnegatives += range(Aci[i] + plist[i].Aci[6], Aci[i] + plist[i].Aci[7]) # muf_cz
        nonnegatives += range(Aci[i] + plist[i].Aci[8], Aci[i] + plist[i].Aci[9]) # eps_delta
    
    for j in xrange(nm):
        i = Aci[nb+2]+j
        bkx[i], blx[i], bux[i] = mosek.boundkey.ra, flist[j].x_rl, flist[j].x_ru
    for i in nonnegatives:
        bkx[i], blx[i], bux[i] = mosek.boundkey.lo, 0, inf
    for i in fixed_ones:
        bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 1, 1
    for i in fixed_zeros:
        bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 0, 0
        
    ### DEBUG PURPOSE (REMOVE IT!) ###
    i = Aci[1]+2 # linear z-axis position of body 1 (top)
    bkx[i], blx[i], bux[i] = mosek.boundkey.ra, 4.5, 4.5
    i = Aci[2] # Muscle fiber tension
    #bkx[i], blx[i], bux[i] = mosek.boundkey.ra, -plist[0].expBody.mass*GRAV, plist[0].expBody.mass*GRAV
    
    #set_printoptions(threshold=nan, linewidth=378, precision=4)
    #print A.todense()
    #print Eta
    
    Acsc = A.tocsc()
    asub = []
    aval = []
    for i in xrange(Acsc.shape[1]):
        asub.append( Acsc.indices[Acsc.indptr[i]:Acsc.indptr[i+1]] )
        aval.append( Acsc.data[Acsc.indptr[i]:Acsc.indptr[i+1]] )

    # Append 'NUMCON' empty constraints.
    # The constraints will initially have no bounds. 
    task.append(mosek.accmode.con,NUMCON)

    #Append 'NUMVAR' variables.
    # The variables will initially be fixed at zero (x=0). 
    task.append(mosek.accmode.var,NUMVAR)

    #Optionally add a constant term to the objective. 
    task.putcfix(0.0)

    for j in range(NUMVAR):
        # Set the linear term c_j in the objective.
        task.putcj(j,c[j])
        # Set the bounds on variable j
        # blx[j] <= x_j <= bux[j] 
        task.putbound(mosek.accmode.var,j,bkx[j],blx[j],bux[j])

    for j in range(len(aval)):
        # Input column j of A
        task.putavec(mosek.accmode.var,  # Input columns of A.
                     j,                  # Variable (column) index.
                     asub[j],            # Row index of non-zeros in column j.
                     aval[j])            # Non-zero Values of column j. 
    for i in range(NUMCON):
        task.putbound(mosek.accmode.con,i,bkc[i],blc[i],buc[i])
    #
    # Input the cones
    #
    for i in xrange(nb):
        # epsilon_Delta >= || Delta_chi_i ||
        task.appendcone(mosek.conetype.quad, 0.0, [ Aci[i] + plist[i].Aci[8],
                                                    Aci[i] + plist[i].Aci[7] + 0,
                                                    Aci[i] + plist[i].Aci[7] + 1,
                                                    Aci[i] + plist[i].Aci[7] + 2 ])
        for j in xrange(nplist[i]):
            # CP movement minimization
            task.appendcone(mosek.conetype.quad, 0.0, [ Aci[i] + plist[i].Aci[5] + j,
                                                        Aci[i] + plist[i].Aci[4]+4*j+0,
                                                        Aci[i] + plist[i].Aci[4]+4*j+1,
                                                        Aci[i] + plist[i].Aci[4]+4*j+2 ])
            # Friction cone constraints
            task.appendcone(mosek.conetype.quad, 0.0, [ Aci[i] + plist[i].Aci[6]+j,
                                                        Aci[i] + plist[i].Aci[1]+6*j+0,
                                                        Aci[i] + plist[i].Aci[1]+6*j+1 ])
    # Minimal tension force constraints
    task.appendcone(mosek.conetype.quad, 0.0, [ Aci[nb+3] ] + range(Aci[nb+0],Aci[nb+1]))
    # Minimal actuation force constraints
    task.appendcone(mosek.conetype.quad, 0.0, [ Aci[nb+4] ] + range(Aci[nb+1],Aci[nb+2]))

    # Input the objective sense (minimize/maximize)
    task.putobjsense(mosek.objsense.minimize)

    task.writedata('mosek_bugreport.opf')
    # Optimize the task
    task.optimize()
    # Print a summary containing information
    # about the solution for debugging purposes
    task.solutionsummary(mosek.streamtype.msg)
    prosta = []
    solsta = []
    [prosta,solsta] = task.getsolutionstatus(mosek.soltype.itr)

    # Output a solution
    xx = zeros(NUMVAR, float)
    task.getsolutionslice(mosek.soltype.itr,
                          mosek.solitem.xx, 
                          0,NUMVAR,          
                          xx)
    frameReport = ''
    cost = dot(c, xx)
    if solsta == mosek.solsta.optimal or solsta == mosek.solsta.near_optimal:
        return frameReport, xx, cost, Ari, Aci
    elif solsta == mosek.solsta.dual_infeas_cer: 
        frameReport += "Dual infeasibility.\n"
    elif solsta == mosek.solsta.prim_infeas_cer:
        frameReport += "Primal infeasibility.\n"
    elif solsta == mosek.solsta.near_dual_infeas_cer:
        frameReport += "Near dual infeasibility.\n"
    elif  solsta == mosek.solsta.near_prim_infeas_cer:
        frameReport += "Near primal infeasibility.\n"
    elif solsta == mosek.solsta.unknown:
        frameReport += "?!?!?!?!?!? W A R N I N G : Unknown solution status ?!?!?!?!?!?\n"
        #print("See mosek_investigate.opf for details.")
        #task.writedata('mosek_investigate.opf')
        return frameReport, xx, cost, Ari, Aci
    else:
        frameReport += "Other solution status?!?!?!?!?"
    return frameReport, None, None, None, None

if __name__ == '__main__':
    Main()
    '''
    rb = RigidBody(array([1.,5,2.5,1,2,3]), array([-1.,-2,-3,3,1,2]), 10, array([1.,2.,3.]))
    rb.updateCurrentStateDependentValues()
    A = rb.getAMatrix()
    eta = rb.getEta()
    print type(A)
    print A
    print eta
    '''