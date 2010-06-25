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
from math import sqrt, pi, ceil
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
    def __init__(self, chi_0, chi_1, mass, boxsize):
        self.chi_0 = chi_0
        vel_wc = (chi_1 - chi_0) / h
        self.expBody = ExpBody.ExpBody('singlerb', None, mass, boxsize,
                                       chi_1,
                                       vel_wc[0:3],
                                       vel_wc[3:6],
                                       [0.1,0.2,0.3])
        self.isDirty = True
        self.nd = 6 # 6-DOF
    def __str__(self):
        return '%s%s' % (str(self.expBody.q[0:3]), str(self.expBody.q[3:6]))
    def getStabilityStr(self):
        th = linalg.norm(self.expBody.q[3:6])
        thStability = 1.-abs(th-2*pi*int(th/(2*pi)))/(2*pi)
        return 'theta=%s / stability=%s' % (str(th), str(thStability))
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
        self.dRdv_tensor = ZV.dRdv_tensor(chi_1)
        
        self.pc = []
        if hasattr(self, 'p') and len(self.p) > 0:
            for j in self.p:
                pcj = array(list(self.expBody.corners[j]) + [1.])
                pcj_1_W = dot(self.W, pcj)
                self.pc.append(pcj_1_W)
        if len(self.pc):
            frameReport += ' '*4 + ' - Previously anticipated (current) contact points' + '\n'
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
            frameReport += ' '*4 + ' - Anticipated contact points fixing positions (pc_fix)' + '\n'
            for pf, pfi in zip(self.pc_fix, self.p):
                frameReport += ' '*10 + str(pfi) + ':' + str(pf) + '\n'
        
        self.np = len(self.p) # eight contact points (worst case)        
        np = self.np
        nd = self.nd
        Asubrowsizes = [nd, nd*np, 4*np, np, 4*np, nd]
        Asubcolsizes = [nd, nd*np, 5*np, 4*np, 4*np, np, np, nd, 1]
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
        eta[ ri[5]:ri[6], 0 ] = self.expBody.q.reshape(nd,1)
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
    '''
    print '           c_c =', c_c[0:4], c_c[4]
    print '     til_p_c_2 =', x_opt[ ci[4]:ci[5] ]
    print '         q_c   =', x_opt[ ci[5]:ci[6] ]
    print '         c_lcp =', x_opt[ ci[6]:ci[7] ]
    '''
    print
    

# We might write everything directly as a script, but it looks nicer
# to create a function.
def Main ():
    '''
    A = rb.getAMatrix()
    eta = rb.getEta()
    print type(A)
    print A
    print eta
    '''
    
    # Configure Numpy printing options
    set_printoptions(threshold=nan, linewidth=378, precision=8)
    
    print 'Initialize a rigid body with initial conditions...'
    p1 = RigidBody(array([0,0,5,0.1,0.2,0.3]), array([0.1,0.2,5,0.1,0.2,0.3]), 2, array([1.,1.,1.]))
    
    # Make a MOSEK environment
    env = mosek.Env ()
    #mosek.iparam.intpnt_num_threads = 4

    nFrame = int(sys.argv[1])
    printMsg = '-d' in sys.argv[1:]
    # Attach a printer to the environment
    if printMsg:
        env.set_Stream (mosek.streamtype.log, streamprinter)
    nd = 6
    for i in xrange(nFrame):
        frameReport = ''
        frameReport += '='*40 + ' FRAME ' + str(i) + ' START ' + '='*40 + '\n'
        frameReport += '  chi_0 = ' + str(p1.chi_0) + '\n'
        frameReport += '  chi_1 = ' + str(p1.expBody.q) + '\n'
        frameReport += '  Updating current state dependent values...' + '\n'
        frameReport += p1.updateCurrentStateDependentValues()
        frameReport += '  Done.' + '\n'
        p1_A = p1.getAMatrix()
        np = len(p1.p)
        
        frameReport += '  Optimizing to compute next state...' + '\n'
        # Objective function coefficients
        c   = zeros(p1_A.shape[1])
        for j in xrange(np):
            c[ p1.Aci[3] + 4*j + 2 ] = 1       # Estimated position of z-coordinate of contact point
        c[ p1.Aci[5]:p1.Aci[6] ] = 1           # minimize the movement of candidate contact points
        
        subFrameReport, x_opt = Optimize(env, printMsg, p1, p1_A, c)
        frameReport += subFrameReport
        if x_opt is None:
            frameReport += 'Mosek optimizer failed?!\n'
            '''
            print '='*30, 'MATRIX A^T', '='*30
            print p1_A.T
            '''
            print frameReport
            sys.exit(-123)
        cost = dot(c, x_opt)
        if np > 0:
            frameReport += ' '*4 + ' - Calculated contact forces\n'
            for pfi, j in zip(p1.p, xrange(np)):
                frameReport += ' '*10 + str(pfi) + ':' + str( x_opt[ p1.Aci[1] + nd*j : p1.Aci[1] + nd*j + 3] ) + '\n'
        
        frameReport += '  Done. Cost = ' + str(cost) + '\n'
        
        frameReport += p1.setChi_1( x_opt[ p1.Aci[0]:p1.Aci[1] ] )
        frameReport += p1.reparameterize()
        frameReport += '  Current state updated to:\n'
        frameReport += '  chi_0 = ' + str(p1.chi_0) + '\n'
        frameReport += '  chi_1 = ' + str(p1.expBody.q) + '\n'
        #PrintOptimizedResult(p1, x_opt)
        '''
        if printMsg:
            PrintOptimizedResult(p1, x_opt)
            pass
        '''
        print frameReport

def Optimize(env, printMsg, p1, p1_A, c):
    # Create a task
    task = env.Task(0,0)

    # Attach a printer to the task
    if printMsg:
        task.set_Stream (mosek.streamtype.log, streamprinter)
        
    nd = p1.nd
    np = len(p1.p)

    # Matrix and vector for equality constraints
    A = p1_A
    Eta = p1.getEta()
    
    

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
    
    nonnegatives += [ p1.Aci[0] +  2                           ] # chi_2_z
    nonnegatives += [ p1.Aci[1] + nd*j + 2 for j in xrange(np) ] # f_c_z
    fixed_zeros  += [ p1.Aci[2] +  5*j + 2 for j in xrange(np) ] # c_c_z (TODO: Assumes flat ground)
    fixed_zeros  += [ p1.Aci[2] +  5*j + 3 for j in xrange(np) ] # c_c_w
    nonnegatives += [ p1.Aci[2] +  5*j + 4 for j in xrange(np) ] # c_c_n
    nonnegatives += [ p1.Aci[3] +  4*j + 2 for j in xrange(np) ] # p_c_2_z
    fixed_zeros  += [ p1.Aci[3] +  4*j + 2 for j in xrange(np) ] # p_c_2_z (TODO: How to allow contact break?)
    fixed_ones   += [ p1.Aci[3] +  4*j + 3 for j in xrange(np) ] # p_c_2_w
    nonnegatives += range(p1.Aci[5],p1.Aci[6]) # eps_fric
    nonnegatives += range(p1.Aci[6],p1.Aci[7]) # muf_cz
    nonnegatives += range(p1.Aci[8],p1.Aci[9]) # eps_delta
    
    for i in nonnegatives:
        bkx[i], blx[i], bux[i] = mosek.boundkey.lo, 0, inf
    for i in fixed_ones:
        bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 1, 1
    for i in fixed_zeros:
        bkx[i], blx[i], bux[i] = mosek.boundkey.fx, 0, 0
    
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
    for j in xrange(np):
        # CP movement minimization
        task.appendcone(mosek.conetype.quad, 0.0, [ p1.Aci[5]+j,
                                                    p1.Aci[4]+4*j+0, p1.Aci[4]+4*j+1, p1.Aci[4]+4*j+2 ])
        # Friction cone constraints
        task.appendcone(mosek.conetype.quad, 0.0, [ p1.Aci[6]+j,
                                                    p1.Aci[1]+6*j+0, p1.Aci[1]+6*j+1 ])

    # Input the objective sense (minimize/maximize)
    task.putobjsense(mosek.objsense.minimize)

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
    if solsta == mosek.solsta.optimal or solsta == mosek.solsta.near_optimal:
        return frameReport, xx
    elif solsta == mosek.solsta.dual_infeas_cer: 
        frameReport += "Dual infeasibility.\n"
    elif solsta == mosek.solsta.prim_infeas_cer:
        frameReport += "Primal infeasibility.\n"
    elif solsta == mosek.solsta.near_dual_infeas_cer:
        frameReport += "Near dual infeasibility.\n"
    elif  solsta == mosek.solsta.near_prim_infeas_cer:
        frameReport += "Near primal infeasibility.\n"
    elif solsta == mosek.solsta.unknown:
        frameReport += "Unknown solution status?!?!?!?!?!?"
        #print("See mosek_investigate.opf for details.")
        #task.writedata('mosek_investigate.opf')
        return frameReport, xx
    else:
        frameReport += "Other solution status?!?!?!?!?"
    return frameReport, None

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