#
#  Copyright: Copyright (c) 1998-2009 MOSEK ApS, Denmark. All rights reserved.
#
#  File:    lo2.py
#
#  Purpose: Demonstrates how to solve small linear
#           optimization problem using the MOSEK Python API.
##

import sys
from numpy import array, zeros, ones, linalg, hstack, dot
import mosek
from math import sqrt
from scipy import sparse

# Since the actual value of Infinity is ignores, we define it solely
# for symbolic purposes:
inf = 0.0
MU = 1.
GRAV = 9.81
h = 0.01

class Particle:
    def __init__(self, chi_0, chi_1, mass):
        self.chi_0  = array(chi_0)
        self.chi_1  = array(chi_1)
        self.mass   = mass
        self.f_g    = array([0, 0, -mass*GRAV])
        self.fibers = []
    def __str__(self):
        return str(self.chi_1)
    def getAMatrix(self):
        A = sparse.lil_matrix((8,16))
        A[0:3,1:4] = self.mass/h**2*sparse.identity(3)
        A[0:3,4:7] = -sparse.identity(3)
        A[0:3,13:16] = -sparse.identity(3)
        A[3:6,1:4] = sparse.identity(3)
        A[3:6,7:10] = sparse.identity(3)
        A[6,3] = sqrt(self.mass)/h
        A[6,10] = -1
        A[6,15] = -h/(2*sqrt(self.mass))
        A[7,6] = MU
        A[7,12] = -1
        return A
    def getEta(self):
        return hstack([ self.mass / h**2 * (2*self.chi_1 - self.chi_0) + self.f_g,
                        self.chi_1,
                        array([ -h*self.getq()/(2*sqrt(self.mass)), 0 ]) ])
    def getq(self):
        return -self.mass/h**2 * (2*self.chi_1[2] - self.chi_0[2]) - self.f_g[2]
    def setChi_1(self, chi_1):
        self.chi_0 = self.chi_1
        self.chi_1 = array(chi_1)

class Fiber:
    def __init__(self, T_0, kse, kpe, b, x_r0, x_rl, x_ru, p1, p2):
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
    def __str__(self):
        return 'T_0=' + str(self.T_0) + ' x_r0=' + str(self.x_r0)
    def setT_0(self, T_0):
        self.T_0 = T_0
    def getK(self):
        return array([ self.kse * self.b + self.kse**2 * h + self.kse * h * self.kpe,
                       -self.kse**2 * h,
                       self.kse**2 * h * self.kpe ])
    def gets(self):
        x_0 = linalg.norm(self.p1.chi_0 - self.p2.chi_0)
        x_1 = linalg.norm(self.p1.chi_1 - self.p2.chi_1)
        return self.kse * self.b * self.T_0 + self.kse**2 * h * self.kpe * x_1 + self.kse**2 * self.b * (x_1 - x_0)

# Define a stream printer to grab output from MOSEK
def streamprinter(text):
    sys.stdout.write(text)
    sys.stdout.flush()

def PrintOptimizedResult(x_opt):
    print 'Optimal solution'
    print '-'*80
    print '       BODY 1'
    print '           e_i =', x_opt[0]
    print '     chi^(i+1) =', x_opt[1:4]
    print '           f_c =', x_opt[4:7]
    print '     delta chi =', x_opt[7:10]
    print '         c_lcp =', x_opt[10 ]
    print '        e_fric =', x_opt[11 ]
    print '     muf_{c,z} =', x_opt[12 ]
    print '           f_T =', x_opt[13:16]
    print
    print '       BODY 2'
    print '           e_i =', x_opt[16+0]
    print '     chi^(i+1) =', x_opt[16+1:16+4]
    print '           f_c =', x_opt[16+4:16+7]
    print '     delta chi =', x_opt[16+7:16+10]
    print '         c_lcp =', x_opt[16+10 ]
    print '        e_fric =', x_opt[16+11 ]
    print '     muf_{c,z} =', x_opt[16+12 ]
    print '           f_T =', x_opt[16+13:16+16]
    print
    print '       FIBER 1'
    print '             T =', x_opt[32]
    print '             u =', x_opt[33]
    print '           x_r =', x_opt[34]
    print '           e_T =', x_opt[35]
    print '           e_u =', x_opt[36]
    
# We might write everything directly as a script, but it looks nicer
# to create a function.
def Main ():
    p1 = Particle([0.,0,0], [0.,0,0], 1.)
    p2 = Particle([0,0,5], [0,0,5], 1.)
    f1 = Fiber(0., 2000., 1000., 1000., 1.0, 0.5, 1.5, p1, p2)
    p1_A = p1.getAMatrix()
    p2_A = p2.getAMatrix()
    f1_K = f1.getK()
    
    # Make a MOSEK environment
    env = mosek.Env ()
    mosek.iparam.intpnt_num_threads = 4
    
    nFrame = int(sys.argv[1])
    printMsg = '-d' in sys.argv[1:]
    # Attach a printer to the environment
    if printMsg:
        env.set_Stream (mosek.streamtype.log, streamprinter)
    try:
        for i in xrange(nFrame):
            # Objective function coefficients
            c   = zeros(37)
            c[ 35    ] = 0                # epsilon_T
            c[ 36    ] = 1                # epsilon_u
            
            c[ 0     ] = 1                # epsilon_{1}
            c[ 6     ] = 10*p1.chi_1[2]  # f_{c,1,z}
            c[ 11    ] = 10                  # epsilon_{fric,1}
            
            c[ 16+0  ] = 1                # epsilon_{2}
            c[ 16+6  ] = 10*p2.chi_1[2]  # f_{c,2,z}
            c[ 16+11 ] = 10                  # epsilon_{fric,2}
            
            x_opt = Optimize(env, printMsg, p1, p2, f1, p1_A, p2_A, f1_K, c)
            if x_opt is None:
                sys.exit(-123)
            cost = dot(c, x_opt)            
            print 'Frame', i, cost, p1, p2, f1, x_opt[4:7], x_opt[16+4:16+7]
            
            p1.setChi_1(x_opt[1:4])
            p2.setChi_1(x_opt[16+1:16+4])
            f1.setT_0(x_opt[32])
            f1.x_r0 = x_opt[34]
            if printMsg:
                PrintOptimizedResult(x_opt)
                
    except mosek.Exception, msg:
        if msg is not None:
            print "\t%s" % msg
            sys.exit(1)
    
    print 'Frame', i+1, p1, p2, f1

def Optimize(env, printMsg, p1, p2, f1, p1_A, p2_A, f1_K, c):
    # Create a task
    task = env.Task(0,0)

    # Attach a printer to the task
    if printMsg:
        task.set_Stream (mosek.streamtype.log, streamprinter)

    # Matrix and vector for equality constraints
    A = sparse.lil_matrix((8+8+3+3+1, 16+16+1+1+1+1+1))
    A[0:8,0:16] = p1_A
    A[8:16,16:32] = p2_A
    A[16:19,13:16] = sparse.identity(3)
    A[16:19,32] = -(p2.chi_1 - p1.chi_1).reshape(3,1)
    A[19:22,29:32] = sparse.identity(3)
    A[19:22,32] = -(p1.chi_1 - p2.chi_1).reshape(3,1)
    A[22,32:35] = f1_K
    
    Eta = hstack([ p1.getEta(),
                   p2.getEta(),
                   zeros(3),
                   zeros(3),
                   f1.gets() ])
    
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
    blc = list(Eta)
    buc = list(Eta)

    
    
    # Optimization variable boundary configuration
    bkx = [ mosek.boundkey.fr, ] * A.shape[1]
    blx = [ -inf, ] * A.shape[1]
    bux = [  inf, ] * A.shape[1]

    bkx[3], blx[3], bux[3] = mosek.boundkey.lo, 0, inf         # chi_{1,z}
    bkx[6], blx[6], bux[6] = mosek.boundkey.lo, 0, inf         # f_{c,1,z}
    bkx[10], blx[10], bux[10] = mosek.boundkey.lo, 0, inf      # c_{lcp,1}
    bkx[11], blx[11], bux[11] = mosek.boundkey.lo, 0, inf      # epsilon_{fric,1}
    bkx[12], blx[12], bux[12] = mosek.boundkey.lo, 0, inf      # muf_{c,1,z}
    
    bkx[16+3], blx[16+3], bux[16+3] = mosek.boundkey.lo, 0, inf         # chi_{1,z}
    bkx[16+6], blx[16+6], bux[16+6] = mosek.boundkey.lo, 0, inf         # f_{c,1,z}
    bkx[16+10], blx[16+10], bux[16+10] = mosek.boundkey.lo, 0, inf      # c_{lcp,1}
    bkx[16+11], blx[16+11], bux[16+11] = mosek.boundkey.lo, 0, inf      # epsilon_{fric,1}
    bkx[16+12], blx[16+12], bux[16+12] = mosek.boundkey.lo, 0, inf      # muf_{c,1,z}
    
    bkx[34], blx[34], bux[34] = mosek.boundkey.ra, f1.x_rl, f1.x_ru     # x_r
    
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

    # Input the cones
    task.appendcone(mosek.conetype.quad, 0.0, [ 35, 32 ])       # epsilon_T >= || T ||
    task.appendcone(mosek.conetype.quad, 0.0, [ 36, 33 ])       # epsilon_u >= || u ||

    task.appendcone(mosek.conetype.quad, 0.0, [  0, 10 ])       # epsilon_1 >= || c_{lcp,1} ||
    task.appendcone(mosek.conetype.quad, 0.0, [ 12, 4, 5 ])     # muf_{c,1,z} >= || f_{c,1,t} ||
    task.appendcone(mosek.conetype.quad, 0.0, [ 11, 7, 8, 9 ])  # epsilon_{fric,1} >= || delta chi_1 ||
    
    task.appendcone(mosek.conetype.quad, 0.0, [  16+0, 16+10 ])             # epsilon_1 >= || c_{lcp,1} ||
    task.appendcone(mosek.conetype.quad, 0.0, [ 16+12, 16+4, 16+5 ])        # muf_{c,1,z} >= || f_{c,1,t} ||
    task.appendcone(mosek.conetype.quad, 0.0, [ 16+11, 16+7, 16+8, 16+9 ])  # epsilon_{fric,1} >= || delta chi_1 ||

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

    if solsta == mosek.solsta.optimal or solsta == mosek.solsta.near_optimal:
        return xx
    elif solsta == mosek.solsta.dual_infeas_cer: 
        print("Primal or dual infeasibility.\n")
    elif solsta == mosek.solsta.prim_infeas_cer:
        print("Primal or dual infeasibility.\n")
    elif solsta == mosek.solsta.near_dual_infeas_cer:
        print("Primal or dual infeasibility.\n")
    elif  solsta == mosek.solsta.near_prim_infeas_cer:
        print("Primal or dual infeasibility.\n")
    elif mosek.solsta.unknown:
        print("Unknown solution status")
    else:
        print("Other solution status")

    return None

if __name__ == '__main__':
    Main()
